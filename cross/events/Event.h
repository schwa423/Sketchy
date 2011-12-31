//
//  Event.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/25/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Event_h
#define Sketchy_Event_h

#include <chrono>
using std::chrono::steady_clock;
typedef steady_clock::time_point time_point;

#include <memory>

class Event
{
public:
	enum state { NEW=0, RUNNING, FINISHED, ABORTED };
	typedef std::shared_ptr<Event> ptr;
	
	Event() {
		m_creation = s_clock.now();
		m_state = NEW;
	}
	
	void run() {
		m_start = s_clock.now();
		reallyRun();
		m_finish = s_clock.now();
		m_state = FINISHED;
		
	}
	virtual void abort() {
		m_finish = s_clock.now();
		m_state = ABORTED;
	}
	
protected:
	// Subclasses should override to do something useful.
	virtual void reallyRun () { }
	
	state m_state;
	
	static steady_clock s_clock;
	time_point m_creation;	// time that Event is instantiated
	time_point m_start;		// time that Event starts running
	time_point m_finish;    // time that Event finishes
};

#endif
