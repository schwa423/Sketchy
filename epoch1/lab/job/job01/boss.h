//
//  boss.h
//  schwa::job01
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//  *** TODO ***
//      - work on API to start/stop Workers
//      - unit-test
//


#ifndef __schwa__job01__boss__
#define __schwa__job01__boss__

#include "job01/impl/boss_impl.h"
#include "job01/core/ring.h"
#include "job01/jobqueue.h"
#include "job01/worker.h"
#include "mem00/align.h"


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


template <int NUM_WORKERS>
class CACHE_ALIGNED Boss : public impl::BossImpl {
 public:
	Boss() {
 		for (int i = 0; i < NUM_WORKERS; i++) {
	 		auto worker = _workers.elementAt(i);
	 		auto queue  = _queues.elementAt(i);
	 		initializeWorker(worker, queue);
 		}
	}

 private:
 	core::Ring<JobQueue, NUM_WORKERS> _queues;
 	core::Ring<Worker, NUM_WORKERS>   _workers;
};


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__boss__