//
//  job_QueueTest.cpp
//  schwa::job::test
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//


#include "gtest/gtest.h"

#include "queue2.h"
#include "job.h"
#include "pool.h"


// namespace schwa::job
namespace schwa {namespace job {


TEST(job_QueueTest, SimpleCreation) {
    JobPool pool;


}


TEST(job_QueueTest, MoveSpawnInvalidatesJobId) {
    JobPool pool;
    std::condition_variable cond;
    std::mutex mutex;

    Queue q(pool, cond, mutex);

    JobData data;

    Spawn s1 = q.spawn(EmptyKernel, data);
    Spawn s2 = q.spawn(EmptyKernel, data);

    // The jobs should have different IDs to start off.
    EXPECT_EQ(s1.id() == s2.id(), false);

    // But this one should match the second job (because we just copied it).
    JobId id = s2.id();
    EXPECT_EQ(id == s2.id(), true);

    // Verify that moving s2 to s3 invalidates the JobId properly of s2.
    Spawn s3 = std::move(s2);
    EXPECT_EQ(id == s2.id(), false);
}


TEST(job_QueueTest, SimpleSpawn) {
    JobPool pool;
    std::condition_variable cond;
    std::mutex mutex;

    Queue q(pool, cond, mutex);

    JobData data;

    q.spawn(EmptyKernel, data);
    EXPECT_EQ(q.count(), 1);

    {
        Spawn s1 = q.spawn(EmptyKernel, data);
        Spawn s2 = q.spawn(EmptyKernel, data);

        // Nothing enqueue until Spawn destructors run.
        EXPECT_EQ(q.count(), 1);
    }
    // Now the jobs should be enqueued.
    EXPECT_EQ(q.count(), 3);
}


}}  // namespace schwa::job