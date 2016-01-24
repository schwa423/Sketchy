#include "qi/base/runner.h"

#include <future>
#include <memory>
#include <vector>

namespace qi {
namespace test {

// Return one of each of the support types of background-thread IRunner
// implementations supported by the current platform.
std::vector<std::unique_ptr<IRunner>> CreateBackgroundRunners();

// Utility class that blocks the specified number of "threads" in the specified
// IRunner.  These threse become unblocked when the returned promise is resolved
// (or broken).
std::promise<void> BlockRunnerThreads(IRunner* runner, size_t count);

}  // namespace test
}  // namespace qi
