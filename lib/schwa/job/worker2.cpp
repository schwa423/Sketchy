//
//  worker.cpp
//  schwa::job
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "worker2.h"
#include "queue2.h"
#include "job.h"

#include <chrono>
using std::chrono::milliseconds;
using std::cv_status;

typedef std::unique_lock<std::mutex> unique_lock;

// namespace schwa::job
namespace schwa {namespace job {


Worker::Worker(Queue* queues, int numQueues, std::condition_variable& cond, std::mutex& mutex) :
    _queues(queues),
    _numQueues(numQueues),
    _cond(cond),
    _mutex(mutex),
    _running(false) {
}


void Worker::run() {
    // TODO: not thread-safe!
    if (_running) return;
    _running = true;

    // WORK IN PROGRESS   WORK IN PROGRESS    WORK IN PROGRESS    WORK IN PROGRESS
    // WORK IN PROGRESS   WORK IN PROGRESS    WORK IN PROGRESS    WORK IN PROGRESS
    // WORK IN PROGRESS   WORK IN PROGRESS    WORK IN PROGRESS    WORK IN PROGRESS
    // WORK IN PROGRESS   WORK IN PROGRESS    WORK IN PROGRESS    WORK IN PROGRESS
    // WORK IN PROGRESS   WORK IN PROGRESS    WORK IN PROGRESS    WORK IN PROGRESS
    // WORK IN PROGRESS   WORK IN PROGRESS    WORK IN PROGRESS    WORK IN PROGRESS
    // TODO: implement a run-loop for Worker
    SCHWASSERT(false, "Worker::run() is not implemented");
}


Job* Worker::unlockedNext() {
    // Look for a job, starting with the highest-priority queue.
    for (int q=0; q < _numQueues; q++) {
        auto job = _queues[q]._core.unlockedNext();
        if (job) return job;
    }
    // Didn't find one.
    return nullptr;
}


// TODO: probably inline
Job* Worker::next() {
    unique_lock lock(_mutex);

    auto job = unlockedNext();
    if (job) return job;

    // Didn't find a job.  Wait for condition variable to signal that
    // a job is available, and return nullptr if we time out.
    // TODO: tune this timeout, or better yet make it configurable
    if (cv_status::timeout == _cond.wait_for(lock, milliseconds(10)))
        return nullptr;

    // Try again, but don't wait this time if we don't find a Job.
    // (it's possible that someone else already grabbed the Job, or
    //  that the condition_variable was signalled spuriously).
    return unlockedNext();
}


}}  // namespace schwa::job