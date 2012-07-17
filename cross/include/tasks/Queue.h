//
//  Queue.h
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Queue_h
#define Sketchy_Queue_h

#include "Task.h"

#include <deque>
#include <memory>
#include <mutex>
#include <set>

namespace Sketchy {
namespace Task {

    
class Queue2;

class Generator;
typedef std::shared_ptr<Generator> GeneratorPtr;

// Queues can be composed of other queues.
// This class represents the owner of a queue
// which is notified whenever an item is enqueued.
class QueueOwner {
 public:
    QueueOwner() { }
    virtual ~QueueOwner() { }

    // Would like to make this protected, except that Section 11.5
    // of the C++-03 standard disallows my desired use (eg: see
    // Queue2::add()).
    virtual void available(Generator& gen) = 0;

 private:    
    friend class Generator;
    void own(GeneratorPtr& gen);
    void disown(GeneratorPtr& gen);
    std::set<GeneratorPtr> _owned;
};


// The "readable part" of a Queue... allows a consumer
// to pull items from the Queue, but provides no interface
// to enqueue these items.
class Generator : public std::enable_shared_from_this<Generator> {
 public:
    Generator(QueueOwner* owner = nullptr) : _owner(owner), _count(0) { }
    virtual ~Generator() { }

    // TODO: can we make this protected?  See ref to C++ standard Section 11.5 above
 //   QueueOwner& owner() { return _owner ? *_owner : *this; }

    void setOwner(QueueOwner* owner);

    TaskPtr next();
    uint32_t count();

 protected:
    // Subclasses must update accurately, while _mutex is locked.
    uint32_t _count;

    // Pop the "next" task off the queue (subclasses decide precisely what "next" means).
    // Invariants:
    // - at least one task is available; this is guaranteed by next()
    // - the mutex is locked while this is called
    virtual TaskPtr _next() = 0;

    std::mutex _mutex;
    std::condition_variable _cond;

    // Subclasses may access this, but should not set it.
    // TODO: make _owner a weak_ptr
    QueueOwner* _owner;
};


class Queue2 : public Generator {
 public:
    Queue2(QueueOwner* owner = nullptr) : Generator(owner) { }

    // Add a new task to the queue, and notify our owner
    // that we now have another task available.
    void add(TaskPtr& item);
    void add(TaskPtr&& item);

 protected:
    virtual TaskPtr _next();

 private:
    std::deque<TaskPtr> _q;

    friend class Task;
    void taskYielded(TaskPtr& task);
    void taskYielded(TaskPtr&& task);
};
    
    
class CompoundQueue : public QueueOwner, public Generator {
 public:
    CompoundQueue(QueueOwner* owner = nullptr) : Generator(owner) { }
    virtual ~CompoundQueue() { }

    virtual void available(Generator& gen) {
        if (_owner) _owner->available(*this);
        _count++;
        Generator::_cond.notify_one();
    }
};
    

class RoundRobinQueue : public CompoundQueue {
 public:
    RoundRobinQueue(std::vector<GeneratorPtr>& queues, QueueOwner* owner = nullptr); 
    RoundRobinQueue(std::vector<GeneratorPtr>&& queues, QueueOwner* owner = nullptr);
    RoundRobinQueue(QueueOwner* owner = nullptr) : CompoundQueue(owner), _index(0) { }

    // TODO: Later, provide ability to add/remove queues.
    void addQueue(GeneratorPtr& queue) { throw std::runtime_error("unimplemented"); }
    void addQueue(GeneratorPtr&& queue) { throw std::runtime_error("unimplemented"); }
    void removeQueue(GeneratorPtr& queue) { throw std::runtime_error("unimplemented"); }

private:
    // Implement abstract method.
    virtual TaskPtr _next();

    // Accumulate sum of counts in each queue.  Thread-safe, because we first acquire a lock,
    // and none of the queues can call available() on us because it also acquires a lock.
    void accumulateCounts();

    std::vector<GeneratorPtr> _queues;
    int _index;
};


} // namespace Task {
} // namespace Sketchy {


#endif // #ifndef Sketchy_Queue_h
