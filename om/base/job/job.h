#ifndef __om_base_Job__
#define __om_base_Job__

#include "om/base/thread/mutex.h"

namespace om {

//**** move to own file
typedef std::mutex Mutex;

class JobQueue;

class Job {
 public:
  Job();
  virtual ~Job();

 protected:
/****
  template<typename T>
  void Wait(std::future<T>& fut);
*/
 private:
  friend class JobRunner;

  Mutex mutex_;
  JobQueue* queue_;

//****  std::atomic<int> flag_;
};

typedef std::shared_ptr<Job> JobPtr;


/*****
template<typename T>
void Job::Wait(std::future<T>& fut) {
  JobRunnerProxy* runner = GetJobRunner();
  JobThread* thread = runner->GetJobThread();
  thread->YieldDuring([&](){ fut.wait(); });
}
*/



}  // namespace om

#endif  // __om_base_Job__
