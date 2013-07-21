//
//    test_threadloop.cc
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/thread/threadloop.h"

#include "job01/core/schwassert.h"
#include "job01/dbg/dbg.h"
#include "job01/thread/turntaker.h"
#include "job01/time/time_util.h"
using namespace schwa::job01;
using namespace schwa::job01::dbg;
using namespace schwa::job01::thread;

#include <atomic>
#include <cstddef>
#include <iostream>
using namespace std;

class TestThreadLoop : public ThreadLoop, public TurnTaker {
 public:
  TestThreadLoop(Mutex& shared_mutex, bool take_turns)
      : TurnTaker(shared_mutex),
        take_turns_(take_turns),
        was_setup_(false),
        start_count_(0),
        run_count_(0),
        stop_count_(0)
        {}

  int GetStartCount() const { return start_count_; }
  int GetRunCount() const { return run_count_; }
  int GetStopCount() const { return stop_count_; }

  bool WasSetup() const { return was_setup_; }

 protected:
  virtual void OnLoopCreate() {
    was_setup_ = true;
  }
  virtual void OnLoopStart() {
    ++start_count_;
  }
  virtual void RunLoop() {
    int iterations = 0;
    while (IsRunning()) {
      if (take_turns_) {
        TakeTurn([this, &iterations](){
          ++run_count_;
        });
      } else {
        ++run_count_;
      }
    }
  }
  virtual void OnLoopStop() {
    UniqueLock lock(TurnTaker::mutex_);
    ++stop_count_;

    // We can't take a full turn here, because the ThreadLoop
    // mutex is locked until we reach the condition variable,
    // so if someone outside were to call Start() or Kill()
    // before telling us it's our turn, we would deadlock.
    // Instead, lock here and notify the next turn-taker, but
    // don't wait ourselves until OnLoopUnstop(), where we
    // simply call PassTurn().
    if (take_turns_)
      NotifyNextTurnTaker(lock);
  }

  virtual void OnLoopUnstop() {
    // See comment in OnLoopStop().
    if (take_turns_)
      PassTurn();
  }

 private:
  bool take_turns_;
  bool was_setup_;
  int  start_count_;
  int  run_count_;
  int  stop_count_;
};

// Helper function to reduce verbosity.
void AssertLoopStatistics(TestThreadLoop& loop,
                          bool expected_was_setup,
                          int expected_start_count,
                          int expected_run_count,
                          int expected_stop_count) {
  bool actual_was_setup = loop.WasSetup();
  int actual_start_count = loop.GetStartCount();
  int actual_run_count = loop.GetRunCount();
  int actual_stop_count = loop.GetStopCount();

  // cerr << "AssertLoopStatistics(" << actual_was_setup
  //      << ", " << actual_start_count
  //      << ", " << actual_run_count
  //      << ", " << actual_stop_count
  //      << ")" << endl;

  SCHWASSERT(expected_was_setup   ==  actual_was_setup,
             "unexpected loop setup state");
  SCHWASSERT(expected_start_count ==  actual_start_count,
             "unexpected start count");
  SCHWASSERT(expected_run_count   ==  actual_run_count,
             "unexpected run count");
  SCHWASSERT(expected_stop_count  ==  actual_stop_count,
             "unexpected stop count");
}

void test_StartupWaitsForFirstRun() {
  Mutex          mutex;
  TestThreadLoop loop(mutex, true);
  TurnTaker      tester(mutex);

  loop.SetNextTurnTaker(&tester);
  tester.SetNextTurnTaker(&loop);

  // Wait a moment and ensure that SetupLoop() has not yet been run.
  // TODO: the test will fail if you remove this, I think because the
  // loop.Start() occurs befor the thread-loop begins, and therefore
  // is already set up by the time the second call to AssertLoopStatistics()
  // happens, which expects it to not be set up yet.
  this_thread::sleep_for(time::msecs(10));
  SCHWASSERT(!loop.WasSetup(),
             "SetupLoop() should not run until after the loop is started.");

  // Start loop and wait.
  tester.TakeTurn([&loop]{
    loop.Start();
    AssertLoopStatistics(loop, false, 0, 0, 0);
  });

  // Loop is now blocked in OnLoopUnstop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, false, 0, 0, 0);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 1, 1, 0);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    loop.Stop();
    AssertLoopStatistics(loop, true, 1, 2, 0);
  });

  // Loop is now blocked on its condition-variable,
  // waiting for its state to change to kRun or kDead.
  tester.TakeTurn([&loop]{
    loop.Start();
    AssertLoopStatistics(loop, true, 1, 2, 1);
  });

  // Loop is now blocked in OnLoopUnstop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 1, 2, 1);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 2, 3, 1);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 2, 4, 1);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    loop.Stop();
    AssertLoopStatistics(loop, true, 2, 5, 1);
  });

  // Loop is now blocked on its condition-variable,
  // waiting for its state to change to kRun or kDead.
  tester.TakeTurn([&loop]{
    loop.Start();
    AssertLoopStatistics(loop, true, 2, 5, 2);
  });

  // Loop is now blocked in OnLoopUnstop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 2, 5, 2);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 3, 6, 2);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 3, 7, 2);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  tester.TakeTurn([&loop]{
    AssertLoopStatistics(loop, true, 3, 8, 2);
  });

  // Loop is now blocked in RunLoop(), waiting for its turn.
  loop.Kill();
  AssertLoopStatistics(loop, true, 3, 9, 2);

  // Loop is now blocked in OnLoopUnstop(), waiting for its turn.
  loop.Notify();
}


// Verify that the actual number of times that the loop
// is started matches the number of times that it is stopped.
void test_StopsMatchStarts() {
  Mutex          mutex;
  TestThreadLoop loop(mutex, false);

  // Randomly start and stop.
  for (int i = 0; i < 10000; ++i) {
    if (rand() % 2 == 0)
      loop.Start();
    else
      loop.Stop();

    // Occasionally sleep for a moment.
    if (rand() % 100 > 97)
      this_thread::sleep_for(time::msecs(rand() % 10));
  }
  loop.Stop();

  loop.Kill();

  SCHWASSERT(loop.GetStartCount() == loop.GetStopCount(),
             "there should be the same number of starts and stops.");
}


int main(void) {
  time::msecs elapsed;
  {
    ScopeTimer time([&elapsed](time::msecs val){
      elapsed = val;
    });

    test_StartupWaitsForFirstRun();
    test_StopsMatchStarts();
  }

  cerr << "job01/thread/test_threadloop...  PASSED!  (" << elapsed << ")" << endl << endl;
}
