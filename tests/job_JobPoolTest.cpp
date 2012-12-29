//
//  job_JobPoolTest.cpp
//  schwa::job::test
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//


#include "gtest/gtest.h"
#include "job.h"
#include "pool.h"

#include "schwassert.h"


// namespace schwa::job
namespace schwa {namespace job {


TEST(job_JobPoolTest, SimpleObtainAndRelease) {
    // Set up job pool, and verify that initial number of free jobs is as expected.
    JobPool pool;
    EXPECT_EQ(pool.freeCount(), JobPool::MAX_JOBS);

    std::vector<Job*> outstanding;

    JobData data;
    Queue* q = nullptr;

    // Obtain jobs from the pool.
    for (int i=0; i<JobPool::MAX_JOBS; i++) {
        Job* job = pool.obtain(EmptyKernel, data, q);
        EXPECT_NE(job, nullptr);
        // If we failed here, this is serious... just bail instead
        // of bulletproofing the rest of the test against nullptr.
        if (!job) return;
        EXPECT_EQ(pool.freeCount(), JobPool::MAX_JOBS - i - 1);
        EXPECT_EQ(job->generation(), 1);
        outstanding.push_back(job);
    }

    // We've exhausted all of the jobs in the pool...
    // when we try to obtain one, bad_alloc should be thrown.
    bool allocFailed;;
    try {
        pool.obtain(EmptyKernel, data, q);
        allocFailed = false;
    } catch(std::bad_alloc ex) {
        allocFailed = true;
    }
    EXPECT_EQ(allocFailed, true);

    // Release jobs back to the pool.
    for (int i=0; i<JobPool::MAX_JOBS; i++) {
        Job* job = outstanding[i];
        EXPECT_EQ(job->nextLink(), nullptr);
        pool.release(job);
        EXPECT_EQ(pool.freeCount(), i+1);
        EXPECT_EQ(job->generation(), 2);
        if (i > 0) {
            EXPECT_NE(job->nextLink(), nullptr);
        } else {
            EXPECT_EQ(job->nextLink(), nullptr);
        }
    }
    outstanding.clear();

    // Same thing again... don't bother stashing, just check generation.
    for (int i=0; i<JobPool::MAX_JOBS; i++) {
        Job* job = pool.obtain(EmptyKernel, data, q);
        EXPECT_NE(job, nullptr);
        // If we failed here, this is serious... just bail instead
        // of bulletproofing the rest of the test against nullptr.
        if (!job) return;
        EXPECT_EQ(pool.freeCount(), JobPool::MAX_JOBS - i - 1);
        EXPECT_EQ(job->generation(), 3);
    }
}


}}  // namespace schwa::job