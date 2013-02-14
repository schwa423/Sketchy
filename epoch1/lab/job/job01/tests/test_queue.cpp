#include "job01/queue.h"
#include "job01/link.h"
using namespace schwa;

#include <assert.h>

#include <iostream>
using namespace std;

/*
template <class HighPriorityQueueT, class LowPriorityQueueT>
class GenericTestQueue {




private:



};
*/


// TODO: comment
class TestJob : public job01::Link<TestJob> {

};


// TODO: comment
class TestQueue : public job01::SimpleQueue<TestJob> {
public:
    TestQueue(SimpleQueue<TestJob>&& other) : SimpleQueue<TestJob>(std::move(other)) { }
    TestQueue() : SimpleQueue<TestJob>() { }

protected:

};


// TODO: probably unnecessary
class TestBossQueue : public job01::BossQueue<TestJob> {
public:
    TestBossQueue() : BossQueue<TestJob>() { }

protected:

};



void testBasicEnqueueAndDequeue() {
    TestQueue::JobT job0, job1, job2, job3, job4;
    TestQueue queue;

    assert(queue.count() == 0);

    queue.add(&job0);
    assert(queue.count() == 1);

    queue.add(&job1);
    assert(queue.count() == 2);

    queue.add(&job2);
    assert(queue.count() == 3);

    assert(queue.next() == &job0);
    assert(queue.count() == 2);

    assert(queue.next() == &job1);
    assert(queue.count() == 1);

    queue.add(&job3);
    assert(queue.count() == 2);

    assert(queue.next() == &job2);
    assert(queue.count() == 1);

    assert(queue.next() == &job3);
    assert(queue.count() == 0);

    queue.add(&job4);
    assert(queue.count() == 1);

    assert(queue.next() == &job4);
    assert(queue.count() == 0);
}


void testMultiEnqueueAndDequeue() {
    TestQueue::JobT jobs[100];
    TestQueue queue;

    for (int i=0; i<100; i++) {
        queue.add(&(jobs[i]));
    }

    assert(queue.count() == 100);

    TestQueue queue10(queue.next(10));
    TestQueue queue80(queue.next(80));
    TestQueue queue20(queue.next(20));

    // These add up to 100 because queue20 received only 10, not the requested 20.
    assert(queue20.count() == 10);
    assert(queue10.count() + queue80.count() + queue20.count() == 100);

    // Ensure that when we count them one-by-one, we still get the same numbers.
    int count = 0;
    while (queue10.next()) count++;
    assert(count == 10);
    while (queue80.next()) count++;
    assert(count == 90);
    while (queue20.next()) count++;
    assert(count == 100);

    // Need to add multiple items to queue.
    assert(false);

}


int main() {
    testBasicEnqueueAndDequeue();
    testMultiEnqueueAndDequeue();

    cerr << "job01/test_queue...  PASSED!" << endl << endl;
}

