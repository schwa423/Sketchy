//
//  Task.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <stdexcept>

#include "Task.h"
#include "Queue.h"


// TODO: things to test:
//  - error if run doesn't call done(), yield(), or error()
//  - observer called properly if added when task is:
//    - not in terminal state yet
//    - already in terminal state
namespace Sketchy {
namespace Task {

    typedef std::lock_guard<std::mutex> lock_guard;

    Task::Task(std::vector<TaskPtr>& prereqs) : m_state(Wait), m_prereqs(prereqs) {
        for (auto pre: m_prereqs) pre->addObserver(shared_from_this());
    }
    

    Task::Task(std::vector<TaskPtr>&& prereqs) 
        : m_state(Wait), m_prereqs(std::forward<std::vector<TaskPtr>>(prereqs)) {
        for (auto pre: m_prereqs) pre->addObserver(shared_from_this());
    }


    void Task::addObserver(std::shared_ptr<TaskObserver>&& observer) {
        {
            lock_guard lock(m_mutex);
            switch(m_state) {
                case Done:
                case Cancel:
                case Error:
                    // In a terminal state, so notify the observer
                    // (but outside of the critical section)
                    break;
                default:
                    m_observers.push_back(observer);
                    return;
            }
        }

        // Immediately notify the observer that the task
        // is in a terminal state (similar to if the observer
        // had been registered before entering the state).
        switch(m_state) {
            case Done:
                observer->taskDone(shared_from_this());
                break;
            case Cancel:
                observer->taskCancel(shared_from_this());
                break;
            case Error:
                observer->taskError(shared_from_this());
                break;
            default:
                // Other cases were handled within critical section.
                break;
        }
    }


    void Task::work() {
        // The subclass must set this to true by calling
        // done(), yield(), or error().
        m_endRun = false;

        // Critical section for state-transition.
        {
            lock_guard lock(m_mutex);

            switch(m_state) {
                case Ready:
                    // Do the actual work outside of the critical section.
                    m_state = Run;
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
        if (!m_endRun) throw std::logic_error("Task.run() must call done(), yield(), or error()");
    }


    void Task::yield() {
        if (m_endRun) throw std::logic_error("yield(): m_endRun was already set");
        m_endRun = true;

        // Change state, unless already cancelled.
        {
            lock_guard lock(m_mutex);
            if (m_state == Cancel) return;
            else m_state = Ready;
        }

        // Push task back onto queue.
        m_queue->taskYielded(shared_from_this());
    }

    // Macro to reduce boilerplate in the methods below.
#define NOTIFY_OBSERVERS(METHOD_NAME)             \
    if (!m_observers.empty()) {                   \
        auto me = shared_from_this();             \
        for (auto weak: m_observers) {            \
            auto strong = weak.lock();            \
            if (strong) strong->METHOD_NAME(me);  \
        }                                         \
        m_observers.clear();                      \
    }                                             

    void Task::done() {
        if (m_endRun) throw std::logic_error("done(): m_endRun was already set");
        m_endRun = true;

        // Change state, unless already cancelled.
        {
            lock_guard lock(m_mutex);
            if (m_state == Cancel) return;
            else m_state = Done;
        }

        NOTIFY_OBSERVERS(taskDone);
    }


    void Task::error() {
        if (m_endRun) std::logic_error("error(): m_endRun was already set");
        m_endRun = true;

        // Change state, unless already cancelled.
        {
            lock_guard lock(m_mutex);
            if (m_state == Cancel) return;
            else m_state = Error;
        }

        NOTIFY_OBSERVERS(taskError);
    }


    void Task::cancel() {
        lock_guard lock(m_mutex);

        switch(m_state) {
            case Done:
            case Cancel:
            case Error:
                // Already in a terminal state.
                return;
            case Wait:
            case Ready:
            case Run:
                m_state = Cancel;
                break;
        }

        NOTIFY_OBSERVERS(taskCancel);        
    }

#undef NOTIFY_OBSERVERS

} // namespace Task {
} // namespace Sketchy {