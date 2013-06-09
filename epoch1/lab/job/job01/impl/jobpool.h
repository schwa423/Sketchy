//
//    jobpool.h
//    schwa::job01::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: refine
//    TODO: document
//    TODO: test better
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
#include "job01/obj/obj.h"

#include <vector>

// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {

using host::CACHE_LINE_SIZE;


// Has a number of sub-pools which contain a free-list of unallocated jobs...
// each one holds jobs of a specific size (1, 2, 4, or 8 cache-lines large).
class JobPool : public obj::ObjMaker {
 public:
    
    // Construct a new job of the specified type, and return a ref to it.
    template <typename JobDescT>
    JobRef Alloc(JobDescT&& desc) {
        // Compile-time constant.
        constexpr auto size_code = SizeCodeForJob<JobDescT>();
        // Choose the free-list with correctly-sized unconstructed jobs.
        JobList& free_list = _free_lists[size_code];
        // Get the next job.
        JobRef ref = free_list.next();
        // If there is no next job...
        if (ref == nullptr) {
            // ... make some new ones, and add them to the same free-list.
            free_list.add(MakeObjects<JobX>(kMaxArraySize));

            // Get the next job; this is the one we will return.
            ref = free_list.next();

            // Iterate through all newly-allocated jobs, and do 
            // whatever initialization is necessary.
            JobX* uninitialized = static_cast<JobX*>(ref);
            do {
                uninitialized->_generation = 0;
                uninitialized = static_cast<JobX*>(uninitialized->nextLink());
            } while (uninitialized != nullptr);
        }
        // Get raw pointer to unconstructed job's memory.
        JobX* ptr = static_cast<JobX*>(ref);
        SCHWASSERT(SizeCodeForPtr(ptr) == size_code, "Job is wrong size!!");

        // Construct and return a new job of the correct type.
        new (ptr) TypedJobX<JobDescT>(std::move(desc));
        return ref;
    }

    // Call destructor, and return job to the pool for later reuse.
    void Free(JobRef& ref, JobX* ptr) {
        // Call the destructor.
        ptr->~JobX();
        // Add to the free-list containing other jobs of the same size.
        JobList& free_list = _free_lists[SizeCodeForPtr(ptr)];
        free_list.add(ref);
    }    

    template <typename JobDescT>
    static constexpr obj::ObjSizeCode SizeCodeForJob() {
        return SizeCodeFor<TypedJobX<JobDescT>>();
    }

 protected:
    typedef core::Queue<JobX> JobList;

    JobList _free_lists[kNumObjSizes];

};


}}}  // schwa::job01::impl ====================================================

#endif  // #ifndef __schwa__job01__impl__jobpool__
