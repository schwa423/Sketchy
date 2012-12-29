//
//  Gists_BenchmarkJobOverhead.h
//  schwa::gist
//
//  Created by Josh Gargus on 12/21/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//
//  Quick'n'dirty measurement of Job overhead in optimal circumstances
//  (i.e. single-threaded, with no lock contention).
//
//  Currently about 40us per job on iPad 2.
//
//

#include <memory>

#include "worker2.h"
#include "kernel.h"

#include "hackity.h"

#include <iostream>


// namespace schwa::gist
namespace schwa {namespace gist {


// Simple extension to Worker to allow us to run a single Job for testing.
class TestWorker : public job::Worker {
public:
    TestWorker(job::Queue* queues, int numQueues, std::condition_variable& cond, std::mutex& mutex)
    : job::Worker(queues, numQueues, cond, mutex) { }

    // Find a single job and run it.
    void work() {
        auto job = next();
        if (job) job->run();
    }
};


// Global queue accessible from SpawnKernel.
job::Queue* Gists_BenchmarkJobOverhead_Queue;

void SpawnKernel(job::JobData& input, job::Spawn& parent);
void SpawnKernel(job::JobData& input, job::Spawn& parent) {
    int spawnsRemaining = reinterpret_cast<int>(input._kernelData) - 1;
    if (spawnsRemaining < 0) return;

    job::JobData data;
    data._kernelData = reinterpret_cast<void*>(spawnsRemaining);

    Gists_BenchmarkJobOverhead_Queue->spawn(SpawnKernel, data);
}

void benchmarkJobOverhead();
void benchmarkJobOverhead() {
    job::JobPool pool;
    std::condition_variable cond;
    std::mutex mutex;

    job::Queue q(pool, cond, mutex);
    Gists_BenchmarkJobOverhead_Queue = &q;
    TestWorker w(&q, 1, cond, mutex);

    // Run all 10 million jobs.
    uint32_t elapsed = schwa::timeToRun([&]() {
        job::JobData data;
        data._kernelData = reinterpret_cast<void*>(9999);
        for (int i = 0; i < 1000; i++) {
            q.spawn(SpawnKernel, data);
        }

        for (int i = 0; i < 10000000; i++) {
            try {
                w.work();
            } catch(...) {
                std::cerr << "caught an exception!" << std::endl;
            }
        }
    });

    std::cerr << "Ran 10 million jobs in " << elapsed / 1000 << " milliseconds" << std::endl;

    std::cerr << "queue size: " << q.count() << std::endl;
    SCHWASSERT(q.count() == 0, "queue should be empty");
}


}}  // namespace schwa::gist