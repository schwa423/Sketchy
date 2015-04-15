//
//  ThreadOwner.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/21/11.
//  Copyright (c) 2011 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#ifndef Sketchy_ThreadOwner_h
#define Sketchy_ThreadOwner_h

class ThreadOwner {
public:
	// Answer true if this ThreadOwner owns the currently-executing thread.
	bool isCurrent();
	
	// Call this immediately in a newly-spawned thread that you own.
	// Don't be a dick by calling this if you don't actually own the thread.
	void setAsThreadOwner();
};

#endif
