//
//  Shadow.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/16/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#ifndef Sketchy_Shadow_h
#define Sketchy_Shadow_h

#include <memory>
using std::shared_ptr;

#include "Event.h"

namespace Sketchy {
namespace Shadow {

	// Object which is directly accessible to the application programmer,
	// and which has a 1-1 correspondence with a "shadow" object that isn't
	// accessible to the application programmer.
	template<class Obj, class ShadowObj, class Loop>
	class Object {
	public:
		// Constructor-like.
		static shared_ptr<Obj> New(shared_ptr<Loop> loop);
		~Object();
		friend ShadowObj;
	protected:
		shared_ptr<ShadowObj> m_shadow;
		shared_ptr<Loop> m_loop;
		// Constructor. Don't use directly... see Object::New()
		Object(shared_ptr<Loop> loop, shared_ptr<ShadowObj> shadow);

		// Event that does nothing except maintain a shared_ptr
		// to our shadow, only to release it within the event-loop.
		class Release : public Event {
		public:
			Release(shared_ptr<ShadowObj> shadow) : m_shadow(shadow) { }
			void reallyRun();
		protected:
			shared_ptr<ShadowObj> m_shadow;
		};
	}; // class Object

	// Shadow object which isn't directly accessible to application programmer.
	template<class Obj, class ShadowObj, class Loop>
	class ShadowObject {
	public:
		friend Obj;
	protected:
		shared_ptr<Loop> m_loop;
		// Constructor.  Don't use directly... see Object::New().
		// TODO: what should the constructor look like?
		//ShadowObject(shared_ptr<Loop> loop);
		ShadowObject() {};
		// Give the shadow an opportunity to initialize itself.
		// This mechanism is flexible... if the shadow can safely
		// initialize itself immediately, it simply does so and
		// returns NULL.  However, if further initialization must
		// be done within the event-loop, it returns an event that
		// we schedule here.
		virtual Event* Init(shared_ptr<Loop> loop, shared_ptr<Obj> obj) = 0;
	}; // class ShadowObject

} // namespace Shadow
} // namespace Sketchy

#include "Shadow_impl.h"

#endif
