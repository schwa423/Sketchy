#include "job01/core/link.h"
#include "job01/core/queue.h"

using namespace schwa;

#include <assert.h>

#include <iostream>


class TestLink : public job01::Link<TestLink> { 
 public:
    int index;
};

class TestQueue : public job01::core::Queue<TestLink> { 
 public:
 	TestQueue(Queue<TestLink>&& other) : Queue<TestLink>(std::move(other)) { }
    TestQueue() : Queue<TestLink>() { }
};


void testBasicEnqueueAndDequeue() {
    TestQueue::ElemT elem0, elem1, elem2, elem3, elem4;
    TestQueue queue;

    assert(queue.count() == 0);

    queue.add(&elem0);
    assert(queue.count() == 1);

    queue.add(&elem1);
    assert(queue.count() == 2);

    queue.add(&elem2);
    assert(queue.count() == 3);

    assert(queue.next() == &elem0);
    assert(queue.count() == 2);

    assert(queue.next() == &elem1);
    assert(queue.count() == 1);

    queue.add(&elem3);
    assert(queue.count() == 2);

    assert(queue.next() == &elem2);
    assert(queue.count() == 1);

    assert(queue.next() == &elem3);
    assert(queue.count() == 0);

    queue.add(&elem4);
    assert(queue.count() == 1);

    assert(queue.next() == &elem4);
    assert(queue.count() == 0);
}


void testMultiEnqueueAndDequeue() {
    TestQueue::ElemT elems[100];
    TestQueue queue;

    for (int i=0; i<100; i++) {
        elems[i].index = i;
        queue.add(&(elems[i]));
    }

    assert(queue.count() == 100);
    TestQueue queue10(queue.next(10));
    TestQueue queue80(queue.next(80));
    TestQueue queue20(queue.next(20));
    assert(queue.count() == 0);

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

    // The previous step should leave all three queues empty.
    assert(queue10.count() == 0);
    assert(queue80.count() == 0);
    assert(queue20.count() == 0);

    // Test adding multiple items to queue; the ordering should not change.
    for (int i = 0; i < 10; i++) {
        queue10.add(&(elems[i]));
    }
    for (int i = 10; i < 90; i++) {
        queue80.add(&(elems[i]));
    }
    for (int i = 90; i < 100; i++) {
        queue20.add(&(elems[i]));
    }
    queue.add(queue10);
    queue.add(queue80);
    queue.add(queue20);
    for (int i = 0; i < 100; i++) {
        assert(queue.next()->index == i);
    }
}


int main() {
    testBasicEnqueueAndDequeue();
    testMultiEnqueueAndDequeue();

    std::cerr << "job01/core/test_queue...  PASSED!" << std::endl << std::endl;
}

