#ifndef _QI_BASE_ThreadRunner_h_
#define _QI_BASE_ThreadRunner_h_

#include "qi/base/runner.h"

#include <condition_variable>
#include <deque>
#include <thread>
#include <vector>

namespace qi {

class ThreadRunner final : public IRunner {
 public:
  ThreadRunner(size_t thread_count = 1);
  ~ThreadRunner() override;

  bool Run(std::function<void()> job) override;
  void Shutdown() override;

 private:
  static void Loop(ThreadRunner* runner);

  Mutex mutex_;
  std::condition_variable condition_;
  std::deque<std::function<void()>> jobs_;
  std::vector<std::thread> threads_;
  bool shutdown_;
};

}  // namespace qi

#endif  // _QI_BASE_ThreadRunner_h_
