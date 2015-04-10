#include "om/base/job/jobrunner.h"

#include "om/base/util/dcheck.h"

namespace om {

JobRunner::JobRunner()
    : queue_(new JobQueue(this)), mutex_(queue_->mutex_) {
}

JobRunner::~JobRunner() {
  Lock lock(mutex_);
  queue_->runner_ = nullptr;
  queue_.reset();
}

void JobQueue::AddJob(JobPtr&& job) {
  //**** TODO: ensure that Job is OK to enqueue.
  Lock lock(mutex_);
  jobs_.push_back(job);
}

void JobQueue::AddJob(const JobPtr& job) {
  //**** TODO: ensure that Job is OK to enqueue.
  Lock lock(mutex_);
  jobs_.push_back(job);
}

JobPtr JobQueue::GetNextJob() {
  Lock lock(mutex_);
  JobPtr result;
  if (!jobs_.empty()) {
    std::swap(result, jobs_.front());
    jobs_.pop_front();
  }
  return result;
}

JobQueue::JobQueue(JobRunner* runner) : runner_(runner) {
}

JobQueue::~JobQueue() {
  DCHECK(!jobs_.size());
}

}  // namespace om
