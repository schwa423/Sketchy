#ifndef __om_base_JobRunner__
#define __om_base_JobRunner__

#include <deque>

#include "om/base/job/job.h"
#include "om/base/thread/mutex.h"

namespace om {

class JobQueue;
typedef std::shared_ptr<JobQueue> JobQueuePtr;

class JobRunner {
 public:
  JobRunner();
  ~JobRunner();

  // Allow others to obtain the runner's JobQueue, in order that they may
  // enqueue Jobs to run.
  JobQueuePtr GetJobQueue() const { return queue_; }

  void RunNextJob();

 private:
  JobQueuePtr queue_;
  Mutex& mutex_;
};

typedef std::shared_ptr<JobRunner> JobRunnerPtr;




//**** put in own file
class JobQueue {
 public:
  ~JobQueue();

  // Enqueue Job, and notify our JobRunner (if any).
  void AddJob(JobPtr&& job);
  void AddJob(const JobPtr& job);

 private:
  friend class JobRunner;

  JobQueue(JobRunner* runner);

  // Pop and return the next Job from the queue, if any.  Called by JobRunner.
  JobPtr GetNextJob();
  // Each JobRunner is associated with a queue for its entire lifetime.
  // Set and cleared by the JobRunner constructor/desctructor, respectively.
  JobRunner* runner_;
  // Synchronize access to the queue.  Also used by JobRunner.
  Mutex mutex_;
  // Hold enqueued Jobs.
  std::deque<JobPtr> jobs_;
};



/*



typedef std::set<JobQueuePtr> JobQueueSet;



// Thread-safety: unsafe.  None of the methods may be called concurrently,
// unless otherwise noted.
class JobPuller final {
 public:
  JobPuller() : job_count_(0) {}
  ~JobPuller() { RemoveAllQueues(); }

  JobPtr GetNextJob

  void AddQueue(JobQueuePtr queue);
  // Add |queue| to the puller, unless it is already present.
  // Queue must not already be attached to another puller.

  void RemoveQueue(JobQueuePtr queue);
  // Remove |queue| from the puller.  No-op if the queue was not already present.

  void RemoveAllQueues();
  // Detach all queues from the puller.

 private:
  friend class JobQueue;

  JobQueueSet queues_;
  std::atomic<int> job_count_;
};

void JobPuller::AddQueue(JobQueuePtr q) {
  auto it = queues_.find(q);
  if (it == queues_.end()) {
    queues_.insert(q);

    Lock lock(q->mutex_);
    DCHECK(!q->puller_);
    q->puller_ = this;
    job_count_ += q->jobs_.size();
  }
}

void JobPuller::RemoveQueue(JobQueuePtr q) {
  auto it = queues_.find(q);
  if (it != queues_.end()) {
    queues_.erase(it);

    Lock lock(q->mutex_);
    DCHECK_EQ(this, q->puller_);
    q->puller_ = nullptr;
    job_count_ -= q->jobs_.size();
  }
}

void JobPuller::RemoveAllQueues() {
  while (queues_.size())
    RemoveQueue(*queues_.begin());
}















class JobThread {

 private:

  void Loop() {

    scheduler_->

    JobPtr job(puller_->GetNext());
    if (job)
  }

  void YieldDuring(std::function func) {

  }
};




class Thread {


};


JobQueue() {
  std::deque<JobPtr> queued_jobs_;
  std::deque<Thread> unblocked_threads_;
}




class ThreadPool {


 ThreadList


};



class ThreadList {
  std::deque<std::unique_ptr<Thread>>


};

*/

}  // namespace om

#endif  // __om_base_JobRunner__
