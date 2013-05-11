//
//  job_impl.h
//  schwa::job01::impl
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//  *** TODO ***
//      - just a sketch
//


#ifndef __schwa__job01__job_impl__
#define __schwa__job01__job_impl__


#include "job01/core/link.h"
// #include "job01/job_pool.h"
// #include "job01/generic_worker.h"


// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {


class JobImpl : public Link<JobImpl> {
 private:
 	int foo, bar, baz;
};



}}}  // schwa::job01::impl ====================================================


#endif  // #ifndef __schwa__job01__job_impl__