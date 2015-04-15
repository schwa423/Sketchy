//
//    boss.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO:
//      - work on API to start/stop Workers
//      - unit test
//
//    IDEA:  Multiple Bosses which can be arranged in networks-of-control over
//           each other (and their subclasses)... I'm thinking:
//      - control over thread priorities
//      - debugger Boss controlling debugged Boss
//        - causing mode switches in the debugged Boss         
//        - IMPORTANT... mode switches in Workers can be simple/efficient as:
//          - each desired state is a virtual function that loops until it
//            notices that the "interrupted" flag was set... it then returns
//            control to the calling function
//          - the calling function is itself a loop (probably again implemented
//            in a virtual function)
//            - Turtles all the way down!  (although I think 2 levels should be
//              sufficient for to implement Worker)
//            - LESSON: judicious use of polymorphism needn't result in any
//              performance impact whatsoever 
//          - when control returns to the calling function, a metaprogramming
//            version of the change might look like:
//
//            template <typename NewState>            
//            future<bool> Worker::ChangeState(NewState&& new_state) {
//                // Double-dispatch with one virtual function,
//                // and one static overload based on argument type.            
//                _state = _state->ChangeTo(new_state);
//                _state->FinishChange();            
//            }
//
//////////////////////////////////////////////////////////////////////////////


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
class__cache_align Boss : public impl::BossImpl {
 public:
	Boss() {
 		for (int i = 0; i < NUM_WORKERS; i++) {
	 		auto& worker = _workers[i];
	 		auto queue  = _queues.elementAt(i);
	 		initializeWorker(worker, queue);
 		}
	}

    void startWorkers();
    void stopWorkers();

 private:
 	core::Ring<JobQueue, NUM_WORKERS> _queues;
    Worker _workers[NUM_WORKERS];
};


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__boss__