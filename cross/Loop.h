//
//  Loop.h
//  Sketchy
//
//  Created by Joshua Gargus on 11/28/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//
//  TODO:
//  - Who destroys Task?  Is Task* appropriate to put in queue?

#ifndef Sketchy_Loop_h
#define Sketchy_Loop_h

#include <deque>
#include <thread>
#include <mutex>

#include "ThreadOwner.h"

template <typename TaskPtr>
class Loop : public ThreadOwner {
public:
	Loop() : m_stopped(false) {
		m_thread = std::thread(&Loop::runLoop, this);
	}
	~Loop() { 
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			m_stopped = true; 
			m_condition.notify_one();
		}
		if (!isCurrent()) { m_thread.join(); }
	};
	
	void addTask(TaskPtr task) { 
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push_back(task);
		m_condition.notify_one();
	}
	
private:
	void runLoop() {
		setAsThreadOwner();
		
		// Loop until we're stopped.
		while (true) {
			std::unique_lock<std::mutex> lk(m_mutex);
			if (m_stopped) {
				// Abort each pending task, and quit looping.
				std::for_each(m_queue.begin(), m_queue.end(), abortTask);
				return;
			}
			if (!m_queue.empty()) {
				TaskPtr tsk = m_queue.front();
				m_queue.pop_front();
				lk.unlock();
				tsk->run();
			}
			else m_condition.wait(lk);
		}
	}

	// Used above in runLoop()... wish I was smart enough to use
	// a lambda, or bind the member function, or something.
	static void abortTask(TaskPtr task) { task->abort(); }
	
	bool m_stopped;
	std::deque<TaskPtr> m_queue;
	std::condition_variable m_condition;
	std::mutex m_mutex;
	std::thread m_thread;	
};

#endif
