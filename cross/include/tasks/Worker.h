//
//  Worker.h
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Worker_h
#define Sketchy_Worker_h

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "Task.h"
#include "Queue.h"

namespace Sketchy {
namespace Task {

class Worker {

 public:
    Worker(std::shared_ptr<QueueOut> queue, int num_threads);
    ~Worker();

    // Start the worker.  Does nothing if already started.
    // Does not start if there is no queue.
    void start();

    // Stop the worker.  Does nothing if already stopped.
    void stop();

    // Stop the worker, set its queue to the new one,
    // and if it was previously running, restart it.
    // TODO: if you call start() when a worker has no
    //       queue, then set a queue, it won't be running
    //       since start() only starts the worker if there
    //       is a queue.  Better to manage thread lifetime
    //       independently of _running.  Eg: if there is no
    //       queue, start() sets _running to true but does
    //       not start threads.  If queue is later set, we
    //       notice that we need to start the threads (since
    //       queue != NULL and _running == _true).
    void setQueue(std::shared_ptr<QueueOut> queue);

    // TODO: these are for testing... can we do without?
    //       Maybe make the test class a friend, too.
    void runTask(TaskPtr& task) { task->work(); }
    void runTask(TaskPtr&& task) { task->work(); }

 private:
    // Mutex lock must be held while these are called.
    void lockedStart();
    void lockedStop();

    // Method run in a worker thread.
    void threadRun();

    std::mutex _mutex;
    std::condition_variable _cond;
    std::vector<std::thread*> _threads;
    int _count;
    bool _running;
    std::shared_ptr<QueueOut> _q;
};


} // namespace Task {
} // namespace Sketchy {


#endif // #ifndef Sketchy_Worker_h
