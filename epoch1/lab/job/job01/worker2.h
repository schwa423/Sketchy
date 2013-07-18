//
//    worker2.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__worker2__
#define __schwa__job01__worker2__

#include <thread>
typedef std::thread Thread;  // TODO: move this into job01::thread

#include "job01/thread/synchronization.h"
#include "job01/thread/threadloop.h"

#include "job01/mem/align.h"
using namespace schwa::job01::mem;  // TODO: cumbersome

#include <iostream>
using std::cerr;
using std::endl;


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

class Boss2;


template <typename T>
class ThreadLocal {
 public:
  ThreadLocal() : initialized_(false) {}

  T* Get();
  void Set(T* val);

 private:
  void Init();
  Mutex mutex_;
  bool  initialized_;
  pthread_key_t key_;

};

class align_cacheline Worker2 : public ThreadLoop {
    friend class Boss2;
 public:
  int ind() const { return index_; }

 protected:
  virtual void SetupLoop();
  virtual void RunLoop();
  virtual void TeardownLoop() {}  // TODO

 private:
  // Only bosses can create workers.
  Worker2(int index, Boss2& boss);
  virtual ~Worker2();

  static void HandleSignal(int sig);

  size_t   index_;
  Boss2&   boss_;
  Worker2* team_;
  bool     interrupted_;

  Mutex   worker_mutex_;

  // Worker* teammate(size_t index) { return }
};

class Boss2 {
    enum { kNumWorkers = 4 };
 public:
    Boss2() : workers_{{0, *this}, {1, *this} ,{2, *this}, {3, *this}} { }

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

    static ThreadLocal<Worker2> current_worker_;
};

// TODO: error handling
template <typename T>
T* ThreadLocal<T>::Get() {
  if (!initialized_)
    Init();
  return static_cast<T*>(pthread_getspecific(key_));
}

// TODO: error handling
template <typename T>
void ThreadLocal<T>::Set(T* value) {
  if (!initialized_)
    Init();
  pthread_setspecific(key_, value);
}

// TODO: error handling
template <typename T>
void ThreadLocal<T>::Init() {
  Lock lock(mutex_);
  if (!initialized_)
    return;
  int result = pthread_key_create(&key_, nullptr);
  if (result)
    cerr << "Failed to create TLS key due to error: " << result << endl;
}

}}  // schwa::job01 ===========================================================

#endif  // #ifndef __schwa__job01__worker2__
