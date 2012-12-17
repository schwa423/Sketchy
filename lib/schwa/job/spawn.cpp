//
//  spawn.cpp
//  schwa::job
//
//  Created by Josh Gargus on 12/13/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "spawn.h"
#include "job.h"
#include "queue2.h"
#include "schwassert.h"


// namespace schwa::job
namespace schwa {namespace job {


// Ensure that old Spawn is invalid.
Spawn::Spawn(Spawn&& orig)
    : _id(std::move(orig._id)), _job(orig._job), _queue(orig._queue) {
    orig._job = nullptr;
    orig._queue = nullptr;
}


Spawn::Spawn(const JobId& id, Job* job, Queue* q) : _id(id), _job(job), _queue(q) {
    // Increment the job count to guard against the following race-condition:
    // - we intend to spawn two child jobs from a parent
    // - after spawning one, it runs and finishes before we can spawn the next
    // - the prereq-count therefore hits zero, triggering the job to be enqueued
    _job->incrPrereqs();
}


Spawn Spawn::spawn(const Kernel& kernel, JobData& data, Queue* queue) {
    if (!_job) {
        // The only way that _job can be null is if this Spawn was invalidated
        // by passing it to a move-constructor.
        throw std::invalid_argument("Spawn was invalidated by move-constructor");
    }

    // Spawn job on specified queue, or failing that on the same queue as this job.
    auto q = queue ? queue : _queue;
    Spawn child = q->spawn(kernel, data);

    // Tell the child who the parent is, and increment parent's prereq-count.
    _job->addChild(_id, child._job);

    return child;
}


Spawn Spawn::spawn(KernelLambda&& lambda, Queue* queue) {
    if (!_job) {
        // The only way that _job can be null is if this Spawn was invalidated
        // by passing it to a move-constructor.
        throw std::invalid_argument("Spawn was invalidated by move-constructor");
    }

    // Spawn job on specified queue, or failing that on the same queue as this job.
    auto q = queue ? queue : _queue;
    Spawn child = q->spawn(std::move(lambda));

    // Tell the child who the parent is, and increment parent's prereq-count.
    _job->addChild(_id, child._job);

    return child;
}


Spawn::~Spawn() {
    // If null, move-constructor made someone else responsible for scheduling the job.
    if (_job == nullptr) return;

    _job->prereqDone();
}


}}  // namespace schwa::job