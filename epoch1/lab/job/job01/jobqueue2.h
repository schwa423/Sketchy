//
//    jobqueue2.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    *** TODO ***
//        - unit test !!!
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__jobqueue2__
#define __schwa__job01__jobqueue2__

// schwa::job01 ===============================================================
namespace schwa { namespace job01 {



class JobQueue2 : public core::RingElement<JobQueue2, 
                                           3 * host::CACHE_LINE_SIZE> {

 public:



 private:
    class PriorityQueue {

    };


};




}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__jobqueue2__

