#include "job01/generic_worker.h"

using namespace schwa;

#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;


class FakeJobPool {
public:
    class LocalPoolT {
    public:
        LocalPoolT(FakeJobPool& pool) : _pool(pool) { }

    protected:
        FakeJobPool& _pool;
    };
};

class FakeJobQueue {
public:
    class LocalQueueT {
    public:
        LocalQueueT(FakeJobQueue& queue) : _queue(queue) { }

    protected:
        FakeJobQueue& _queue;
    };
};

typedef job::impl::GenericWorker<FakeJobPool, FakeJobQueue> FakeWorker;

int main(void) {
    FakeJobPool pool;
    FakeJobQueue queue;
    FakeWorker worker(pool, queue);

    worker.start();

    this_thread::sleep_for(milliseconds(10));

    worker.stop();

    cerr << "stopped worker... finishing!" << endl;

    return 0;
}