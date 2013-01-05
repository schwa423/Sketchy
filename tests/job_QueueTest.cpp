//
//  job_QueueTest.cpp
//  schwa::job::test
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//


#include "gtest/gtest.h"

#include "queue2.h"
#include "job.h"
#include "pool.h"

#include "mem.h"

#include <atomic>
using std::atomic;

// namespace schwa::job
namespace schwa {namespace job {


TEST(job_QueueTest, SimpleCreation) {
    JobPool pool;


}


TEST(job_QueueTest, MoveSpawnInvalidatesJobId) {
    
    struct foo {
        int f1;
        int f2;
//        void* f3;
//        void* f4;
//        void* f5;
//        void* f6;
//        void* f7;
    };
    
    struct bar {
        void* p1;
        void* p2;
    };
    
    atomic<int> a;
    atomic<bool> b;
    atomic<void*> c;
    atomic<foo> d;
    atomic<bar> e;
    
    EXPECT_TRUE(a.is_lock_free());
    EXPECT_TRUE(b.is_lock_free());
    EXPECT_TRUE(c.is_lock_free());
    EXPECT_TRUE(d.is_lock_free());
    EXPECT_TRUE(e.is_lock_free());
    
    EXPECT_EQ(CACHE_LINE_SIZE, 64);
    
    
    
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