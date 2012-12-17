//
//  spawn.h
//  schwa::job
//
//  Created by Josh Gargus on 12/13/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//
//  Spawn's class name is a pun, because it both represents a job that has been spawned,
//  and it provides an interface to spawn new children (i.e dependencies) of that job.
//
//  The API is designed to be convenient, safe, and efficient.  Conveniently, there is no
//  need to separately create and enqueue a job; both are handled by calling spawn() on
//  a Queue or Spawn.
//
//  This also safe; for example, there is no way to create cycles, as there would be in an
//  API like "job.addChild(job2)".
//
//  It is also impossible to add a child to a job that might currently be running.  This
//  is because the newly-spawned job is not enqueued until:
//    - the ~Spawn() destructor has executed
//    - all spawned children have run to completion
//  This simplifies both the API and implementation, and provides the application developer
//  control over the interval during which spawning is allowed (by controlling the lifespan
//  of the Spawn object).
//
//  A move-constructor is provided.  When this is invoked, the old Spawn becomes invalid
//  to use for any purpose, and does not cause a Job to be scheduled upon destruction.
//
//  As a general rule, Spawn objects should not be stashed for long... children should be
//  spawned as soon as convenient, and the Spawn should be destroyed.  They must not be
//  leaked, since Jobs are allocated from a fixed-size pool.
//
//  Spawn is not thread-safe.
//
//


#ifndef __schwa__job__spawn__
#define __schwa__job__spawn__


#include "jobid.h"
#include "kernel.h"

// namespace schwa::job
namespace schwa {namespace job {


class Job;
class Queue;

class Spawn {
 public:
    // Move constructor... the only public way to instantiate a Spawn.
    Spawn(Spawn&& orig);
    ~Spawn();

    // Spawn a child job, and return another Spawn which can spawn children of its own.
    // The child will be scheduled on the specified Queue, if one is provided.  Otherwise,
    // it will be scheduled on the same queue as this Spawn's Job.
    Spawn spawn(const Kernel& kernel, JobData& data, Queue* queue = nullptr);
    Spawn spawn(KernelLambda&& lambda, Queue* queue = nullptr);

    // Answer the ID of the spawned Job.
    const JobId& id() const { return _id; }

 protected:
    // Private so that only Spawn and friends can instantiate new instances.
    // Copy-constructor is forbidden, although a public move-constructor is provided.
    Spawn(const JobId& id, Job* job, Queue* queue);
    Spawn(const Spawn& orig);

    JobId _id;
    Job*  _job;
    Queue* _queue;

    friend class Queue;
    friend class Job;
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__spawn__