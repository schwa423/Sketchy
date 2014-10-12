#import "TestRunner.h"

#include <iostream>
#include <memory>

// C++ class that is held by the Obj-C class, and does all of the work.
class TestRunnerImpl {
public:
    TestRunnerImpl() {
        std::cerr << "instantiating TestRunnerImpl" << std::endl;
    }
    ~TestRunnerImpl() {
        std::cerr << "destroying TestRunnerImpl" << std::endl;
    }
    int RunTests() {
        std::cerr << "TestRunnerImpl is doing its thing." << std::endl;
        ++run_count_;
        return 42;
    }
    int GetRunCount() const { return run_count_; }
    
private:
    int run_count_ = 0;
};




// Extend TestRunnerInterface to add a private instance variable:
// a unique_ptr<TestRunnerImpl>.
@interface TestRunner()
{
    std::unique_ptr<TestRunnerImpl> runner_;
}
@end  // @interface TestRunner()


@implementation TestRunner

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

@end  // @implementation TestRunner