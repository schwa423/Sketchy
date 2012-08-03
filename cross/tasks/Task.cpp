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

// TODO: things to test:
//  - error if run doesn't call done(), yield(), or error()
//  - observer called properly if added when task is:
//    - not in terminal state yet
//    - already in terminal state
namespace Sketchy {
namespace Task {

    // Task::Prereqs /////////////////////////////////////////////////////

    Task::Prereqs::~Prereqs() {
        stopWatching();
    };

    // Task //////////////////////////////////////////////////////////////

    void Task::Init(std::vector<TaskPtr>& prereqs) {
        lock_guard lock(_mutex);
        _state = Wait;
        _prereqs_remaining = prereqs.size();
        for (TaskPtr& pre: prereqs) {
            _prereqs.watch(pre);
        }
    }


    void Task::addWatcher(TaskObserver* watcher) {
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
                    _watchers.insert(watcher);
                    return;
            }
        }

        // Immediately notify the observer that the task
        // is in a terminal state (similar to if the observer
        // had been registered before entering the state).
        switch(_state) {
            case Done:
                watcher->taskDone(shared_from_this());
                break;
            case Cancel:
                watcher->taskCancel(shared_from_this());
                break;
            case Error:
                watcher->taskError(shared_from_this());
                break;
            default:
                // Other cases were handled within critical section.
                break;
        }
    }


    void Task::removeWatcher(TaskObserver* watcher) {
        _watchers.erase(watcher);
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
            // Error in our code... propagate up for someone else to handle (probably log/barf).
            throw e;
        } catch(...) {
            // TODO: add descriptive argument
            //  error("uncaught error in Task.run()"); 
            error();
        }

        // Ensure that one of done(), yield(), or error() was called.
        if (!_endRun) throw std::logic_error("Task.run() must call done(), yield(), or error()");
    }


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
#define NOTIFY_WATCHERS(METHOD_NAME)             \
    cerr << "notifying " << _watchers.size() << " WATCHERS" << endl; \
    if (_watchers.empty()) return;               \
    std::set<TaskObserver*> watchers(_watchers); \
    _watchers.clear();                           \
    auto me = shared_from_this();                \
    for (auto watcher: watchers) {               \
        watcher->METHOD_NAME(me);                \
    }


    void Task::done() {
        if (_endRun) throw std::logic_error("done(): _endRun was already set");
        _endRun = true;

        // Change state, unless already cancelled.
        {
            lock_guard lock(_mutex);
            if (_state == Cancel) return;
            else _state = Done;
        }

        NOTIFY_WATCHERS(taskDone);
    }


    void Task::error() {
        if (_endRun) std::logic_error("error(): _endRun was already set");
        _endRun = true;

        // Change state, unless already cancelled.
        {
            lock_guard lock(_mutex);
            if (_state == Cancel) return;
            else _state = Error;
        }

        NOTIFY_WATCHERS(taskError);
    }


    void Task::cancel() {
        lock_guard lock(_mutex);

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

        NOTIFY_WATCHERS(taskCancel);
    }


    void Task::prereqDone(const TaskPtr& task) {
        {
            lock_guard lock(_mutex);
            cerr << "PREREQS REMAINING: " << _prereqs_remaining << endl;
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


    void Task::prereqCancel(const TaskPtr& task) {
        cancel();
    }


    void Task::prereqError(const TaskPtr& task) {
        // TODO: maybe error?
        cancel();
    }

#undef NOTIFY_OBSERVERS

} // namespace Task {
} // namespace Sketchy {