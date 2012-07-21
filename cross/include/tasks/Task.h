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

class Task : public TaskObserver, public std::enable_shared_from_this<Task> {
public:		
    Task() : _state(Ready), _queue(nullptr) { }
    Task(std::vector<TaskPtr>& prereqs);
    Task(std::vector<TaskPtr>&& prereqs);  // TODO: necessary? actually, the lvalue version is
                                           // probably the superfluous one.

	// Called by user code when the task is no longer necessary.
    // Doesn't interrupt the task immediately if it is currently
    // running; instead waits for it to finish or yield.  Has no
    // effect if task is already in a terminal state (i.e. Done,
    // Cancel, or Error).
    void cancel();

    // Subclasses override this to do a specific type of work.
    virtual void run() = 0;

    // "Inherited" from TaskObserver; subclasses must implement.
    // Called when changes occur in prerequisites.
  	virtual void taskDone(const TaskPtr& task) = 0;
    virtual void taskCancel(const TaskPtr& task) = 0;
    virtual void taskError(const TaskPtr& task) = 0;

protected:
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

    std::vector<TaskPtr> _prereqs;

    void addObserver(std::shared_ptr<TaskObserver>&& observer);
    std::vector<std::weak_ptr<TaskObserver>> _observers;

    std::mutex _mutex;
    State _state;

    // Used to enforce that Task.run() calls precisely one of done(), yield(), or error().
    bool _endRun;

    Queue* _queue;
    
}; // class Task


} // namespace Task {
} // namespace Sketchy {


#endif  // #ifndef Sketchy_Task_h
