#include "qi/disk/cache.h"

#include "qi/base/threadrunner.h"

namespace qi {
namespace disk {

Cache::Cache()
    : runner_(make_unique<ThreadRunner>()), work_scheduled_(false) {}

Cache::~Cache() {
  runner_->Shutdown();
}

void Cache::Read(Key key, ReadCallback callback) {
  LockGuard lock(mutex_);

  // First check whether |key| appears in |writes_| or |erases_|.  If so,
  // we can immediately fulfill the read-request without querying the
  // persistent storage.

  auto write_it = writes_.find(key);
  if (write_it != writes_.end()) {
    Data data(write_it->second);
    // Run in background so that the caller is guaranteed that it won't
    // run synchronously within the call to Read().
    Qi::Run([data = std::move(data), callback = std::move(callback)] () {
      callback(kSuccess, std::move(data));
    });
    return;
  }

  auto erase_it = erases_.find(key);
  if (erase_it != erases_.end()) {
    // Run in background so that the caller is guaranteed that it won't
    // run synchronously within the call to Read().
    Qi::Run([callback = std::move(callback)] () {
      callback(kNotFound, Data());
    });
    return;
  }

  // There was no write or erase corresponding to |key|, so we must query the
  // persistent storage.
  reads_.push_back({std::move(key), std::move(callback)});

  if (!work_scheduled_) {
    work_scheduled_ = true;
    runner_->Run([this] () { this->DoRunQueries(); });
  }
}

std::future<Data> Cache::Read(Key key) {
  auto p = std::make_shared<std::promise<Data>>();
  // TODO: remove |key| from capture-list.
  Read(key, [p, key] (ReadStatus status, Data data) {
    p->set_value(std::move(data));
  });
  return p->get_future();
}

void Cache::Write(Key key, Data data) {
  LockGuard lock(mutex_);
  erases_.erase(key);
  writes_[std::move(key)] = std::move(data);

  if (!work_scheduled_) {
    work_scheduled_ = true;
    runner_->Run([this] () { this->DoRunQueries(); });
  }
}

void Cache::Erase(Key key) {
  LockGuard lock(mutex_);
  writes_.erase(key);
  erases_.insert(std::move(key));

  if (!work_scheduled_) {
    work_scheduled_ = true;
    runner_->Run([this] () { this->DoRunQueries(); });
  }
}

void Cache::DoRunQueries() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!work_scheduled_) return;
  work_scheduled_ = false;

  // Obtain the list of reads/writes/erases to perform this time.
  std::vector<ReadRequest> reads{std::move(reads_)};
  std::unordered_map<Key, Data> writes{std::move(writes_)};
  std::unordered_set<Key> erases{std::move(erases_)};
  ASSERT(0 == reads_.size());
  ASSERT(0 == writes_.size());
  ASSERT(0 == erases_.size());

  // Unlock before calling RunQueries() so that Read()/Write()/Erase() can
  // be called without blocking.
  lock.unlock();
  this->RunQueries(std::move(reads), std::move(writes), std::move(erases));
}

void Cache::Flush() {
  auto p = std::make_shared<std::promise<void>>();
  runner_->Run([this, p] () {
    this->DoRunQueries();
    p->set_value();
  });
  p->get_future().wait();
}

}  // namespace disk
}  // namespace qi
