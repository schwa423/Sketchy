//
//  job.cpp
//  schwa::job
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "job.h"
#include "pool.h"
#include "queue2.h"


// namespace schwa::job
namespace schwa {namespace job {


Job::Job(const Kernel kernel, const JobData& data, Queue* queue) :
    _generation(_generation),  // noop for emphasis... we intentionally reuse the generation
    _kernel(kernel),
    _data(data),
    _queue(queue),
    _prereqCount(0),
    _state(WANTS_ENQUEUE) {

    SCHWASSERT_CODE(_isSpawning = true);
}


void Job::addChild(const JobId& parentId, Job* childJob) {
    SCHWASSERT(parentId.isValid(), "invalid parent ID");
    SCHWASSERT(_isSpawning && childJob->_isSpawning,
               "when adding child, both parent and child must be in process of spawning");

    incrPrereqs();
    childJob->_parent = parentId;
}


void Job::prereqDone() {
    int32_t count = decrPrereqs();
    if (count > 0) {
        // More prerequisites remain.
        return;
    }

    SCHWASSERT(count == 0, "negative prereq count");

    switch (_state) {
        case WANTS_ENQUEUE:
            _queue->_core.enqueue(this);
            break;
        case WANTS_REQUEUE:
            _queue->_core.requeue(this);
            break;
        case NO_MORE_WORK:
            JobPool& pool = _queue->_pool;

            // Notify parent that one of its children has finished.
            Job* parent = pool.findJob(_parent);
            if (parent) parent->prereqDone();

            pool.release(this);

            break;
        // No default case... all enum values are explicitly handled.
    }
}


void Job::run() {
    SCHWASSERT(_state != NO_MORE_WORK, "Job has no more work to do");
    SCHWASSERT(_prereqCount == 0, "Job cannot run because it still has prerequisites");

    // This can change if the Kernel yields.
    _state = NO_MORE_WORK;

    // TODO: debletcherize
    JobId id = _queue->_pool.findId(this);

    // Create a new Spawn... when destroyed at the end of this method, it will either:
    // - notify the parent that the job has finished, and release the job to the job-pool, or
    // - wait for all prerequisites to complete, then enqueue the job.
    Spawn me(id, this, _queue);

    // Invoke the kernel.
    _kernel(_data, me);
}


}}  // namespace schwa::job