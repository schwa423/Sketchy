//
//  Shadow_impl.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/16/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#include <iostream>
using std::cerr;
using std::endl;

#include "Shadow.h"

namespace Sketchy {
namespace Shadow {

	// Object //////////////////////////////////////////////////////////////////////////////////////

	// Static function that acts in place of a constructor.
	template<class Obj, class ShadowObj, class Loop>
	shared_ptr<Obj> // return value
	Object<Obj, ShadowObj, Loop>::New(shared_ptr<Loop> loop)
	{
		// Instantiate original and counterpart.
		shared_ptr<ShadowObj> shadow(new ShadowObj());
		shared_ptr<Obj> object(new Obj(loop, shadow));

		// Initialize the shadow.  See comment on Init() declaration.
		shared_ptr<Event> init(shadow->Init(loop, object));
		if (init) {
			loop->addTask(init);
		}

		return object;
	}

	// Destructor.
	template<class Obj, class ShadowObj, class Loop>
	Object<Obj, ShadowObj, Loop>::~Object()
	{
		shared_ptr<Event> release(new Release(m_shadow));

		// Release shadow to ensure that we don't continue hanging
		// onto it until after the event-loop has finished processing
		// the task.
		m_shadow.reset();

		m_loop->addTask(release);
	}

	// Constructor.  Private; only called by static New() function.
	template<class Obj, class ShadowObj, class Loop>
	Object<Obj, ShadowObj, Loop>::Object(shared_ptr<Loop> loop, shared_ptr<ShadowObj> shadow)
	: m_loop(loop), m_shadow(shadow)
	{

	}

	// Object::Release::reallyRun()... do a sanity check.
	template<class Obj, class ShadowObj, class Loop>
	void
	Object<Obj, ShadowObj, Loop>::Object::Release::reallyRun()
	{
		if (m_shadow.use_count() != 1) {
			cerr
				<< "WTF: "
				<< m_shadow->className() << " has "
				<< m_shadow.use_count()-1 << " extra references to it."
				<< endl;
		}
	}

} // namespace Shadow
} // namespace Sketchy

