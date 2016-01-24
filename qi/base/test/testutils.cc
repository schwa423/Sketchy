#include "qi/base/test/testutils.h"

#include "qi/base/runner.h"

namespace qi {
namespace test {

namespace {

struct BlockRunnerThreadsHelper {
  std::atomic<size_t> remaining;
  std::promise<void> all_blocked;
  std::shared_future<void> unblocker;
};

}  // anonymous namespace

std::promise<void> BlockRunnerThreads(IRunner* runner, size_t count) {
  ASSERT(count > 0);

  // Promise that will be returned to the caller after all threads are blocked,
  // allowing the caller to unblock them.
  std::promise<void> unblocker;
  auto helper = make_shared<BlockRunnerThreadsHelper>();
  helper->remaining = count;
  helper->unblocker = unblocker.get_future();
  auto all_blocked = helper->all_blocked.get_future();

  while (count--) {
    runner->Run([helper] () {
      // When all invocations of Run() have reached this point, resolve
      // |all_blocked| so that |unblocker| can be returned to the caller.
      if (0 == --helper->remaining) {
        helper->all_blocked.set_value();
      }
      // Wait for the caller to resolve the returned promise.
      helper->unblocker.wait();
    });
  }

  // Wait for all threads to be blocked, the return the unblocker to the caller.
  all_blocked.wait();
  return move(unblocker);
}

}  // namespace test
}  // namespace qi
