//
//  job.h
//  schwa::job
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__job__job__
#define __schwa__job__job__


#include "free_list.h"
#include "kernel.h"
#include "jobid.h"

#include <memory>

#include <atomic>
#include <mutex>



// TODO: should be in schwa::job::impl, since app-devs never touch one.

// namespace schwa::job
namespace schwa {namespace job {


class Queue;

class Job : public mem::Link {
 public:
    Job(Kernel kernel, const JobData& data, Queue* queue);

    uint32_t generation() const { return _generation; }

    // Run the job.
    void run();

 private:
    // Add dependency on specified child-job; this job
    // will not be scheduled until the child has completed.
    // This calls addPrereq().
    void addChild(const JobId& parentId, Job* childJob);

    // Increment prereq-count, and return the new value.
    int32_t incrPrereqs() {
        // TODO: avoid locking by making _prereqCount atomic.
        std::lock_guard<std::mutex> lock(_mutex);
        return ++_prereqCount;
    }

    // Decrement prereq-count, and return the new value.
    int32_t decrPrereqs() {
        // TODO: avoid locking by making _prereqCount atomic.
        std::lock_guard<std::mutex> lock(_mutex);
        return --_prereqCount;
    }

    // A prerequisite has been satisfied.  If there are no remaining
    // prerequisites, then:
    // - if there is more work to do, enqueue ourself to do it.
    // - otherwise, notify our parent (if any) that a prereq is done.
    void prereqDone();

    Kernel _kernel;
    JobData _data;

    // Allows us to determine whether the job identified by a JobId has completed
    // (if the job's generation differs from the JobId's, then it has completed).
    uint32_t _generation;
    // TODO: use an atomic int.
    // std::atomic<uint32_t> _generation;

    std::mutex _mutex;

    int32_t _prereqCount;
    JobId _parent;

    enum State { WANTS_ENQUEUE, WANTS_REQUEUE, NO_MORE_WORK };
    State _state;

    Queue* _queue;

    // Declare variable used for assertions.
    SCHWASSERT_CODE(bool _isSpawning);

    friend class JobPool;
    friend class Queue;
    friend class Spawn;
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__job__