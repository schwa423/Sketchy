//
//  Fence.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/25/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Fence_h
#define Sketchy_Fence_h

#include "Event.h"
#include <thread>
using std::promise;
using std::shared_future;

class Fence : public Event
{
public:
	Fence() {
	
	}
	
	// This is not synchronized, so call it to obtain the
	// future before scheduling this fence on an event-loop.
	std::shared_future<bool> get_future() {
		return m_promise.get_future().share(); 
	}
	
protected:
	// Subclasses should override to do something useful.
	virtual void reallyRun () {
		m_promise.set_value(true);
	}
	
	promise<bool> m_promise;
	
};

#endif
