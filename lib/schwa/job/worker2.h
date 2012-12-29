//
//  worker.h
//  schwa::job
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__job__worker__
#define __schwa__job__worker__


#include <condition_variable>
#include <mutex>


// namespace schwa::job
namespace schwa {namespace job {


class Job;
class Queue;


class Worker {
 public:
    Worker(Queue* queues, int numQueues, std::condition_variable& cond, std::mutex& mutex);

    // Start a thread and loop, pulling jobs from queues until stopped.
    // TODO: revamp API for thread-safety.  We don't want to be constantly be
    //       grabbing a mutex, but as it is we're not thread-safe with addQueue().
    void run();
    void stop();

 protected:

    Job* next();
    Job* unlockedNext();

    const int _numQueues;
    Queue* _queues;

    std::condition_variable& _cond;
    std::mutex& _mutex;

    bool _running;

    friend void runLoop(Worker* w);
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__worker__