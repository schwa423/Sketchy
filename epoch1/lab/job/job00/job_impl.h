#ifndef __schwa__job00__job_impl__
#define __schwa__job00__job_impl__

#include "job00/link.h"

#include <utility>


// schwa::job00 ===============================================================
namespace schwa { namespace job00 {


class Queue;

class JobImpl : public Link<JobImpl> {
  friend class Job;
  friend class JobPool;

  public:
    // Cast job to actual static type before invoking or destroying it.
    enum operation { JOB_RUN, JOB_DESTROY };
    typedef void (*CastAndDo)(JobImpl* job, JobImpl::operation op);

    // The memory for the "generation_" and "LinkT" parts of the
    // object were initialized to a sane state when the pool was created,
    // and we shall strive to our utmost to maintain the "sanity invariant".
    explicit JobImpl() : generation_(0) { }
    JobImpl(CastAndDo d) :
      do_(d),
      schedulable_(true),
      children_(0),
      queue_(nullptr) {
      ++generation_;
    }

    // Just for fun... we don't otherwise use it in job00.
    bool isAllocated() { return generation_ % 2; }

    // NOTE: include "job_impl_create.h" to use this... this allows me to
    //       break a circular dependency with the use of JobPool.  Ugly,
    //       but not a big deal... these aren't public headers.
    //
    // Use template magic to:
    // - find a JobPool with jobs at least as large as GenericJobImpl<JobDescT>
    // - instantiate one
    // - stash an unpickle-and-run function which casts the job back to
    //   its real type before running it.
    template <class JobDescT>
    static JobImpl* Create(JobDescT&& desc);

    unsigned incrementChildren() { return ++children_; }
    unsigned decrementChildren() { return --children_; }

    void run()     { do_(this, JOB_RUN); }
    void destroy() { do_(this, JOB_DESTROY); }

  protected:
    // Protected so that it can only be called from Destructor.
    ~JobImpl() { ++generation_; }

  private:
    typedef unsigned Generation;

    Generation     generation_;  // keep track of constructions/destructions
    CastAndDo      do_;          // casts job to subclass before running
    Queue*         queue_;       // schedule job on this queue

    struct     {
      // number of outstanding children
      unsigned children_    : 20;
      // index of the sub-pool this job was allocated from.
      // 8 subpools support jobs from 64 to 8192 bytes... i.e, bigger than we need.
      unsigned pool_        : 3;
      // true if job should be scheduled once all children are finished,
      // false if it should be destroyed.
      bool     schedulable_ : 1;
    };
};


// Internal helper class used by JobImpl.  Conceptually, this class is private
// to JobImpl::Create(), however we let it stand alone for two reasons:
// - C++ doesn't allow definition of function-scoped classes
// - more convenient for testing
template <class JobDescT>
class GenericJobImpl : public JobImpl {
  friend class JobImpl;

  protected:
    typedef GenericJobImpl<JobDescT> MyJobT;

    GenericJobImpl(JobDescT&& desc)
      : JobImpl(MyJobT::CastAndDo), desc_(std::move(desc)) { }

    // Casts job to actual static type and does one of the following:
    // - JOB_RUN: invoke operator() on job-description
    // - JOB_DESTROY: invoke in-place destructor, and release pointer back to JobPool.
    static void CastAndDo(JobImpl* ptr, JobImpl::operation op);

    JobDescT desc_;
};


}}  // schwa::job00 ===========================================================


#endif  // #ifndef __schwa__job00__job_impl__