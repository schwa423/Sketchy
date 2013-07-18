//
//    job_impl.h
//    schwa::job01::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: rename to jobx.h
//    TODO: this is just the first step... needs to integrate with:
//          - spawn()
//          - worker (eg: after Run(), run again, or release back to JobPool?)
//    TODO: document where this fits in the big picture.  For example,
//          "not exposed to the end-user, but this is where the magic happens"
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__job_impl__
#define __schwa__job01__job_impl__


#include "job01/core/link.h"
#include "job01/host/host.h"
#include "job01/mem/align.h"
#include "job01/obj/obj.h"

#include <atomic>
#include <new>


// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {

using namespace mem;


// Forward declaration of JobX, so we can define the reference-type.
class JobX;
typedef obj::ObjRef<JobX> JobRef;


// Base class for (internal implementation of) jobs in the job-system.
class JobX : public obj::Obj<JobX> {
  friend class JobPool;
  friend class JobArray;

 public:
    // Overridden in subclasses.
    virtual void Run() { };

 protected:
    // This is the constructor called by TypedJobX when allocating
    // a job from a JobPool.
    JobX() : _priority(0),
//             _generation(_generation+1),
             _schedulable(true),
             _parent(nullptr),
             _children(0) {
      ++_generation;
    }

    // Destructor and constructor update the _generation,
    // to catch potential misuse.
    virtual ~JobX() {
        ++_generation;
        SCHWASSERT(0 == _generation % 2,
                   "generation must be even after destruction");
    }

 private:
    // JobRef is 3 bytes, so explicitly reserve 4th byte until
    // we have something useful to do with it.  Otherwise it would
    // be wasted anyway, to pad _children to the desired alignment.
    JobRef      _parent;
    unsigned    _priority     : 2;
    bool        _schedulable  : 1;
    unsigned    _generation   : 5;

  	// Number of outstanding children.  We don't need to support
    // this many children, but we might as well, because atomics
    // are always going to be at least 4 bytes.
    std::atomic<uint32_t> _children;


    static void InitForJobArray(JobX* ptr) {
        ptr->_generation = 1;
        new(ptr) JobX();
    }
};

// Size breakdown:
//   4 bytes:  virtual function table pointer
//   4 bytes:  superclass
//   3 bytes:  parent
//   1 byte:   priority/size_code/generation/schedulable
//   4 bytes:  children
//   ---------
//   16 bytes total
//
// This might be different on 64-bit platforms due to the
// larger size of the pointer to the virtual function table.
// If so, deal with it later.
#if SCHWA_32_BIT
static_assert(sizeof(JobX) == 16, "JobX should be 16 bytes");
#elif SCHWA_64_BIT
static_assert(sizeof(JobX) == 20, "JobX should be 20 bytes");
static_assert(sizeof(obj::Obj<JobX>) == 12, "super class should be 12 bytes");
static_assert(sizeof(JobRef) == 3, "JobRef should be 3 bytes");
//static_assert(sizeof(JobX._children) == 8, "JobRef._children should be 4 bytes");
static_assert(sizeof(std::atomic<uint32_t>) == 4, "std::atomic<uint32_t> should be 4 bytes");
static_assert(sizeof(core::Link<JobX, JobRef>) == 11, "link should be 3 bytes");
#endif

}}}  // schwa::job01::impl ====================================================

#endif  // #ifndef __schwa__job01__job_impl__
