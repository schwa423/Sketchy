#ifndef __om_base_JobThreadPool__
#define __om_base_JobThreadPool__

namespace om {

class JobThread;
typedef std::set<JobThread*> JobThreadSet;

class JobThreadPool {
 public:
  JobThreadPool();

  void SetDesiredActiveThreadCount(size_t desired_count);

 private:
  void MoveThread(const UniqueLock& lock, JobThread* thread,
                  JobThreadSet* source, JobThreadSet* destination);

  void AdjustActiveThreads(UniqueLock lock);

  int desired_active_thread_count_;

  JobThreadSet free_threads_;
  JobThreadSet active_threads_;
  JobThreadSet blocked_threads_;
  JobThreadSet dead_threads_;

  Mutex mutex_;
};



class JobThread final {
 public:
  ~JobThread()


 private:
  static void Run
  void Run();

  JobThread(JobThreadPool* owner);

  JobThreadPool* pool_;
  bool running_;
  bool quitting_;
  std::condition_variable condition_;
  Mutex& mutex_;
  std::thread thread_;


};





}  // namespace om

#endif  // __om_base_JobThreadPool__
