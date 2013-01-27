#include "job00/job_impl.h"
#include "job00/job_pool.h"

#include <new>
#include <assert.h>

// Use template magic to create a GenericJobImpl<JobDescT>.
// - allocate memory from a JobPool, chosen at compile-time
//   to have big-enough objects.
// - in that memory, instantiate concrete JobImpl subclass.
template <class JobDescT> 
JobImpl* JobImpl::Create(JobDescT&& desc) {

  typedef GenericJobImpl<JobDescT> JobT;

  // Allocate an object from the pool, and bail out if we can't.
  JobImpl* ptr = JobPool::Alloc(desc);
  if (!ptr) return nullptr;

  // Create Job using placement new.
  auto result = new (ptr) JobT(std::move(desc));
  return ptr;
}


// Responsibilities:
// - cast to correct static type
// - invoke operator() on job-description
template <class JobDescT>
void GenericJobImpl<JobDescT>::CastAndDo(JobImpl* ptr, JobImpl::operation op) {

  // Cast to actual static type.
  auto job = static_cast<MyJobT*>(ptr);

  // Apply the specified operation.
  switch (op) {

    // Run the job.  This works on lambdas too!
    case JOB_RUN:
      // TODO: dependencies would look something like this.
      //      {
      //        Job spawner(JobImpl* job, false);
      //        job->desc(spawner);
      //      }
      job->desc_();
      return;

    // Run destructor, and deallocate job.
    case JOB_DESTROY:
      job->~MyJobT();
      JobPool::Dealloc(job);
      return;

    // Error!
    default:
      bool unknownJobOperation = false;
      assert(unknownJobOperation);
  }
}