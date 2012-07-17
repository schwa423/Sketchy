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
//   - new RoundRobinQueue(std::vector<GeneratorPtr>{ q1, q2, q3})
//     - should call &&
//   - std::vector<GeneratorPtr> vec{ q1, q2, q3 }; new RoundRobinQueue(vec);
//     - should call &
//   - new RoundRobinQueue([]{ return std::vector<GeneratorPtr>{ q1, q2, q3} });
//     - should call &&
// - implement and test Task::yield()
// - implement at test Task dependencies.

#include "Queue.h"

#include <numeric>
#include <utility>

typedef std::lock_guard<std::mutex> lock_guard;
typedef std::unique_lock<std::mutex> unique_lock;

namespace Sketchy {
namespace Task {
    
    void QueueOwner::own(GeneratorPtr& gen) {
        _owned.insert(gen);
    }


    void QueueOwner::disown(GeneratorPtr& gen) {
        int erased = _owned.erase(gen);
        if (!erased) throw std::logic_error("cannot disown generator we don't already own");
    }


    void Generator::setOwner(QueueOwner* owner) {
        lock_guard lock(_mutex);
        if (_owner == owner) return;
        auto thisptr = shared_from_this();
        if (_owner) _owner->disown(thisptr);
        _owner = owner;
        if (_owner) _owner->own(thisptr);
    }


    TaskPtr Generator::next() {
        unique_lock lock(_mutex);

        // TODO:
        // Would rather use a lambda than this bullshit,
        // but Clang crashes when trying to compile it.
        //_cond.wait(lock, [](){ return _count > 0; });
        class NotEmptyPred {
        public:
            NotEmptyPred(uint32_t& count) : _val(count) { }
            bool operator()() const { return _val > 0; }

        private:
            QueueOwner *_owner;
            uint32_t& _val;
        };
        NotEmptyPred pred(_count);
        _cond.wait(lock, pred);
        
        _count--;
        return _next();
    }


    uint32_t Generator::count() {
        unique_lock lock(_mutex);
        return _count;
    }


    void Queue2::add(TaskPtr&& item) {
        lock_guard lock(_mutex);
        _q.push_back(item);
        _count = _q.size();
        if (_owner) _owner->available(*this);
        _cond.notify_one();
    }


    void Queue2::add(TaskPtr& item) {
        lock_guard lock(_mutex);
        _q.push_back(item);
        _count = _q.size();
        if (_owner) _owner->available(*this);
        _cond.notify_one();
    }


    TaskPtr Queue2::_next() {
        auto result = _q.front();
        _q.pop_front();
        _count = _q.size();
        return result;
    }


    void Queue2::taskYielded(TaskPtr& task) {
        lock_guard lock(_mutex);
        _q.push_front(task);
    }


    void Queue2::taskYielded(TaskPtr&& task) {
        lock_guard lock(_mutex);
        _q.push_front(std::move(task));
    }


    RoundRobinQueue::RoundRobinQueue(std::vector<GeneratorPtr>& queues, QueueOwner* owner) :
        CompoundQueue(owner), _index(0), _queues(queues) 
    {  
        accumulateCounts();
        for (auto q : _queues) { q->setOwner(this); }
    }


    RoundRobinQueue::RoundRobinQueue(std::vector<GeneratorPtr>&& queues, QueueOwner* owner) :
        CompoundQueue(owner), _index(0), _queues(queues)
    {
        accumulateCounts();
    }


    TaskPtr RoundRobinQueue::_next() {
        int queues = _queues.size();
        int start = _index <= queues ? _index : 0;

        // Find a non-empty queue, and pop from it.
        do {
            GeneratorPtr& q = _queues[_index];
            _index = (_index + 1) % queues;
            if (q->count() > 0) return q->next();
        } while(_index != start);
        
        // Oops.  Debuggin' time...
        throw std::logic_error("supposed to be guaranteed to find a task");
    }

    void RoundRobinQueue::accumulateCounts() {
        lock_guard lock(_mutex);
        _count = 0;
        for (GeneratorPtr& q : _queues) {
            _count += q->count();
        }
    }


} // namespace Task {
} // namespace Sketchy {