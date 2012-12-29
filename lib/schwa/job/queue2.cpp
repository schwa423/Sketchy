//
//  queue.cpp
//  schwa::job
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "queue2.h"
#include "job.h"
#include "pool.h"


// namespace schwa::job
namespace schwa {namespace job {


Queue::Queue(JobPool& pool, std::condition_variable& cond, std::mutex& mutex) :
    _pool(pool), _core(cond, mutex) {

}


Spawn Queue::spawn(Kernel kernel, JobData& data) {
    Job* job = _pool.obtain(kernel, data, this);
    JobId id = _pool.findId(job);
    return Spawn(id, job, this);
}


void KernelLambdaTrampoline(JobData& data, Spawn& parent) {
    auto& lambda = data._lambda;
    lambda(parent);

    // Because we used placement-new, we need to explicitly destroy the lambda.
    lambda.~KernelLambda();
}


Spawn Queue::spawn(KernelLambda&& lambda) {
    JobData data;
    Job* job = _pool.obtain(KernelLambdaTrampoline, data, this);
    JobId id = _pool.findId(job);

    // Use placement-new to install the lambda into the Job.
    // We'll need to explicitly run the destructor... this is
    // done by KernelLambdaTrampoline().
    KernelLambda& location = job->_data._lambda;
    new (&location) KernelLambda(std::move(lambda));

    return Spawn(id, job, this);
}


}}  // namespace schwa::job