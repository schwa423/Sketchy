//
//    boss2.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__boss2__
#define __schwa__job01__boss2__

#include "job01/worker2.h/synchronization.h"
#include "job01/thread/threadlocal.h"

// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

class Boss2 {
    enum { kNumWorkers = 4 };
 public:
    Boss2() : workers_{{0, *this}, {1, *this} ,{2, *this}, {3, *this}} { }

    ~Boss2() {
      // Non-blocking Kill() to get 'em all started.
      for (auto& w : workers_)
        w.Kill(false);

      // Blocking Kill() to finish 'em off.
      for (auto& w : workers_)
        w.Kill();
    }

    void Start() {
      for (auto& w : workers_)
        w.Start();
    }

    void Stop() {
      for (auto& w : workers_)
        w.Stop();
    }

    static Worker2* GetCurrentWorker() { return current_worker_.Get(); }
    static void SetCurrentWorker(Worker2* worker) {
      current_worker_.Set(worker);
    }

 private:
    Worker2 workers_[kNumWorkers];

    int requests_[kNumWorkers];

    int responses_[kNumWorkers];


    static ThreadLocal<Worker2> current_worker_;
};



}}  // schwa::job01 ===========================================================

#endif  // #ifndef __schwa__job01__boss2__
