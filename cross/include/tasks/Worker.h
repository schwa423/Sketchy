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
    Worker(int num_threads);
    ~Worker();

    void runTask(TaskPtr& task) { task->work(); }
    void runTask(TaskPtr&& task) { task->work(); }

 private:

    // Method run in a worker thread.
    void run();

    std::mutex _mutex;
    std::condition_variable _cond;
    std::vector<std::thread*> _threads;

    Queue2 _q;
};


} // namespace Task {
} // namespace Sketchy {


#endif // #ifndef Sketchy_Worker_h
