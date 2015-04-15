//
//  impl_queue.cpp
//  schwa::job
//
//  Created by Josh Gargus on 12/13/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "impl_queue.h"
#include "job.h"

#include <iostream>
#include <condition_variable>
#include <mutex>
typedef std::lock_guard<std::mutex> lock_guard;


// namespace schwa::job::impl
namespace schwa {namespace job {namespace impl {


Queue::Queue(std::condition_variable& cond, std::mutex& mutex) :
    _back(nullptr), _front(nullptr),
    _cond(cond), _mutex(mutex),
    _count(0) {

}


void Queue::enqueue(Job* job) {
    // Safe to unlink new job before we obtain lock.
    unlink(job);

    lock_guard lock(_mutex);

    if (!_back) {
        _back = _front = job;
    } else {
        link(_back, job);
        _back = job;
    }

    _count++;
    _cond.notify_one();
}


void Queue::requeue(Job* job) {
    lock_guard lock(_mutex);

    link(job, _front);
    _front = job;
    if (!_back)
        _back = job;

    _count++;
    _cond.notify_one();
}


Job* Queue::next() {
   lock_guard lock(_mutex);
   return unlockedNext();
}


}}}  // namespace schwa::job::impl