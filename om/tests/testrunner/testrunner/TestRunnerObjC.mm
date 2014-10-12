#import "TestRunnerObjC.h"

#include <iostream>
#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace {
// C++ class that is held by the Obj-C class, and does all of the work.
class TestRunnerImpl {
public:
    TestRunnerImpl() {
        std::cerr << "instantiating TestRunnerImpl" << std::endl;

        static const int kMaxNameLength = 50;
        static const char* prog_name = "testrunner";
        char mutable_prog_name[kMaxNameLength];
        strncpy(mutable_prog_name, prog_name, kMaxNameLength);

        int argc = 1;
        char* argv0 = mutable_prog_name;
        ::testing::InitGoogleTest(&argc, &argv0);
    }
    ~TestRunnerImpl() {
        std::cerr << "destroying TestRunnerImpl" << std::endl;
    }
    int RunTests() {
        std::cerr << "TestRunnerImpl is doing its thing." << std::endl;
        ++run_count_;
        return RUN_ALL_TESTS();
    }
    int GetRunCount() const { return run_count_; }

private:
    int run_count_ = 0;
};
}  // anonymous namespace

// Extend TestRunnerInterface to add a private instance variable:
// a unique_ptr<TestRunnerImpl>.
@interface TestRunnerObjC()
{
    std::unique_ptr<TestRunnerImpl> runner_;
}
@end  // @interface TestRunnerObjC()


@implementation TestRunnerObjC

- (id)init {
    std::cerr << "instantiating TestRunner" << std::endl;
    runner_.reset(new TestRunnerImpl);
    return self;
}

- (NSNumber*)runTests {
    int result = runner_->RunTests();
    if (runner_->GetRunCount() > 10) {
        runner_.reset(new TestRunnerImpl);
    }
    return [NSNumber numberWithInt:result];
}

@end  // @implementation TestRunnerObjC
