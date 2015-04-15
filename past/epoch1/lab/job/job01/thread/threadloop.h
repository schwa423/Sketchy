//
//    threadloop.h
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__thread__threadloop__
#define __schwa__job01__thread__threadloop__

#include "job01/thread/synchronization.h"

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

/// ThreadLoop is a base-class that encapsulates a thread, and controls
/// execution of a subclass-provided Run() function.  The state of the
/// ThreadLoop is controlled by the following functions, which are documented
/// later:
///   - Init()
///   - Start()
///   - Stop()
///   - Kill()
///
/// Subclasses may customize the behavior of ThreadLoop by overriding (some of)
/// the following virtual functions:
///   - Run()
///   - OnLoopInit()
///   - OnLoopStart()
///   - OnLoopStop()
///   - OnLoopDestroy()
///
/// Run() is the most important of these, and is the only one which subclasses
/// are required to implement.  Run() must periodically return control to the
/// ThreadLoop state-machine.  Two main approaches are anticipated:
///   - Run() does a single action, then returns.
///   - Run() implements an inner-loop which periodically checks IsRunning(),
///     and returns control to ThreadLoop only when it becomes false.
/// The benefit of the second approach is efficiency... implementing a loop
/// in Run() avoids the costs associated with returning control to ThreadLoop
/// (mutex locks, virtual function calls, etc.) and can therefore be used for
/// demanding applications such as work-stealing task-schedulers. ;-)
///
/// Next most important are OnLoopInit() and OnLoopDestroy()... these are the
/// hooks that a subclass would use to manage thread-local-storage, if desired.
///
/// OnLoopStart() and OnLoopStop() are mostly used for testing, but subclasses
/// may find other uses for them (and possibly even for OnLoopUnstop()).
class ThreadLoop {
 public:
  virtual ~ThreadLoop();

  // Users of ThreadLoop must call Init() before calling Start() or Kill().
  // The calling thread blocks until OnLoopInit() is called in the loop-thread.
  // This is necessary because initialization cannot be done completely within
  // the ThreadLoop constructor... there would be no way to guarantee that all
  // subclass constructors had run by the time that OnLoopInit() runs; in this
  // case, the correct vtable pointer would not be installed in the object, and
  // the wrong implementation of OnLoopInit() would be invoked.
  //
  // Returns the loop itself to make it convenient to call Start() afterward:
  //   myLoop.Init().Start();
  //
  // Idempotent.  There is no reason to call Init() multiple times, but also
  // no harm in doing so.
  ThreadLoop& Init();

  // Start the loop, if it isn't already running.
  void Start();

  // Stop the loop, if it isn't already stopped.
  void Stop();

  // Kill the loop permanently, causing the thread be destroyed.
  //
  // The default is to block until the thread has been joined, but sometimes
  // this is not acceptable (see test_threadloop.cc for an example).
  // To return without waiting for the thread to finish, use Kill(false).
  // IMPORTANT: Kill() must still be called before the ThreadLoop is destroyed;
  // otherwise incorrect virtual functions may be invoked.  Eg:
  //   for (ThreadLoop* loop : loops) { loop->Kill(false); }
  //   // do other cleanup for a while
  //   for (ThreadLoop* loop : loops) { loop->Kill(); delete loop; }
  void Kill(bool join_thread = true);

  /// Query whether the loop is running, stopped, or dead.
  bool IsRunning() const { return kRun == state_; }
  bool IsStopped() const { return kStop == state_; }
  bool IsDead() const { return kDead == state_; }

 protected:
  // Cannot instantiate ThreadLoop; use a subclass instead.
  ThreadLoop();

  // ThreadLoop does nothing on its own.  Subclasses customize their behavior
  // by overriding the functions below, which are invoked by StateMachine()...
  // they should not be called directly by subclasses.
  virtual void Run() = 0;
  virtual void OnLoopInit() {}
  virtual void OnLoopStart() {}
  virtual void OnLoopStop() {}
  virtual void OnLoopDestroy() {}
  // Subclasses may override this, but the only known use-case is testing.
  virtual void OnLoopUnstop() {}

 private:
  enum State { kNew, kInit, kStop, kRun, kDead };

  // StateMachine() implements the state-machine for ThreadLoop, reacting
  // to calls to Init(), Start(), Stop(), and Kill().
  void StateMachine();

  // Assert that Init() was called before this function.
  bool AssertInitWasCalled() const;

  State     state_;
  Thread    thread_;
  Mutex     mutex_;
  Condition cond_;
};

}}}  // schwa::job01::thread ==================================================

#endif  // __schwa__job01__thread__threadloop__
