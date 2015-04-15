//
//  pool.cpp
//  schwa::job
//
//  Created by Josh Gargus on 12/16/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "pool.h"
#include "schwassert.h"

typedef std::lock_guard<std::mutex> lock_guard;


// namespace schwa::job
namespace schwa {namespace job {


// static variable
const uint32_t JobPool::MAX_JOBS;


JobPool::JobPool() :
    _free(_mem, MAX_JOBS, sizeof(Job)),
    _jobs(reinterpret_cast<Job*>(_mem)) {

    // Initialize job generations to 0.
    for (int i=0; i<MAX_JOBS; i++)
        _jobs[i]._generation = 0;
}


Job* JobPool::obtain(const Kernel& kernel, JobData& data, Queue* queue) {
    Job* job;
    {
        lock_guard lock(_mutex);
        job = static_cast<Job*>(_free.obtain());
    }

    if (!job)
        throw std::bad_alloc();

    // We carefully refrain from stomping this when constructing the job below.
    job->_generation++;
    // Use placement-new to construct the job.
    new (job) Job(kernel, data, queue);

    return job;
}


void JobPool::release(Job* job) {
    job->_generation++;
    {
        lock_guard lock(_mutex);
        _free.release(job);
    }
}


// TODO: potential performance optimizations
// - inline findId()... maybe also obtain()/release()?
// - if sizeof(Job) is power-of-two (which we might want anyway to match cache-line size),
//   then use bit-shift instead of division to compute byte-offset.
JobId JobPool::findId(Job* job) const {
    // Implemented this way so that Clang uses RVO.
    JobOffset offset;
    uint32_t generation;
    if (job) {
        int32_t byteOffset = reinterpret_cast<const char*>(job) - _mem;
        offset = byteOffset / sizeof(Job);
        generation = job->generation();
    } else {
        offset = JobId::BAD_OFFSET;
        generation = 0;
    }

    JobId id(offset, generation);
    SCHWASSERT_CODE( id._pool = const_cast<JobPool*>(this); )
    return id;
}


// TODO: potential performance optimizations
// - inline findJob
Job* JobPool::findJob(JobId id) const {
    if (JobId::BAD_OFFSET == id._offset)
        return nullptr;

    Job* job = _jobs + id._offset;

    // This might miss some race-conditions, but will still
    // identify misuse with high probability.
    SCHWASSERT(job->generation() == id._generation, "job generations don't match");

    return job;
}


}}  // namespace schwa::job