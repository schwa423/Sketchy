#include "qi/base/test/testutils.h"

#include "gtest/gtest.h"

namespace qi {
namespace test {

TEST(Runner, CreateAndDestroy) {
  auto runners = CreateBackgroundRunners();
  for (auto& runner : runners) {
    runner->Shutdown();
    runner.reset();
  }
}

TEST(Runner, ConcurrentSum) {
  auto runners = CreateBackgroundRunners();
  for (auto& runner : runners) {
    std::atomic<int> sum(0);

    ASSERT_TRUE(runner->Run([&sum](){ sum += 1; }));
    ASSERT_TRUE(runner->Run([&sum](){ sum += 2; }));
    ASSERT_TRUE(runner->Run([&sum](){ sum += 3; }));
    runner->Shutdown();
    ASSERT_FALSE(runner->Run([&sum](){ sum += 4; }));

    EXPECT_EQ(6, sum);
  }
}

}  // namespace test
}  // namespace qi
