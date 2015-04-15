//
//  queue.h
//  schwa::job
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__job__queue__
#define __schwa__job__queue__


#include "free_list.h"
#include "impl_queue.h"
#include "spawn.h"

#include <condition_variable>
#include <mutex>


// namespace schwa::job
namespace schwa {namespace job {


class JobPool;

class Queue {
 public:
    Queue(JobPool& pool, std::condition_variable& cond, std::mutex& mutex);

    // Spawn a new job, with no parent job.
    Spawn spawn(Kernel kernel, JobData& data);
    Spawn spawn(KernelLambda&& lambda);

    unsigned count() const { return _core.count(); }

 protected:
    impl::Queue _core;
    JobPool& _pool;

    friend class Job;

    // TODO: Avoid this?
    friend class Worker;
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__queue__