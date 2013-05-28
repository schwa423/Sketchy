//
//    job.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: just diddling around... this code probably doesn't even compile.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__job__
#define __schwa__job01__job__


#include "job01/mem/blocks.h"
#include "job01/core/queue.h"

// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


// Forward declarations.
namespace impl { class JobX; }
typedef mem::BlockRef<impl::JobX> JobRef;


typedef core::Queue<JobRef> JobChain;


class Job {
 public:
  Job(JobRef id) : _id(id), _x(static_cast<JobX*>(id)) {
    _jobx->ref();
  }

  virtual ~Job(); {
    _jobx->unref();
  }

  template <typename JobDescT>
  Job spawn(JobDescT&& desc) {

  }

 private:
  JobID _id;
  JobX* _jobx;
};


}}  // schwa::job01 ===========================================================

#endif  // #ifndef __schwa__job01__job__