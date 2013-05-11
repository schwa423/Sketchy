//
//  worker.h
//  schwa::job01
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//  *** TODO **
//      - just a sketch... once we have JobQueue finished and tested
//        it will be time to think about the interface to Worker.
//


#ifndef __schwa__job01__worker__
#define __schwa__job01__worker__


#include "job01/core/ring.h"
#include "job01/host/host.h"


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

namespace impl {
	// Forward declaration.
	class BossImpl;
}


class Worker : public core::RingElement<Worker, host::CACHE_LINE_SIZE> {
 public:
 	Worker() : _queue(nullptr) { }

 private:
 	void initialize(JobQueue* queue) { 
 		assert(_queue == nullptr);
 		_queue = queue;
 	}

	friend class impl::BossImpl;

 private:
 	JobQueue* _queue;
};


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__worker__