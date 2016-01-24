#include "qi/base/threadrunner.h"

namespace qi {

ThreadRunner::ThreadRunner(size_t thread_count) : shutdown_(false) {
  ASSERT(thread_count > 0);
  for (size_t i = 0; i < thread_count; ++i) {
    threads_.push_back(std::thread(Loop, this));
  }
}

ThreadRunner::~ThreadRunner() {
  std::unique_lock<Mutex> locker(mutex_);
  ASSERT(shutdown_);
}

bool ThreadRunner::Run(std::function<void()> job) {
  std::unique_lock<Mutex> locker(mutex_);
  if (shutdown_) return false;
  jobs_.push_back(move(job));
  condition_.notify_one();
  return true;
}

void ThreadRunner::Shutdown() {
  {
    std::unique_lock<Mutex> locker(mutex_);
    ASSERT(!shutdown_);
    shutdown_ = true;
    condition_.notify_all();
  }
  // Wait for all threads to finish.
  for (auto& t : threads_) {
    t.join();  // TODO: shouldn't be necessary; clear() should be enough.
  }
  threads_.clear();
}

void ThreadRunner::Loop(ThreadRunner* runner) {
  while (true) {
    std::unique_lock<Mutex> lock(runner->mutex_);
    if (!runner->shutdown_ && runner->jobs_.empty()) {
      runner->condition_.wait(lock);
    }  // ... otherwise, don't wait before running all remaining jobs.

    ASSERT(lock.owns_lock());  // sanity-check.
    while (!runner->jobs_.empty()) {
      auto job = move(runner->jobs_.front());
      runner->jobs_.pop_front();
      lock.unlock();
      job();
      lock.lock();
    }

    ASSERT(lock.owns_lock());  // sanity-check.
    if (runner->shutdown_) {
      ASSERT(runner->jobs_.empty());
      return;
    }
  }
}

}  // namespace qi
