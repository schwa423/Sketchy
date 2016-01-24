#include "qi/base/port_ios/runner_ios.h"
#include "qi/base/timetypes.h"

#include <thread>

namespace qi {
namespace port {

Runner_iOS::Runner_iOS()
    : queue_(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)),
      shutdown_(false),
      job_count_(0) {}

Runner_iOS::Runner_iOS(dispatch_queue_t queue)
    : queue_(queue), shutdown_(false), job_count_(0) {}

Runner_iOS::~Runner_iOS() {
  // Assert that Shutdown() has already been called in order to avoid
  // race-conditions where background threads try to enqueue more work.
  ASSERT(shutdown_);
}

bool Runner_iOS::Run(std::function<void()> job) {
  // TODO: this should be OK if Run() is called from closures run by the same
  // runner, but there could be problems if run from an unrelated thread (since
  // the runner might be on the verge of destruction).
  ++job_count_;
  if (shutdown_) {
    --job_count_;
    return false;
  } else {
    dispatch_async(queue_, ^{
      job();
      --job_count_;
    });
    return true;
  }
}

void Runner_iOS::Shutdown() {
  ASSERT(!shutdown_);
  shutdown_ = true;
  while (job_count_ > 0) {
    std::this_thread::sleep_for(10ms);
  }
}

}  // namespace port
}  // namespace qi
