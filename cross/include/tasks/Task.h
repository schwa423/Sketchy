//
//  Task.h
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#ifndef Sketchy_Task_h
#define Sketchy_Task_h

#include <memory>
#include <mutex>
#include <vector>
#include <set>

#include <iostream>

namespace Sketchy {
namespace Task {

class Task;
typedef std::shared_ptr<Task> TaskPtr;

// Interface used to observe changes in a task's state.
class TaskObserver {
public:
    typedef std::shared_ptr<TaskObserver> ptr;
    typedef std::weak_ptr<TaskObserver> weak_ptr;

    virtual void taskDone(const TaskPtr& task) = 0;
    virtual void taskCancel(const TaskPtr& task) = 0;
    virtual void taskError(const TaskPtr& task) = 0;
};


class Queue;

// Note: I would rather make TaskObserver a protected base class,
//       and to cast shared_from_this() to shared_ptr<TaskObserver>.
//       The goal is POLA... I don't want just anyone with a ref to
//       a Task to call these TaskObserver methods on it.  However,
//       C++ disallows this... why?  Probably for some "good" reason,
//       but yet again it gets in the way of a perfectly reasonable
//       use-case.
class Task : public TaskObserver, public std::enable_shared_from_this<Task> {
public:
    typedef std::shared_ptr<Task> ptr;
    typedef std::weak_ptr<Task> weak_ptr;

	// Called by user code when the task is no longer necessary.
    // Doesn't interrupt the task immediately if it is currently
    // running; instead waits for it to finish or yield.  Has no
    // effect if task is already in a terminal state (i.e. Done,
    // Cancel, or Error).
    void cancel();

    // Subclasses override this to do a specific type of work.
    virtual void run() = 0;

    bool isDone()      { return _state == Done; }
    bool isCancelled() { return _state == Cancel; }
    bool isError()     { return _state == Error; }
    // Is the task in a terminal state (Done, Cancel, or Error)?
    bool isSettled()   { return _state >= Done; }

    void addObserver(const TaskObserver::weak_ptr& observer);

    // A prereq was finished, cancelled, or encountered an error.
    // NOTE: should only be called by a task that we have registered
    // ourself with as a TaskObserver, but I couldn't figure out how
    // to enforce this with the C++ type system (ideally, I would
    // make TaskObserver a protected base class, but then I can't
    // upcast myself to TaskObserver... grrr).
    virtual void taskDone(const TaskPtr& task);
    virtual void taskCancel(const TaskPtr& task);
    virtual void taskError(const TaskPtr& task);

protected:
    Task() : _state(Ready), _queue(nullptr), _prereqs_remaining(0) { }
    virtual ~Task() { }

    // Called by static New() function of subclasses.
    void Init(std::vector<TaskPtr>& prereqs);

    // Subclasses must call precisely one of these during each invocation of run().
    void done();
    void yield();
    void error();

private:
    // The states that a task may be in.
    //   Wait: task is waiting for prequisites to be fulfilled
    //     -> Ready: when final prerequisite is fulfilled)
    //     -> Cancel: when cancel() is called by client code 
    //   Ready: task is scheduled to run
    //     -> Run: when worker dequeues the task and runs it
    //     -> Cancel: when cancel() is called by client code 
    //   Run: task is currently running
    //     -> Done: when task is completed: run() calls done()
    //     -> Ready: when task cooperatively yields: run() calls yield()
    //     -> Error: when uncaught exception occurs during run()
    //     -> Cancel: when cancel() is called by client code
    //   Done: task sucessfully completed
    //       (terminal state)
    //   Cancel: task was cancelled
    //       (terminal state)
    //   Error: task encountered an error during execution
    //       (terminal state)
    enum State { Wait, Ready, Run, Done, Cancel, Error };

	friend class Worker;
    friend class Queue;

	// Called by Worker to do some work.
	void work();

    std::mutex _mutex;
    State _state;

    // Used to enforce that Task.run() calls precisely one of done(), yield(), or error().
    bool _endRun;

    Queue* _queue;

    int _prereqs_remaining;
    std::set<TaskPtr> _prereqs;
    std::vector<TaskObserver::weak_ptr> _observers;

}; // class Task


} // namespace Task {
} // namespace Sketchy {


#endif  // #ifndef Sketchy_Task_h
