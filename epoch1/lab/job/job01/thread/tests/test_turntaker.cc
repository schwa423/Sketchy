//
//    test_turntaker.cc
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/thread/turntaker.h"

#include "job01/core/schwassert.h"
#include "job01/dbg/dbg.h"
using namespace schwa::job01;
using namespace schwa::job01::dbg;
using namespace schwa::job01::thread;

#include <iostream>
using namespace std;

// Encapsulates a simple thread-loop that takes runs
// one loop iteration each turn.
class TestTurnTaker : public TurnTaker {
 public:
  TestTurnTaker(Mutex& mutex) : TurnTaker(mutex),
                                stopped_(true),
                                finished_(false),
                                num_iterations_(0) {}
  ~TestTurnTaker() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void Start() {
    SCHWASSERT(stopped_, "Thread was already started.");
    stopped_ = false;
    finished_ = false;
    thread_ = Thread(&TestTurnTaker::ThreadFunc, this);
  }
  bool Stop() { stopped_ = true; }
  bool IsStopped() { return stopped_; }
  bool IsFinished() { return finished_; }

  int GetNumIterations() const { return num_iterations_; }

 private:
  void ThreadFunc() {
    while (!stopped_) {
      TakeTurn([this]{ ++num_iterations_; });
    }
    finished_ = true;

    // Don't use PassTurn(), because we would block forever.
    UniqueLock lock(mutex_);
    NotifyNextTurnTaker(lock);
  }

  bool stopped_;
  bool finished_;
  int  num_iterations_;
  Thread thread_;
};

void test_TurnTaker(void) {
  Mutex mutex;
  TurnTaker me(mutex);
  TestTurnTaker others[3]{{mutex}, {mutex}, {mutex}};

  // Set it up so that my turn is next, after any other TurnTaker takes a turn.
  // We do this so that when we start the threads below, we are notified once
  // each has successfully started.
  others[0].SetNextTurnTaker(&me);
  others[1].SetNextTurnTaker(&me);
  others[2].SetNextTurnTaker(&me);

  // Start threads.
  {
    // Note: this call to SetNextTurnTaker() is unnessary, as are the others
    // in this scope.  They are included to illustrate a point, which will
    // be developed below...
    me.SetNextTurnTaker(&others[0]);

    // Because we earlier called:
    //   others[0].SetNextTurnTaker(&me);
    // we are guaranteed that once we block at the end of TakeTurn(), we will
    // will be woken by the TakeTurn() in TestTurnTaker::ThreadFunc().  Note
    // that this would happen regardless of whether we called:
    //   me.SetNextTurnTaker(&others[0]);
    // (read on...)
    me.TakeTurn([&]{ others[0].Start(); });

    SCHWASSERT(others[0].GetNumIterations() == 1, "expected 1 iteration");
    SCHWASSERT(others[1].GetNumIterations() == 0, "expected 0 iterations");
    SCHWASSERT(others[2].GetNumIterations() == 0, "expected 0 iterations");

    // However, because we did call:
    //   me.SetNextTurnTaker(&others[0]);
    // is is now "necessary" to call
    //   me.SetNextTurnTaker(&others[1]);
    // below (i.e. otherwise the assertions would fail, because once others[1]
    // tells us it is our turn, we would tell others[0] to run again, and as a
    // result GetNumIterations() would return 2).
    me.SetNextTurnTaker(&others[1]);
    me.TakeTurn([&]{ others[1].Start(); });
    me.SetNextTurnTaker(&others[2]);
    me.TakeTurn([&]{ others[2].Start(); });

    SCHWASSERT(others[0].GetNumIterations() == 1, "expected 1 iteration");
    SCHWASSERT(others[1].GetNumIterations() == 1, "expected 1 iterations");
    SCHWASSERT(others[2].GetNumIterations() == 1, "expected 1 iterations");
  }

  // Use SetNextTurnTaker() to control which threads are given turns
  // to run another loop iteration.
  {
    me.SetNextTurnTaker(&others[0]);
    me.PassTurn();
    me.PassTurn();
    me.SetNextTurnTaker(&others[1]);
    me.PassTurn();

    SCHWASSERT(others[0].GetNumIterations() == 3, "expected 3 iteration");
    SCHWASSERT(others[1].GetNumIterations() == 2, "expected 2 iterations");
    SCHWASSERT(others[2].GetNumIterations() == 1, "expected 1 iterations");
  }

  // Use SetNextTurnTaker() to control which threads are stopped; once
  // their stopped_ flag is set, they're notified that it's their turn,
  // and once they exit their loop, they tell us that it's our turn again.
  {
    me.SetNextTurnTaker(&others[0]);
    me.TakeTurn([&]{ others[0].Stop(); });

    SCHWASSERT( others[0].IsFinished(), "should be finished");
    SCHWASSERT(!others[1].IsFinished(), "should not be finished");
    SCHWASSERT(!others[2].IsFinished(), "should not be finished");

    me.SetNextTurnTaker(&others[1]);
    me.TakeTurn([&]{ others[1].Stop(); });
    me.SetNextTurnTaker(&others[2]);
    me.TakeTurn([&]{ others[2].Stop(); });

    SCHWASSERT(others[0].IsFinished(), "should be finished");
    SCHWASSERT(others[1].IsFinished(), "should be finished");
    SCHWASSERT(others[2].IsFinished(), "should be finished");
  }
}

int main(void) {
  time::msecs elapsed;
  {
    ScopeTimer([&elapsed](time::msecs val){ elapsed = val; });
    test_TurnTaker();
  }

  cerr << "job01/thread/test_turntaker...  PASSED!  (" << elapsed << ")" << endl << endl;
  return 0;
}
