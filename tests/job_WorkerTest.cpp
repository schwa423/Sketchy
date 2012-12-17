//
//  job_WorkerTest.cpp
//  schwa::job::test
//
//  Created by Josh Gargus on 12/22/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//


#include "gtest/gtest.h"

#include "queue2.h"
#include "worker2.h"
#include "job.h"
#include "pool.h"


// namespace schwa::job
namespace schwa {namespace job {

    // Simple extension to Worker to allow us to run a single Job for testing.
    class TestWorker : public Worker {
     public:
        TestWorker(Queue* queues, int numQueues, std::condition_variable& cond, std::mutex& mutex)
        : Worker(queues, numQueues, cond, mutex) { }

        // Find a single job and run it.
        void work() {
            Job* job = next();
            if (job) job->run();
        }
    };


    TEST(job_WorkerTest, MultipleChildren) {
        JobPool pool;
        std::condition_variable cond;
        std::mutex mutex;

        Queue q(pool, cond, mutex);

        TestWorker w1(&q, 1, cond, mutex);
        TestWorker w2(&q, 1, cond, mutex);

        bool finished = false;

        JobData data;

        {
            // We can spawn lambdas!
            Spawn parent = q.spawn([&finished](Spawn& spawn) {
                finished = true;
            });

            // Ensure sanity of the test.  The values won't necessarily reflect the
            // precise number of children/grandchildren in the queue, but will let
            // us keep track of the total number queued/dequeued.
            int children = 0;
            int grandchildren = 0;

            for (int i = 0; i < 10; i++) {
                Spawn child = parent.spawn(EmptyKernel, data);
                children++;
                for (int j = 0; j < 10; j++) {
                    Spawn grandchild = child.spawn(EmptyKernel, data);
                    grandchildren++;
                }
            }

            // All of the grandchildren should be enqueued.  None of the children
            // should, since they're waiting for the grandchildren.
            EXPECT_EQ(q.count(), 100);

            // Run 9 grandchildren; queue should shrink by 9.
            for (int i = 0; i < 4; i++) {
                w1.work();
                w2.work();
                grandchildren -= 2;
            }
            w1.work();
            grandchildren--;
            EXPECT_EQ(q.count(), 91);

            // Run one more grandchild; queue should remain the same size
            // because now that 10 grandchildren are finished, one child is
            // now enqueued.
            w2.work();
            grandchildren--;
            EXPECT_EQ(q.count(), 91);

            // Run child... now only grandchildren are enqueued.
            w1.work();
            children--;
            EXPECT_EQ(q.count(), 90);

            // Run grandchildren and all but one child... that child will be
            // the only job in the queue.
            for (int i = 0; i < 45; i++) {
                w1.work();
                w2.work();
                grandchildren -= 2;
            }
            for (int i = 0; i < 4; i++) {
                w1.work();
                w2.work();
                children -= 2;
            }
            EXPECT_EQ(q.count(), 1);

            // Run the last child.  This won't quite enqueue the parent, because
            // the parent Spawn still exists.
            w1.work();
            children--;
            EXPECT_EQ(q.count(), 0);

            // Ensure that the test is sane.
            EXPECT_EQ(children + grandchildren, 0);
        }

        // Now the parent should be enqueued.  Run it!
        EXPECT_EQ(q.count(), 1);
        EXPECT_EQ(finished, false);
        w2.work();
        EXPECT_EQ(q.count(), 0);
        EXPECT_EQ(finished, true);
    }


    // Test that we can spawn lambda-jobs from both inside and outside of other Spawns.
    TEST(job_WorkerTest, LambdasOfLambdas) {
        JobPool pool;

        std::condition_variable cond;
        std::mutex mutex;

        Queue q(pool, cond, mutex);

        TestWorker w(&q, 1, cond, mutex);

        int outsideChildren = 0;
        int insideChildren = 0;

        int outsideGrandChildren = 0;
        int insideGrandChildren = 0;

        // All spawning done in a nested scope, otherwise "parent" woudn't
        // be enqueued until the end of this test-method.
        {
            Spawn parent = q.spawn([&insideChildren, &insideGrandChildren](Spawn& spawn) {
                for (int i = 0; i < 5; i++) {
                    // Spawn an "inside child" that spawns two more "inside children",
                    // for a total of 5 children and 10 grandchildren.
                    Spawn inside = spawn.spawn([&insideChildren, &insideGrandChildren](Spawn& spawn) {
                        insideChildren++;

                        spawn.spawn([&insideGrandChildren](Spawn& spawn) {
                            insideGrandChildren++;
                        });

                        spawn.spawn([&insideGrandChildren](Spawn& spawn) {
                            insideGrandChildren++;
                        });
                    });
                }
            });
            for (int i=0; i<10; i++) {
                // Spawn an "outside child" and another from it, for a total of
                // 10 children and 10 grandchildren.
                Spawn outside = parent.spawn([&outsideChildren](Spawn& spawn) {
                    outsideChildren++;
                });

                outside.spawn([&outsideGrandChildren](Spawn& spawn) {
                    outsideGrandChildren++;
                });
            }
        }

        // We've spawned the parent, plus the 10 outside children,
        // However, the parent isn't enqueued until the children are done.
        // Furthermore, the children aren't enqueued until the grandchildren are done.
        EXPECT_EQ(10, q.count());

        // As each grandchild finishes, its parent is immediately enqueued.
        for (int i=0; i<10; i++) {
            w.work();
            EXPECT_EQ(i+1, outsideGrandChildren);
        }
        EXPECT_EQ(q.count(), 10);
        for (int i=0; i<10; i++) {
            w.work();
            EXPECT_EQ(i+1, outsideChildren);
        }

        // Now, all of the outside-children/grandchildren are done,
        // and only the parent is on the queue.
        EXPECT_EQ(10, outsideChildren);
        EXPECT_EQ(10, outsideGrandChildren);
        EXPECT_EQ(1, q.count());

        // When we run the parent, 5 children are enqueued.
        w.work();
        EXPECT_EQ(5, q.count());

        // When each of these children run, they spawn two grandchildren
        // to the back of the queue.
        for (int i = 0; i < 5; i++) {
            w.work();
            EXPECT_EQ(i+1, insideChildren);
        }
        // All 10 "inside grandchildren" are enqueued; none of them have run.
        EXPECT_EQ(10, q.count());
        EXPECT_EQ(0, insideGrandChildren);

        // Once we run them, the queue will be empty.
        for (int i = 0; i < 10; i++) {
            w.work();
            EXPECT_EQ(i+1, insideGrandChildren);
        }
        EXPECT_EQ(0, q.count());
    }


}}  // namespace schwa::job