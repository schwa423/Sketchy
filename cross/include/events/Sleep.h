//
//  Sleep.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/25/11.
//  Copyright (c) 2011 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#ifndef Sketchy_Sleep_h
#define Sketchy_Sleep_h

#include "Event.h"
#include <thread>
#include <chrono>

class Sleep : public Event
{
public:
	Sleep(long msecs) : m_milliseconds(msecs) { }
	
protected:
	virtual void reallyRun() {
		std::this_thread::sleep_for(m_milliseconds);
	}
	
	std::chrono::milliseconds m_milliseconds;
};

#endif
