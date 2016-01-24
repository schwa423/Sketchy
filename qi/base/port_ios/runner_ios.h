#ifndef _QI_BASE_PORT_Runner_iOS_h_
#define _QI_BASE_PORT_Runner_iOS_h_

#include "qi/base/runner.h"

#include <dispatch/dispatch.h>

namespace qi {
namespace port {

class Runner_iOS : public IRunner {
 public:
  Runner_iOS();
  Runner_iOS(dispatch_queue_t queue);
  ~Runner_iOS() override;

  bool Run(std::function<void()> job) override;
  void Shutdown() override;

 private:
  dispatch_queue_t queue_;
  std::atomic_bool shutdown_;
  std::atomic<int> job_count_;
};

}  // namespace port
}  // namespace qi

#endif  // _QI_BASE_PORT_Runner_iOS_h_
