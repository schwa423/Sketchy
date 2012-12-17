//
//  boss.cpp
//  schwa::job
//
//  Created by Josh Gargus on 12/11/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "boss.h"

#include "queue2.h"
#include "worker2.h"
#include "schwassert.h"

#include <cstdlib>


// namespace schwa::job
namespace schwa {namespace job {


Boss::Boss(JobPool& pool, int numQueues, int numWorkers) :
    _pool(pool),
    _numQueues(numQueues), _queues(nullptr),
    _numWorkers(numWorkers), _workers(nullptr) {

    SCHWASSERT(numQueues > 0 && numQueues <= MAX_QUEUES, "invalid number of queues");
    SCHWASSERT(numWorkers > 0 && numWorkers <= MAX_WORKERS, "invalid number of workers");

    // Set up memory for queues and workers.
    size_t byteCount = numQueues * sizeof(Queue) + numWorkers * sizeof(Worker);
    uint8_t* mem = static_cast<uint8_t*>(malloc(byteCount));

    _queues = reinterpret_cast<Queue*>(mem);
    mem += (numQueues * sizeof(Queue));
    _workers = reinterpret_cast<Worker*>(mem);

    // Before initializing queues/workers, ensure memory was allocated.
    if (mem == nullptr) {
        _queues = nullptr;
        _workers = nullptr;
        _numQueues = 0;
        _numWorkers = 0;

        SCHWASSERT(mem != nullptr, "could not allocate space for Queues and Workers");
        return;
    }

    // Initialize the queues (using placement new).
    for (int i=0; i<numQueues; i++)
        new (&(_queues[i])) Queue(_pool, _cond, _mutex);

    // Now, initialize the workers (using placement new).
    for (int i=0; i<numWorkers; i++)
        new (&(_workers[i])) Worker(_queues, numQueues, _cond, _mutex);
}


Boss::~Boss() {
    // Destroy the queues and workers, invoking the destructor directly
    // since we constructed them all using placement new.
    for (int i=0; i<_numQueues; i++)
        _queues[i].~Queue();

    for (int i=0; i<_numWorkers; i++)
        _workers[i].~Worker();

    // No need to free _workers, because they're allocated from the same block of memory.
    if (_queues != nullptr)
        free(_queues);
}


Queue& Boss::queueAt(int index) {
    SCHWASSERT(index >= 0 && index < _numQueues, "no queue exists at specified index");
    return _queues[index];
}


}}  // namespace schwa::job