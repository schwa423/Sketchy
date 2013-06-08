//
//    job_impl.h
//    schwa::job01::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: rename to jobx.h
//    TODO: move AlignedJobX into a separate file.
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
#include "job01/mem/blocks.h"

#include <atomic>
#include <new>


// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {

using namespace mem;


// Forward declaration of JobX, so we can define the reference-type.
class JobX;
typedef mem::BlockRef<JobX> JobRef;


// Base class for (internal implementation of) jobs in the job-system.
class JobX : public core::Link<JobX, JobRef> {
  friend class JobPool;
  friend class JobArray;

 public:
    // Overridden in subclasses.
    virtual void Run() { };

 protected:
    // This is the constructor called by AlignedJobX during JobPool
    // initialization.  The other fields are not initialized because
    // they're set each time a job is allocated from a pool.
    JobX(unsigned size_code) 
    : _size_code(size_code), _generation(0) { }

    // This is the constructor called by TypedJobX when allocating
    // a job from a JobPool.
    // NOTE: we intentionally initialize _size_code to itself to keep the
    //       compiler happy (since a const member needs an initializer)...
    //       it was set in the constructor above (called by AlignedJobX),
    //       and remains unchanged across multiple Alloc/Free cycles.
    JobX() : _size_code(_size_code),
             _schedulable(true), 
             _parent(nullptr),
             _children(0) {
        ++_generation;
        SCHWASSERT(1 == _generation % 2, 
                   "generation must be odd after instatiation");
    }

    // Destructor and constructor update the _generation,
    // to catch potential misuse.
    virtual ~JobX() { 
        ++_generation;
        SCHWASSERT(0 == _generation % 2, 
                   "generation must be odd after destruction");        
    } 

 private:
    // Superclass is 3 bytes.  Fill in 4th byte, otherwise it
    // will be wasted to pad _parent to the desired alignment.
   	unsigned       _priority     : 2;
   	const unsigned _size_code    : 2;
   	unsigned       _generation   : 3;
   	bool           _schedulable  : 1;

    // JobRef is 3 bytes, so explicitly reserve 4th byte until
    // we have something useful to do with it.  Otherwise it would
    // be wasted anyway, to pad _children to the desired alignment.
    JobRef   _parent;
    uint8_t  _reserved;

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
//   3 bytes:  superclass
//   1 byte:   priority/size_code/generation/schedulable
//   3 bytes:  parent
//   1 byte:   reserved
//   4 bytes:  children
//   ---------
//   16 bytes total
//   
// This might be different on 64-bit platforms due to the
// larger size of the pointer to the virtual function table.
// If so, deal with it later.
static_assert(sizeof(JobX) == 16, "JobX should be 16 bytes");


// Used to instantiate BlockArrays with jobs of the desired size.
// Not intended to ever be scheduled/run in the job-system.
template<int SIZE_CODE>
class alignas(host::CACHE_LINE_SIZE << SIZE_CODE) AlignedJobX : public JobX {
 public:
    // Called during JobPool initialization.
    AlignedJobX() : JobX(SIZE_CODE) { }

    // Need to define this so that AlignedJobX is not an abstract class...
    // but it should never be run.
    virtual void Run() {
        SCHWASSERT(false, "AlignedJobX is not intended to ever run");
    }
};


}}}  // schwa::job01::impl ====================================================

#endif  // #ifndef __schwa__job01__job_impl__
