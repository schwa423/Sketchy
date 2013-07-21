//
//    threadloop.cc
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/thread/threadloop.h"

// TODO: remove this
#include <iostream>
using std::cerr;
using std::endl;

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

ThreadLoop::ThreadLoop() : state_(kStop),
                           thread_(&ThreadLoop::ThreadFunc, this) {}

ThreadLoop::~ThreadLoop() {
  if (!IsDead()) {
    // Freak out!  Need to call Kill() before destruction, since
    // calling it during would happen after the subclass destructor
    // has been run, and therefore after the subclass's virtual function
    // table has been replaced by the superclass's.
    //
    // Note that Kill() is idempotent, so subclasses can call it in their
    // destructor, as can their subclasses.
    // TODO: make this a DCHECK().
    cerr << "WARNING!!!  ThreadLoop destroyed without calling Kill()" << endl;
    Kill();
  }
  thread_.join();
}

void ThreadLoop::Start() {
  Lock lock(mutex_);
  // Only run if we're stopped, not dead.
  if (kStop == state_) {
    state_ = kRun;
    cond_.notify_one();
  }
}

void ThreadLoop::Stop() {
  Lock lock(mutex_);
  if (kRun == state_)
    state_ = kStop;

}

void ThreadLoop::Kill() {
  { Lock lock(mutex_);
    if (kDead == state_)
      return;
    state_ = kDead;
    // LoopUntilDead() might be waiting for condition.
    cond_.notify_one();
  }
}

// TODO: is there a race condition in the state-management code?
//       how difficult to fix?  consequences?
//       I think yes, and here's how to fix it... first thing, grab
//       the mutex and signal the cond-var.
void ThreadLoop::ThreadFunc() {

  // Tricky start-up logic to ensure that the loop is not initialized
  // until the ThreadLoop subclass is completely constructed.  This is
  // crucial... otherwise the subclass-overridable virtual functions
  // might be looked up in the wrong virtual function table!
  bool was_setup = false;
  bool is_running = false;

  // Loop until dead.  At each loop iteration, either pass control to
  // the subclass RunLoop() function, or wait on a condition-variable
  // for the state to change from kStop.
  while (true) {
    UniqueLock lock(mutex_);

    State state = state_;
    switch (state) {
      case kRun:
        if (!was_setup) {
          // Lazily setup thread after the first time Start() was called.
          was_setup = true;
          OnLoopCreate();
        }
        if (!is_running) {
          // I wasn't running, but I am now.
          is_running = true;
          OnLoopStart();
        }
        // Release the lock before running the loop.
        // This allows me to be stopped/killed.
        lock.unlock();
        RunLoop();
        break;

      case kStop:
        if (is_running) {
          // I was running, but now I'm stopped.
          is_running = false;
          OnLoopStop();
        }
        // I'll be awoken when either Start() or Kill() is called.
        cond_.wait(lock, [&]{ return kStop != state_; });
        // Unlock the lock before invoking OnLoopUnstop() to support
        // testing via TurnTaker... otherwise deadlock will occur.
        lock.unlock();
        OnLoopUnstop();
        break;

      case kDead:
        if (is_running)
          OnLoopStop();
        if (was_setup)
          OnLoopDestroy();

        // The thread is dead, baby.
        return;
    }
    std::this_thread::yield();
  }
}

}}}  // schwa::job01::thread ==================================================
