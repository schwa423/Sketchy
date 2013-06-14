//
//    worker.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Simple version, just to get things up and running.  Manages a thread.
//    Loops on a queue, pulling jobs off to execute.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__worker__
#define __schwa__job01__worker__


#include "job01/host/host.h"
#include "job01/impl/jobpool.h"
#include "job01/jobqueue.h"

#include <mutex>
#include <thread>


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

namespace impl {
	// Forward declaration.
	class BossImpl;
}


class__cache_align Worker {
 public:
 	Worker();

 private:
 	void initialize(JobQueue* queue) { 
 		assert(_queue == nullptr);
 		_queue = queue;
 	}

	friend class impl::BossImpl;

    void start();
    void stop();

 private:
    void run();

    virtual void processJobs();

    enum WorkerState { kProcessing, kStopped };

 	JobQueue*     _queue;
    impl::JobPool _pool;
    std::mutex    _mutex;
    bool          _interrupted;
    bool          _is_stopped;
    WorkerState   _state;
    std::thread   _thread;
};


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__worker__