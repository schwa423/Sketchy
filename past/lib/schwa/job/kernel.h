//
//  kernel.h
//  Sketchy
//
//  Created by Josh Gargus on 12/16/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__job__kernel__
#define __schwa__job__kernel__


#include <functional>


// namespace schwa::job
namespace schwa {namespace job {


// TODO: instead of Spawn, use a subclass: RunningJob.  Maybe.
union JobData;
class Spawn;

// TODO: Document difference between Kernel and KernelLambda.
typedef void (*Kernel)(JobData& data, Spawn& parent);
typedef std::function<void (Spawn& parent)> KernelLambda;

typedef void* KernelData;

union JobData {
    KernelData _kernelData;

    // NOTE: Everything below this point is to support running lambda-functions in a job,
    // and should be ignored by app-developers... whenever they have access to a JobData,
    // the union will contain KernelData, not a KernelLambda.

    // Allow JobData to act like POD (plain old data) by default.
    // If it must contain a KernelLambda, we will explicitly construct and destruct it.
    JobData() { }
    ~JobData() { }

private:
    JobData(const JobData& other) : _kernelData(other._kernelData) { }

    KernelLambda _lambda;
    friend class Job;
    friend class Queue;
    friend void KernelLambdaTrampoline(JobData&, Spawn&);
};


// No-op kernel... Jobs are initialized with this by default.
void EmptyKernel(JobData& data, Spawn& parent);


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__kernel__