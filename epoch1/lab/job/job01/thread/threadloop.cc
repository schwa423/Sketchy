//
//    threadloop.cc
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/thread/threadloop.h"

#include "job01/core/schwassert.h"

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

ThreadLoop::ThreadLoop() : state_(kNew),
                           thread_(&ThreadLoop::StateMachine, this) {}

ThreadLoop::~ThreadLoop() {
  if (!IsDead()) {
    // Kill() must be called before this point, since all subclass destructors
    // have already run, and therefore the object's vtable pointer is the one
    // defined by ThreadLoop.  As a result, we will not call any functions that
    // the subclass overrode, such as OnLoopDestroy().
    SCHWASSERT(IsDead(), "Kill() must be called before ~ThreadLoop().");
    // Better than nothing.
    Kill();
  } else if (thread_.joinable()) {
    SCHWASSERT(false, "Kill(false) was called, but not Kill().");
    // This is also not good... it means that the user called Kill(false),
    // but did not later call Kill().  Similar to the case above, the wrong
    // virtual functions may have been called.
    thread_.join();
  }
}

ThreadLoop& ThreadLoop::Init() {
  UniqueLock lock(mutex_);

  // Init() was already called.
  if (kNew != state_)
    return *this;

  // Wait until state changes, which means that OnLoopInit() has run.
  state_ = kInit;
  cond_.wait(lock, [this]{ return kInit != state_; });

  return *this;
}

void ThreadLoop::Start() {
  Lock lock(mutex_);
  AssertInitWasCalled();
  // Only run if we're stopped, not dead.
  if (kStop == state_) {
    state_ = kRun;
    cond_.notify_one();
  }
}

void ThreadLoop::Stop() {
  Lock lock(mutex_);
  AssertInitWasCalled();
  // Only stop if we're currently running.
  if (kRun == state_)
    state_ = kStop;
}

void ThreadLoop::Kill(bool join_thread) {
  { Lock l(mutex_);
    // No-op if not already initialized.
    if (!AssertInitWasCalled())
      return;

    // Kill if not already dead.
    if (kDead != state_) {
      state_ = kDead;
      cond_.notify_one();
    }
  }
  // Either block and wait for thread to complete, or don't.
  if (join_thread && thread_.joinable())
    thread_.join();
}

void ThreadLoop::StateMachine() {
  // Perform initial loop initialization.  See Init() comment.
  {
    // Wait until someone calls Init().
    while(true) {
      { Lock l(mutex_);
        if (kInit == state_)
          break;
      }
      std::this_thread::yield();
    }
    // Do initialization, then notify Init() that we've finished
    // via the condition-variable.
    OnLoopInit();
    state_ = kStop;
    cond_.notify_one();
  }

  // Record whether the most recent state was kRun.  This is used below to
  // decide whether to call OnLoopStop() and OnLoopStart().
  bool is_running = false;

  // Loop until dead.
  while (true) {
    UniqueLock lock(mutex_);
    switch (state_) {
      case kRun:
        if (!is_running) {
          // I wasn't running before, but I am now.
          is_running = true;
          OnLoopStart();
        }
        // Release the lock before running the loop, so that Start/Stop/Kill()
        // can be called without blocking... otherwise, since the typical usage
        // of Run() is to keep looping until IsRunning() returns false, we
        // would have a deadlock, and the thread would never stop.
        lock.unlock();
        Run();
        break;

      case kStop:
        if (is_running) {
          // I was running, but now I'm stopped.
          is_running = false;
          OnLoopStop();
        }
        // Sleep until woken by either Start() or Kill().
        cond_.wait(lock, [&]{ return kStop != state_; });
        // Unlock before invoking OnLoopUnstop() to support testing
        // via TurnTaker... otherwise deadlock will occur.
        lock.unlock();
        OnLoopUnstop();
        break;

      case kDead:
        // Ensure that there is always a matching OnLoopStop() invocation
        // for each call to OnLoopStart().
        if (is_running)
          OnLoopStop();
        // Perform any final teardown that must occure within the thread.
        OnLoopDestroy();
        // The thread is dead, baby.
        return;

      case kNew:
      case kInit:
        SCHWASSERT(false, "state should not be kNew or kInit here.");
        state_ = kStop;
        break;
    }
  }
}

bool ThreadLoop::AssertInitWasCalled() const {
  SCHWASSERT(kNew != state_,
             "Must call Init() before any other ThreadLoop function");
  return kNew != state_;
}

}}}  // schwa::job01::thread ==================================================
