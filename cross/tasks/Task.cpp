//
//  Task.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <stdexcept>
#include <iostream>
using std::cerr;
using std::endl;

#include "Task.h"
#include "Queue.h"

typedef std::lock_guard<std::mutex> lock_guard;
typedef std::unique_lock<std::mutex> unique_lock;

// TODO: things to test:
//  - error if run doesn't call done(), yield(), or error()
//  - observer called properly if added when task is:
//    - not in terminal state yet
//    - already in terminal state
namespace Sketchy {
namespace Task {

    void Task::Init(std::vector<TaskPtr>& prereqs) {
        _state = Ready;
        if (prereqs.empty()) return;

        // Lock ourself in case any prereqs that we register
        // with below terminate before we're done initializing.
        lock_guard lock(_mutex);

        // One weak_ptr to pass to all prereqs; it will be copied
        // into their observer list anyway.
        TaskObserver::weak_ptr me = std::dynamic_pointer_cast<TaskObserver>(shared_from_this());

        // Iterate through the list of (potential) prereqs;
        // update appropriately based on the state of each prereq.
        for (TaskPtr& pre: prereqs) {
            lock_guard lock_prereq(pre->_mutex);
            switch(pre->_state) {
                case Done:
                    // Don't need to wait for the prereq to finish,
                    // because it is already done.
                    break;
                case Cancel:
                    // If any prereqs are cancelled or erroneous,
                    // then so are we.  Don't bother adding any
                    // additional prereqs.
                    _state = Cancel;
                    return;
                case Error:
                    // See comment for "Cancel", above.
                    _state = Error;
                    return;
                default:
                    // Prereq is not in a terminal state, so add it to the list
                    // of prereqs to wait for (and us to its list of observers).
                    _state = Wait;
                    _prereqs.insert(pre);
                    // We don't call addObserver() here, because it's cleaner
                    // to handle ourself any prereqs that are already in a
                    // terminal state (see the cases above).
                    pre->_observers.push_back(me);
            }
        }
        _prereqs_remaining = _prereqs.size();
    }


    void Task::addObserver(const TaskObserver::weak_ptr& observer) {
        {
            lock_guard lock(_mutex);
            switch(_state) {
                case Done:
                case Cancel:
                case Error:
                    // In a terminal state, so notify the observer
                    // (but outside of the critical section)
                    break;
                default:
                    _observers.push_back(observer);
                    return;
            }
        }

        // Immediately notify the observer that the task
        // is in a terminal state (similar to if the observer
        // had been registered before entering the state).
        auto strong = observer.lock();
        if (!strong) return;
        switch(_state) {
            case Done:
                strong->taskDone(shared_from_this());
                break;
            case Cancel:
                strong->taskCancel(shared_from_this());
                break;
            case Error:
                strong->taskError(shared_from_this());
                break;
            default:
                // Other cases were handled within critical section.
                break;
        }
    }


    void Task::work() {
        // The subclass must set this to true by calling
        // done(), yield(), or error().
        _endRun = false;

        // Critical section for state-transition.
        {
            lock_guard lock(_mutex);

            switch(_state) {
                case Ready:
                    // Do the actual work outside of the critical section.
                    _state = Run;
                    break;
                case Cancel:
                    // This task was cancelled while in the queue; we didn't bother removing it,
                    // but instead waited for it to reach the front so that we can ignore it here.
                    return;
                default:
                    // This should never happen.  A Task should never be enqueued if it is not
                    // Ready, and since all state transitions (except cancellation) occur within
                    // work(), something has gone terribly wrong here.
                    throw std::logic_error("attempt to run task that is not ready or cancelled");
            }
        }

        // If we made it here, we are now in the Run state.
        try {
            run();
        } catch (std::logic_error e) {
            // Error in our code... propagate up for someone else to handle.
            // TODO: should probably log here, though
            throw e;
        } catch(...) {
            // TODO: add descriptive argument
            //  error("uncaught error in Task.run()"); 
            error();
        }

        // Ensure that one of done(), yield(), or error() was called.
        if (!_endRun) throw std::logic_error("Task.run() must call done(), yield(), or error()");
    }


    // Subclasses must call precisely one of done(), yield(), or error()
    // during each invocation of run().
    void Task::yield() {
        if (_endRun) throw std::logic_error("yield(): _endRun was already set");
        _endRun = true;

        // Change state, unless already cancelled.
        {
            lock_guard lock(_mutex);
            if (_state == Cancel) return;
            else _state = Ready;
        }

        // Push task back onto queue.
        _queue->taskYielded(shared_from_this());
    }


    // Macro to reduce boilerplate in the methods below.
#define NOTIFY_OBSERVERS(METHOD_NAME)                                       \
    if (_observers.empty()) return;                                        \
    std::vector<TaskObserver::weak_ptr> observers(std::move(_observers));  \
    _observers.clear();                                                    \
    lock.unlock();                                                         \
    auto me = shared_from_this();                                          \
    for (auto weak: observers) {                                           \
        auto o = weak.lock();                                              \
        if (o.get()) o->METHOD_NAME(me);                                   \
    }

    // Subclasses must call precisely one of done(), yield(), or error()
    // during each invocation of run().
    void Task::done() {
        if (_endRun) throw std::logic_error("done(): _endRun was already set");
        _endRun = true;

        // Change state, unless already cancelled.
        unique_lock lock(_mutex);
        if (_state == Cancel) return;
        else _state = Done;

        NOTIFY_OBSERVERS(taskDone);
    }


    // Subclasses must call precisely one of done(), yield(), or error()
    // during each invocation of run().
    void Task::error() {
        if (_endRun) std::logic_error("error(): _endRun was already set");
        _endRun = true;

        // Change state, unless already cancelled.
        unique_lock lock(_mutex);
        if (_state == Cancel) return;
        else _state = Error;

        NOTIFY_OBSERVERS(taskError);
    }


    void Task::cancel() {
        unique_lock lock(_mutex);

        switch(_state) {
            case Done:
            case Cancel:
            case Error:
                // Already in a terminal state.
                return;
            case Wait:
            case Ready:
            case Run:
                _state = Cancel;
                break;
        }

        NOTIFY_OBSERVERS(taskCancel);
    }

#undef NOTIFY_OBSERVERS


    void Task::taskDone(const TaskPtr& task) {
        {
            lock_guard lock(_mutex);
            if (--_prereqs_remaining > 0) return;

            // No more prereqs left... perhaps it's time
            // to switch from Wait to Ready?
            switch(_state) {
                case Done:
                case Cancel:
                case Error:
                    // Already in a terminal state
                    return;
                case Wait:
                    _state = Ready;
                    if (_queue == nullptr) return;
                    break;
                default:
                    throw std::logic_error("prereqDone(): should be in Wait state");
                    return;
            }
        }
        // If we made it this far, we are now Ready and should enqueue ourself.
        _queue->add(shared_from_this());
    }


    void Task::taskCancel(const TaskPtr& task) {
        cancel();
    }


    void Task::taskError(const TaskPtr& task) {
        // TODO: or perhaps error()?
        cancel();
    }


} // namespace Task {
} // namespace Sketchy {