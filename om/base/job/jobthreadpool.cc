#include "om/base/job/jobthreadpool.h"

namespace om {

JobThreadPool::JobThreadPool() : desired_active_thread_count_(0) {
}

void JobThreadPool::SetDesiredActiveThreadCount(size_t desired_count) {
  UniqueLock lock(mutex_);
  desired_active_thread_count_ = desired_count;

  AdjustThread

}


void JobThreadPool::MoveThread(const UniqueLock& lock, JobThread* thread,
                               JobThreadSet* source, JobThreadSet* destination) {
  DCHECK(lock.owns_lock());
  DCHECK(&mutex == lock->mutex());
  DCHECK(nullptr != thread);
  bool found_in_source = source->erase(thread);
  DCHECK(found_in_source);
  destination->insert(thread);
}

JobThread::JobThread(JobThreadPool* owner)
    : pool_(owner),
      running_(false),
      quitting_(false),
      mutex_(pool_->mutex),
      thread(&JobThread::Run, this) {

}

JobThread::~JobThread() {
  DCHECK(quitting_);

}

JobThread::AboutToBlock() {
  UniqueLock lock(mutex);
  pool_->MoveThread(lock, this, pool_->active_threads_, pool_->blocked_threads_);
  pool_->AdjustActiveThreads(std::move(lock));
}

void JobThread::Run() {
  std::unique_ptr<JobThread*

  while (true) {
    UniqueLock lock(mutex_);
    condition_.wait(lock, [this](){ return this->running_ || this->quitting_; });

    // Pool wants to destroy thread.
    if (quitting_) {
      pool_->ThreadFinished
      return;
    }

    }


    while
    while (!predicate) {

      }
    }

  }


}

}  // namespace om
