//
//    threadlocal.h
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__thread__threadlocal__
#define __schwa__job01__thread__threadlocal__

#include "job01/thread/synchronization.h"

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

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

}}}  // schwa::job01::thread ==================================================

#endif  // __schwa__job01__thread__threadlocal__
