//
//    jobpool.h
//    schwa::job01::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: refine
//    TODO: document
//    TODO: test much better
//    TODO: consider a way to avoid the reinterpret_cast() below
//          - perhaps an extra (or simply different?) template parameter to
//            BlockArray so that operator[] returns JobRefs?
//          - I like that better than allowing implicit conversion via
//            copy-constructor.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__impl__jobpool__
#define __schwa__job01__impl__jobpool__


#include "job01/core/meta.h"
#include "job01/core/queue.h"
#include "job01/core/schwassert.h"
#include "job01/host/host.h"
#include "job01/impl/job_impl.h"
#include "job01/impl/typedjobx.h"

#include <vector>


// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {

using host::CACHE_LINE_SIZE;


// Has a number of sub-pools which contain a free-list of unallocated jobs...
// each one holds jobs of a specific size (1, 2, 4, or 8 cache-lines large).
class JobPool {
 public:
    
 public:  // TODO: make private

    // This is a sub-pool that contains jobs of a specific size.
    template <int SIZE_CODE>    
    class SizedJobPool {
     public:
        // BlockArray containing jobs of the desired size.
        static const int kJobArraySize = 1024;
        typedef AlignedJobX<SIZE_CODE> JobT;
        typedef mem::BlockArray<JobT, kJobArraySize> JobArray;

        JobRef Alloc() {
            if (_queue.empty()) {
                JobArray* jobs = Create<JobArray>();
                _arrays.push_back(jobs->id);
                for (int i = 0; i < kJobArraySize; i++) {
                    BlockRef<JobT> block_ref = (*jobs)[i];
                    // TODO: ugh!!
                    JobRef* job_ref = reinterpret_cast<JobRef*>(&block_ref);

                    _queue.add(*job_ref);
                }
            }
            return _queue.next();
        }

        void Free(JobRef& ref) {
            SCHWASSERT(SIZE_CODE == static_cast<Job*>(ref)->_size_code,
                       "attempting to return job to wrong-sized pool");
            // Invoke virtual destructor.
            static_cast<JobX*>(ref)->~JobX();
            // Add job back to the free-list.
            _queue.add(ref);
        }

     private:
        core::Queue<JobX> _queue;

        // TODO: this is a good reason to have a BlockArrayRef...
        //       it probably shouldn't be in mem::impl.
        std::vector<mem::impl::BlockArrayRef> _arrays;
    };


// Macro to define:
// - a queue to hold free blocks of the desired size
// - an Alloc() function which:
//   - obtains a free block
//   - uses placement-new to initialize a JobX of the appropriate type
// - a Free() function which:
//   - invokes the job's destructor
//   - returns the memory to the appropriate pool
#define DEFINE_SIZED_JOB_POOL(POOL_NAME, SIZE_CODE)                           \
    SizedJobPool<SIZE_CODE> POOL_NAME;                                        \
    template <typename JobDescT>                                              \
    JobRef Alloc(JobDescT&& desc,                                             \
                 typename std::enable_if<SIZE_CODE == SizeCode<JobDescT>()>::type* dummy = nullptr) { \
        JobRef ref = POOL_NAME.Alloc();                                       \
        JobX* ptr = static_cast<JobX*>(ref);                                  \
        SCHWASSERT(ptr->_size_code == SIZE_CODE, "Job is wrong size!!");     \
        new (ptr) TypedJobX<JobDescT>(std::move(desc));                       \
        return ref;                                                           \
    }

    DEFINE_SIZED_JOB_POOL(_pool64,   0);
    DEFINE_SIZED_JOB_POOL(_pool128,  1);
    DEFINE_SIZED_JOB_POOL(_pool256,  2);
    DEFINE_SIZED_JOB_POOL(_pool512,  3);
#undef DEFINE_SIZED_JOB_POOL


    void Free(JobRef& ref, JobX* ptr) {
        SCHWASSERT(ptr == static_cast<JobX*>(ref),
                   "JobRef dereferences to different pointer");
        switch(ptr->_size_code) {
            case 0:
                _pool64.Free(ref);
                break;
            case 1:
                _pool128.Free(ref);
                break;
            case 2:
                _pool256.Free(ref);
                break;
            case 3:
                _pool512.Free(ref);
                break;
            default: 
                SCHWASSERT(false, "size-code must be between 0 and 3");
        }
    }


};


}}}  // schwa::job01::impl ====================================================

#endif  // #ifndef __schwa__job01__impl__jobpool__
