//
//  Task.h
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Task_h
#define Sketchy_Task_h

#include <memory>
#include <mutex>
#include <vector>

#include <iostream>

#include "base/Watcher.h"

namespace Sketchy {
namespace Task {

class Task;
typedef std::shared_ptr<Task> TaskPtr;

// Interface used to observe changes in a task's state.
class TaskObserver {
public:
    virtual void taskDone(const TaskPtr& task) = 0;
    virtual void taskCancel(const TaskPtr& task) = 0;
    virtual void taskError(const TaskPtr& task) = 0;
};



class Queue;

class Task : public std::enable_shared_from_this<Task> {
public:
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

protected:
    Task() : _state(Ready), _queue(nullptr), _prereqs(*this), _prereqs_remaining(0) { }
    virtual ~Task() {
        // TODO: mutex lock
        _prereqs.stopWatching();
    }

    // Called by static New() function of subclasses.
    void Init(std::vector<TaskPtr>& prereqs);

    // Subclasses must call precisely one of these during each invocation of Task.run()
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

    void addWatcher(TaskObserver* watcher);
    void removeWatcher(TaskObserver* watcher);

    std::set<TaskObserver*> _watchers;

    std::mutex _mutex;
    State _state;

    // Used to enforce that Task.run() calls precisely one of done(), yield(), or error().
    bool _endRun;

    Queue* _queue;

    class Prereqs : public Watcher<Task, Task, TaskObserver> {
     public:
        Prereqs(Task& owner) : Watcher<Task, Task, TaskObserver>(owner) { }
        virtual ~Prereqs();

        virtual void taskDone(const TaskPtr& task)   { ignore(task); _owner->prereqDone(task);   }
        virtual void taskCancel(const TaskPtr& task) { ignore(task); _owner->prereqCancel(task); }
        virtual void taskError(const TaskPtr& task)  { ignore(task); _owner->prereqError(task);  }
    };
    friend class Watcher<Task,Task, TaskObserver>;
    friend class Prereqs;
    Prereqs _prereqs;
    int _prereqs_remaining;

    void prereqDone(const TaskPtr& task);
    void prereqCancel(const TaskPtr& task);
    void prereqError(const TaskPtr& task);
    
}; // class Task


} // namespace Task {
} // namespace Sketchy {


#endif  // #ifndef Sketchy_Task_h
