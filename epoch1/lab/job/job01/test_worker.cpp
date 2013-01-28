#include "job01/worker.h"

#include <chrono>
#include <iostream>

using namespace std;


class FakeJobPool {
public:
    class LocalPoolT {
    public:
        LocalPoolT(FakeJobPool& pool) : _pool(pool) { }

    protected:
        FakeJobPool& _pool;
    };
};

typedef Worker<FakeJobPool> FakeWorker;

int main(void) {
    FakeJobPool pool;
    FakeWorker worker(pool);

    worker.run();

    this_thread::sleep_for(chrono::milliseconds(10));

    worker.stop();

    cerr << "Called work() " << worker.count() << " times " << endl;

    return 0;
}