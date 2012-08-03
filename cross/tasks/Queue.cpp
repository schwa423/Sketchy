//
//  Queue.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//


// TODO:
// - ensure that all classes keep _count up to date
// - locked RoundRobinQueue calls count() on each queue in constructor...
//   how do we avoid deadlock (since if one of these queues has started
//   to add an item, it will be locked, and then try to call _owner->available()... ruh-roh!
//   - maybe available doesn't need to have count as an argument... then add() could call
//     it outside of the critical section... the only reason it's inside is to ensure that
//     the count remains valid (in case someone adds to the same queue first).
// - ensure that we can call both constructors of RoundRobinQueue:
//   - new RoundRobinQueue({ queue1, queue2, queue3 });
//     - should call && ... actually probably won't work
//       - maybe
//   - new RoundRobinQueue(std::vector<QueueOutPtr>{ q1, q2, q3})
//     - should call &&
//   - std::vector<QueueOutPtr> vec{ q1, q2, q3 }; new RoundRobinQueue(vec);
//     - should call &
//   - new RoundRobinQueue([]{ return std::vector<QueueOutPtr>{ q1, q2, q3} });
//     - should call &&
// - implement and test Task::yield()
// - implement at test Task dependencies.

#include "Queue.h"

#include <numeric>
#include <utility>
#include <cassert>

typedef std::lock_guard<std::mutex> lock_guard;
typedef std::unique_lock<std::mutex> unique_lock;

namespace Sketchy {
namespace Task {
    
    void QueueOwner::own(QueueOutPtr& gen) {
        _queues.push_back(gen);
    }


    void QueueOwner::disown(QueueOutPtr& q) {
        iterator it = find(_queues.begin(), _queues.end(), q);
        if (it != _queues.end()) _queues.erase(it);
        else throw std::logic_error("cannot disown queue we don't already own");
    }


    void QueueOut::setOwner(QueueOwner* owner) {
        lock_guard lock(_mutex);
        if (_owner == owner) return;
        auto thisptr = shared_from_this();
        if (_owner) _owner->disown(thisptr);
        _owner = owner;
        if (_owner) _owner->own(thisptr);
    }


    TaskPtr QueueOut::next() {
        unique_lock lock(_mutex);

        _cond.wait(lock, [&](){ return _count > 0; });

        if (!_count) return TaskPtr();
        else {
            _count--;
            return _next();
        }
    }


    TaskPtr QueueOut::next(int timeoutMsecs) {
        unique_lock lock(_mutex);

        std::chrono::milliseconds duration(timeoutMsecs);
        _cond.wait_for(lock, duration, [&](){ return _count > 0; });

        if (!_count) return TaskPtr();
        else {
            _count--;
            return _next();
        }
    }


    uint32_t QueueOut::count() {
        // TODO: is this necessary/sufficient?
        unique_lock lock(_mutex);
        return _count;
    }


    void Queue::add(TaskPtr& task) {
        lock_guard taskLock(task->_mutex);

        // Tasks can't change queues.
        assert(task->_queue == nullptr || task->_queue == this);
        task->_queue = this;
        if (task->_state == Task::Wait) return;

        lock_guard lock(_mutex);
        _q.push_back(task);
        _count = _q.size();
        if (_owner) _owner->available(*this);
        _cond.notify_one();
    }


    void Queue::add(TaskPtr&& task) {
        lock_guard taskLock(task->_mutex);
        if (task->_state != Task::Ready) return;

        lock_guard lock(_mutex);
        _q.push_back(std::move(task));
        _count = _q.size();
        if (_owner) _owner->available(*this);
        _cond.notify_one();
    }


    TaskPtr Queue::_next() {
        auto result = _q.front();
        _q.pop_front();
        _count = _q.size();
        return result;
    }


    void Queue::taskYielded(TaskPtr&& task) {
        lock_guard taskLock(task->_mutex);
        if (task->_state != Task::Ready) return;

        lock_guard lock(_mutex);
        _q.push_front(std::move(task));
        _count = _q.size();
        if (_owner) _owner->available(*this);
        _cond.notify_one();
    }


    CompoundQueue::CompoundQueue(const std::vector<QueueOutPtr>& queues, QueueOwner* owner)
    : QueueOut(owner), _subqueues(*this)
    {
        accumulateCounts();
        for (auto q : queues) { q->setOwner(&_subqueues); }
    }

    
    void CompoundQueue::accumulateCounts() {
        lock_guard lock(_mutex);
        _count = 0;
        for (QueueOutPtr& q : queues()) {
            _count += q->count();
        }
    }


    RoundRobinQueue::RoundRobinQueue(const std::vector<QueueOutPtr>& queues, QueueOwner* owner)
    : CompoundQueue(queues, owner), _index(0)
    {

    }


    TaskPtr RoundRobinQueue::_next() {
        int sz = queues().size();
        int start = _index <= sz ? _index : 0;

        // Find a non-empty queue, and pop from it.
        do {
            QueueOutPtr& q = queues()[_index];
            _index = (_index + 1) % sz;
            if (q->count() > 0) return q->next();
        } while(_index != start);
        // Oops.  Debuggin' time...
        throw std::logic_error("supposed to be guaranteed to find a task");
    }


    PriorityQueue::PriorityQueue(const std::vector<QueueOutPtr>& queues, QueueOwner* owner)
    : CompoundQueue(queues, owner)
    {

    }


    TaskPtr PriorityQueue::_next() {
        for (QueueOutPtr& q : queues()) {
            if (q->count() > 0) return q->next();
        }
        // Oops.  Debuggin' time...
        throw std::logic_error("supposed to be guaranteed to find a task");
    }


} // namespace Task {
} // namespace Sketchy {