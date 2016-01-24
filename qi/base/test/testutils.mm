#include "qi/base/test/testutils.h"

#include "qi/base/port_ios/runner_ios.h"
#include "qi/base/threadrunner.h"

namespace qi {
namespace test {

std::vector<std::unique_ptr<IRunner>> CreateBackgroundRunners() {
  std::vector<std::unique_ptr<IRunner>> runners;
  runners.push_back(std::make_unique<port::Runner_iOS>());
  runners.push_back(std::make_unique<ThreadRunner>());
  return runners;
}

}  // namespace test
}  // namespace qi
