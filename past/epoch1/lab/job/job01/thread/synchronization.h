//
//    synchronization.h
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: consider splitting into different files... if I choose to do so,
//          I should take it as an opportunity to experiment with the impact
//          on compile-time.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__thread__synchronization__
#define __schwa__job01__thread__synchronization__

#include <mutex>
#include <condition_variable>
#include <thread>

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

typedef std::mutex              Mutex;
typedef std::lock_guard<Mutex>  Lock;
typedef std::unique_lock<Mutex> UniqueLock;
typedef std::condition_variable Condition;
typedef std::thread             Thread;

}}}  // schwa::job01::thread ==================================================

namespace schwa { namespace job01 {
    using namespace schwa::job01::thread;
}}

#endif  // __schwa__job01__thread__synchronization__
