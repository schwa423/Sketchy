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

    // TODO: no cerr.

    // TODO: make Kill() idempotent, so subclasses can call it in their
    //       destructor, and so can their subclasses.
    cerr << "WARNING!!!  ThreadLoop destroyed without calling Kill()" << endl;
    Kill();
  }
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
  if (kRun == state_) {
    state_ = kStop;
  }
}

void ThreadLoop::Kill() {
  { Lock lock(mutex_);
    if (kDead == state_)
      return;
    state_ = kDead;
    // LoopUntilDead() might be waiting for condition.
    cond_.notify_one();
  }
  thread_.join();
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

  // Loop until dead.  At each loop iteration, either pass control to
  // the subclass RunLoop() function, or wait on a condition-variable
  // for the state to change from kStop.
  while (!kDead == state_) {
    UniqueLock lock(mutex_);

    switch (state_) {
      case kRun:
        // Lazily setup thread after the first time Start() was called.
        if (!was_setup) {
          was_setup = true;
          SetupLoop();
        }
        lock.unlock();  // ... so that we can be stopped/killed.
        RunLoop();
        break;
      case kStop:
        // Will be woken when either Start() or Kill() is called.
        cond_.wait(lock);
        break;
      case kDead:
        // No action required: next loop iteration will notice this.
        break;
    }
  }

  if (was_setup)
    TeardownLoop();
}

}}}  // schwa::job01::thread ==================================================
