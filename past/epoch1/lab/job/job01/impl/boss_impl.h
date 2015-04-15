//
//  boss_impl.h
//  schwa::job01::impl
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//  *** TODO ***
//      - just a sketch
//


#ifndef __schwa__job01__boss_impl__
#define __schwa__job01__boss_impl__


#include "job01/host/host.h"
#include "job01/jobqueue.h"
#include "job01/worker.h"

#include "job01/mem/align.h"


// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {


class__cache_align  BossImpl {
 protected:
	void initializeWorker(Worker& worker, JobQueue* queue) {
		worker.initialize(queue);
	}
};


}}}  // schwa::job01::impl =====================================================


#endif  // #ifndef __schwa__job01__boss__