//
//  pool.h
//  schwa::job
//
//  Created by Josh Gargus on 12/16/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//
//  TODO: consider having one global JobPool.  Then, we wouldn't need to
//        worry about a JobId indexing into the wrong pool, and interfaces
//        can be simpler because the JobId wouldn't need a (transitive)
//        reference to a specific pool.
//
//  TODO: use lock-free implementation
//
//

#ifndef __schwa__job__pool__
#define __schwa__job__pool__


#include "job.h"
#include "jobid.h"

#include <mutex>


// namespace schwa::job
namespace schwa {namespace job {


// Forward declaration.
class Job;
class Queue;

class JobPool {
 public:
    const static uint32_t MAX_JOBS = 8192;

    JobPool();

    // Obtain a job from the pool, or release it back to the pool.
    // Throws bad_alloc if there are no free jobs available.
    Job* obtain(const Kernel& kernel, JobData& data, Queue* queue);
    void release(Job* job);

    // Answer the ID of the specified job.
    JobId findId(Job* job) const;

    // Answer the Job corresponding to the ID.
    // NOTE: only valid to call this when we know the Job exists (i.e. it has not
    //       been released back to the JobPool).
    Job* findJob(JobId id) const;

    // Answer the number of free jobs remaining in the pool.
    uint32_t freeCount() const { return _free.freeCount(); }

 private:
    char _mem[MAX_JOBS * sizeof(Job)];
    Job* _jobs;

    mem::FreeList _free;

    std::mutex _mutex;
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__pool__