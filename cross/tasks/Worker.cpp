//
//  Worker.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#include "Worker.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

typedef std::lock_guard<std::mutex> lock_guard;

namespace Sketchy {
namespace Task {

    Worker::Worker(std::shared_ptr<QueueOut> queue, int num_threads)
    : _q(queue), _count(num_threads), _running(false)
    {
        start();
    }


    Worker::~Worker() {
        stop();
    }


    void Worker::start() {
        lock_guard lock(_mutex);
        lockedStart();
    }
    void Worker::lockedStart() {
        if (_running || !_q.get()) return;
        _running = true;
        for (int i = 0; i < _count; i++) {
            _threads.push_back(new std::thread(&Worker::threadRun, this));
        }
    }


    void Worker::stop() {
        lock_guard lock(_mutex);
        lockedStop();
    }
    void Worker::lockedStop() {
        if (!_running) return;
        _running = false;
        for (auto th: _threads) { th->join(); }
        _threads.clear();
    }


    void Worker::setQueue(std::shared_ptr<QueueOut> queue) {
        lock_guard lock(_mutex);
        bool wasRunning = _running;
        lockedStop();
        _q = queue;
        if (wasRunning) lockedStart();
    }


    void Worker::threadRun() {
        {
            // Memory barrier to guarantee that this thread sees that _running == true.
            // TODO: necessary?
            lock_guard lock(_mutex);
        }

        while (_running) {
            TaskPtr task = _q->next(10);

            // TODO: could potentially check _running again, but then
            //       we'd incur the complexity of either cancelling the
            //       task, or (probably better) pushing it back in the
            //       queue whence it came (as though it yielded).
            if (task.get()) task->work();
        }
    }


} // namespace Task {
} // namespace Sketchy {
