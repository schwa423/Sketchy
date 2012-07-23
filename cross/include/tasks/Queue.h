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

    
class Queue;
class QueueOut;
typedef std::shared_ptr<QueueOut> QueueOutPtr;
typedef std::shared_ptr<Queue> QueuePtr;

// Queues can be composed of other queues.
// This class represents the owner of a queue
// which is notified whenever an item is enqueued.
class QueueOwner {
 public:
    QueueOwner() { }
    virtual ~QueueOwner() { }

    // Would like to make this protected, except that Section 11.5
    // of the C++-03 standard disallows my desired use (eg: see
    // Queue::add()).
    virtual void available(QueueOut& q) = 0;

 protected:
    typedef std::vector<QueueOutPtr>::iterator iterator;
    typedef std::pair<iterator, iterator> iterator_pair;
    iterator_pair iterators() { return iterator_pair(_queues.begin(), _queues.end()); }

    friend class QueueOut;
    void own(QueueOutPtr& q);
    void disown(QueueOutPtr& q);
    std::vector<QueueOutPtr> _queues;
};


// The "readable part" of a Queue... allows a consumer
// to pull items from the Queue, but provides no interface
// to enqueue these items.
class QueueOut : public std::enable_shared_from_this<QueueOut> {
 public:
    QueueOut(QueueOwner* owner = nullptr) : _owner(owner), _count(0) { }
    virtual ~QueueOut() { }

    void setOwner(QueueOwner* owner);

    TaskPtr next();
    TaskPtr next(int timeoutMsecs);
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


class Queue : public QueueOut {
 public:
    Queue(QueueOwner* owner = nullptr) : QueueOut(owner) { }

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
    
    
// Base class for queues that draw tasks from other queues.
// Subclasses implement different policies to decide which
// queue to get the next task from.
class CompoundQueue : public QueueOut {
 public:
    CompoundQueue(const std::vector<QueueOutPtr>& queues, QueueOwner* owner = nullptr);
    virtual ~CompoundQueue() { }

    // TODO: Later, provide ability to add/remove queues.
    //       Be sure to update _count appropriately!
    void addQueue(QueueOutPtr& queue) { throw std::runtime_error("unimplemented"); }
    void addQueue(QueueOutPtr&& queue) { throw std::runtime_error("unimplemented"); }
    void removeQueue(QueueOutPtr& queue) { throw std::runtime_error("unimplemented"); }

 protected:
    // Helper class which holds the queues that this compound-queue draws
    // from, and acts as a limited interface through which to be notified
    // of newly-available tasks.
    class Queues : public QueueOwner {
        friend class CompoundQueue;
     public:
        Queues(CompoundQueue& owner) : _owner(owner) { }
        virtual ~Queues() { }

        virtual void available(QueueOut& q) { _owner.taskAvailable(q); }

     private:
        CompoundQueue& _owner;
    };
    Queues _subqueues;
    friend class Queues;

    std::vector<QueueOutPtr>& queues() { return _subqueues._queues; }

 private:
    // Accumulate sum of counts in each queue.  Thread-safe, because we first acquire a lock,
    // and none of the queues can call available() on us because it also acquires a lock.
    void accumulateCounts();

    // Called by our helper class Queues.
    void taskAvailable(QueueOut& q) {
        if (_owner) _owner->available(*this);
        _count++;
        _cond.notify_one();
    }
};
    

// CompoundQueue which generates its next task by looping through its list of queues,
// and pulling a task from the first non-empty queue.  When generating the next task
// after that, it starts with the next queue (i.e. the queue following the one that
// the previous task was drawn from).
class RoundRobinQueue : public CompoundQueue {
 public:
    RoundRobinQueue(const std::vector<QueueOutPtr>& queues, QueueOwner* owner = nullptr);

private:
    // Implement abstract method.
    virtual TaskPtr _next();
    int _index;
};


// CompoundQueue which generates its next task by pulling one from its highest-priority
// non-empty queue.  For now, there is no convenient interface for dynamically repriortizing
// queues.  Instead, the queues' relative priorities are derived from the order they are added
// to the PriorityQueue; queues added earlier have higher priorities than those added later.
// TODO: test me please!
class PriorityQueue : public CompoundQueue {
 public:
    PriorityQueue(const std::vector<QueueOutPtr>& queues, QueueOwner* owner = nullptr);

 private:
    // Implement abstract method.
    virtual TaskPtr _next();
};


} // namespace Task {
} // namespace Sketchy {


#endif // #ifndef Sketchy_Queue_h
