//
//  boss.h
//  schwa::job
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//
//  A Boss is responsible for a set of Workers and a list of
//  Queues.  The order of the list defines relative queue
//  priorities... a worker will prefer to pull a job from an
//  earlier queue, only pulling from the next queue if no job
//  was available.
//
//  All workers pull from exactly the same list of Queues.  I
//  originally wanted to mix'n'match... a sample use-case would be:
//    - number of workers = number of CPUs
//    - 2 queues:
//      - one for general CPU work
//      - one for OpenGL rendering
//    - All workers pull from CPU queue
//    - A single worker pulls from OpenGL thread (it owns the OpenGL context)
//
//  To explain why this causes problems for mix'n'match, some background is necessary...
//
//  Since most platforms only support waiting for a single condition at a time
//  (WaitForMultipleObjects() makes Windows the exception), a set of workers/queues
//  must share the same condition_var... otherwise a worker waiting on one queue might
//  miss notifications that another queue has work available.
//
//  (The reason Boss exists is to manage a set of workers/queues which share the same
//   condition_var.  Of course, there's nothing that precludes having multiple Bosses).
//
//  The problem this poses for mix'n'match is that the wrong queue might get a notification.
//  In the example, if all workers are idle and a job is pushed into the OpenGL queue, there
//  is is only a 1 in N chance that the worker which pulls from the OpenGL thread will be
//  notified.  In the other N-1 cases, a different worker will wake up, check the general
//  CPU queue and find it still empty, and go back to sleep.  Meanwhile, the OpenGL thread
//  will remain dormant, even though there is queued work for it to do.
//
//  The simplest way to deal with this is to use condition_var::notify_all() instead of
//  notify_one().  However, this has the adverse effect of increasing contention, as multiple
//  workers wake up and compete for the same job.
//
//  Another potential approach is: when a woken worker does not find work to do, it notifies
//  a different worker to try.  There are multiple problems:
//    - condition variables have spurious wake-ups... a worker cannot tell whether it didn't find
//      work because it was looking in the wrong queue, or because it was woken spuriously.
//    - if spurious wake-ups didn't exist (they do!), it might be tempting to simply call
//      notify_one() to wake another worker when a worker cannot find a job.  However, this
//      would probably happen multiple times before the worker on the OpenGL thread is notified
//      (and depending on the implementation of condition_var, it's conceivable to ping-pong
//      indefinitely between two of the wrong workers).
//    - etc.
//
//  TODO: benchmark overhead of using notify_all()... how much worse is it?
//
//

#ifndef __schwa__job__boss__
#define __schwa__job__boss__


#include <condition_variable>
#include <mutex>


// namespace schwa::job
namespace schwa {namespace job {


class JobPool;
class Queue;
class Worker;

class Boss {
 public:
    Boss(JobPool& pool, int numQueues, int numWorkers);
    ~Boss();

    // Some random numbers that should be plenty big.
    const static uint32_t MAX_QUEUES = 8192;
    const static uint32_t MAX_WORKERS = 64;

    // Return the queue at the specified index.
    Queue& queueAt(int index);

 private:
    JobPool& _pool;

    std::condition_variable _cond;
    std::mutex _mutex;

    Queue* _queues;
    Worker* _workers;

    uint16_t _numQueues;
    uint16_t _numWorkers;
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__boss__