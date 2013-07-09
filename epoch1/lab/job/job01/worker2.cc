//
//    worker2.cc
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: test plan
//      -
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/worker2.h"

// TODO: remove this
#include <iostream>
using std::cerr;
using std::endl;

// schwa::job01 ===============================================================
namespace schwa { namespace job01 {




Worker2::Worker2(int index, Boss2& boss) : index_(index),
                                           boss_(boss),
                                           team_(nullptr), // TODO
                                           interrupted_(false) {}

Worker2::~Worker2() {
  Kill();
}

void Worker2::SetupLoop() {
  Boss2::SetCurrentWorker(this);

  pthread_sigmask
}

void Worker2::RunLoop() {
  while(true) {
    if (interrupted_) {
      Lock lock(worker_mutex_);
      interrupted_ = false;
      if (!IsRunning())
        return;

      // TODO: interworker communication and other interrupt processing
    }

    // TODO: get and run next job
  }
}


// static variable
ThreadLocal<Worker2> Boss2::current_worker_;


}}  // schwa::job01 ===========================================================
