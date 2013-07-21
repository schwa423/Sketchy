//
//    threadloop.h
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: TLS
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__thread__threadloop__
#define __schwa__job01__thread__threadloop__

#include "job01/thread/synchronization.h"

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

/// ThreadLoop is a base-class that encapsulates a thread, and controls
/// execution of a subclass-provided RunLoop() function.  By giving control
/// to RunLoop(), and making it responsible for periodically checking whether
/// the loop is still running, ThreadLoop avoids invoking a virtual function
/// at every loop iteration, making it suitable for even very high-frequency
/// loops.
class ThreadLoop {
 public:
  virtual ~ThreadLoop();

  // Start the loop, if it isn't already running.
  // DO NOT call Start() from within a subclass constructor.
  void Start();
  // Stop the loop, if it isn't already stopped.
  void Stop();
  // Kill the loop permanently... destroys the thread.
  void Kill();

  /// Query whether the loop is running, stopped, or dead.
  bool IsRunning() const { return kRun == state_; }
  bool IsStopped() const { return kStop == state_; }
  bool IsDead() const { return kDead == state_; }

 protected:
  ThreadLoop();

  // ThreadLoop does nothing on its own.  Subclasses implement their behavior
  // by overriding these functions, which are executed in the worker thread:
  //   - SetupLoop()
  //   - RunLoop()
  //   - TeardownLoop()
  // TODO: elaborate / merge above and below
  // Keep looping until stopped or dead.
  // Subclasses are intended to override this function.  All subclasses
  // must follow these simple rules:
  //   - RunLoop() must frequently check IsRunning(),
  //     and return immediately if it is not.
  //   - do not call Run() in the constructor
  // TODO: better place for the comment above?
  virtual void OnLoopCreate() {}
  virtual void OnLoopStart() {}
  virtual void RunLoop() {}
  virtual void OnLoopStop() {}
  virtual void OnLoopUnstop() {}
  virtual void OnLoopDestroy() {}
  // TODO: no good reason for OnLoopUnstop() except for testing.

 private:
  enum State { kStop, kRun, kDead };

  // LoopControlFunc() is nested inside of LoopSetupFunc().
  // TODO: elaborate
  void ThreadFunc();
  void LoopUntilStopped();

  State   state_;
  Thread  thread_;
  Mutex   mutex_;
  Condition cond_;
};

}}}  // schwa::job01::thread ==================================================

#endif  // __schwa__job01__thread__threadloop__
