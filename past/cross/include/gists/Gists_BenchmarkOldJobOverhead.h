//
//  Gists_BenchmarkOldJobOverhead.h
//  schwa::gist
//
//  Created by Josh Gargus on 12/21/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//
//  Benchmark the old job framework, to have a rough idea how bad
//  it is before I delete this stuff.
//
//

#include "Task.h"
#include "Queue.h"
#include "Worker.h"

#include "schwassert.h"
#include "hackity.h"


// namespace schwa::gist
namespace schwa {namespace gist {

    class SpawnTask : public Sketchy::Task::Task {
     public:
        SpawnTask(Sketchy::Task::Queue& queue, int spawnsRemaining)
        : _queue(queue), _spawnsRemaining(spawnsRemaining) { }

        virtual void run() {
            if (_spawnsRemaining > 0) {
                _queue.add(Sketchy::Task::TaskPtr(new SpawnTask(_queue, _spawnsRemaining-1)));
            }
            done();
        }

     protected:
        Sketchy::Task::Queue& _queue;
        int _spawnsRemaining;
    };

    void benchmarkOldJobOverhead();
    void benchmarkOldJobOverhead() {

        std::shared_ptr<Sketchy::Task::Queue> qptr(new Sketchy::Task::Queue);
        Sketchy::Task::Worker w(qptr, 0);

        uint32_t elapsed = schwa::timeToRun([&]() {
            Sketchy::Task::Queue& q = *(qptr.get());

            for (int i = 0; i < 1000; i++) {
                Sketchy::Task::TaskPtr ptr(new SpawnTask(q, 9999));
                q.add(std::move(ptr));
                std::cerr << "Queue now has " << q.count() << " enqueued tasks" << std::endl;
            }

            for (int i = 0; i < 10000000; i++) {
                if (i%1000000 == 0) std::cerr << "i: " << i << std::endl;
                Sketchy::Task::TaskPtr ptr = q.next(10);
                w.runTask(ptr);
            }
        });

        SCHWASSERT(qptr->count() == 0, "there are still tasks in the queue");

        std::cerr << "Ran 10 million tasks in " << elapsed / 1000 << " milliseconds" << std::endl;
    }


}}  // namespace schwa::gist