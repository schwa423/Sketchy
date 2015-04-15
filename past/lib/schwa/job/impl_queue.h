//
//  impl_queue.h
//  schwa::job
//
//  Created by Josh Gargus on 12/13/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__job__impl_queue__
#define __schwa__job__impl_queue__


#include "free_list.h"
#include "job.h"

#include <condition_variable>
#include <mutex>


// namespace schwa::job::impl
namespace schwa {namespace job {namespace impl {


// This is used internally by the other classes of schwa::job,
// but is not exposed to the app developer.
class Queue : public mem::Linker {
public:
    explicit Queue(std::condition_variable& cond, std::mutex& mutex);

    // Add job to the back of the queue.
    void enqueue(Job* job);

    // Add job to the front of the queue.  This is used when a job yields
    // to give higher-priority jobs a chance to run; if there are none, it
    // will immediately run again.
    void requeue(Job* job);

    // Obtain job from front of the queue, or nullptr if no job is queued.
    // No locking is performed; the caller must have a reference to the same mutex.
    inline Job* unlockedNext();

    // Obtain job from front of the queue, or nullptr if no job is queued.
    Job* next();

    // For testing.
    unsigned count() const { return _count; }

private:
    Job* _back;
    Job* _front;

    std::condition_variable& _cond;
    std::mutex& _mutex;

    unsigned _count;
};


// Inline method definitions /////////////////////////////////////////////////////

Job* Queue::unlockedNext() {
    // Return immediately if there is no queued job.
    if (!_front)
        return nullptr;

    // Grab the job to return.
    _count--;
    auto next = _front;

    // Unlink it from the queue, and update the front of the queue.
    _front = static_cast<Job*>(unlink(_front));

    // If there is no front, there is no back either (queue is empty).
    if (!_front)
        _back = nullptr;

    return next;
}


}}}  // namespace schwa::job::impl


#endif  // #ifndef __schwa__job__impl_queue__