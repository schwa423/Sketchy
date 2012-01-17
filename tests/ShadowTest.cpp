//
//  ShadowTest.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 1/16/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include <memory>
using std::shared_ptr;
using std::weak_ptr;

#include "gtest/gtest.h"

#include "Loop.h"
#include "Event.h"
#include "Sleep.h"
#include "Fence.h"
#include "Shadow.h"

using std::this_thread::sleep_for;
using std::chrono::milliseconds;

typedef shared_ptr<Event> EventPtr;
typedef Loop<EventPtr> EventLoop;

class TestObject;
class TestShadow;

using namespace Sketchy;
using namespace Sketchy::Shadow;

typedef Object<TestObject, TestShadow, EventLoop> TestObjectParent;
class TestObject : public TestObjectParent
{
public:
	const char* className() { return "TestObject"; }
	shared_ptr<TestShadow> shadow() { return m_shadow; }
protected:
	friend TestObjectParent;
	TestObject(shared_ptr<EventLoop> l, shared_ptr<TestShadow> s) : TestObjectParent(l,s) { }
};

typedef ShadowObject<TestObject, TestShadow, EventLoop> TestShadowParent;
class TestShadow : public TestShadowParent, public std::enable_shared_from_this<TestShadow>
{
public:
	const char* className() { return "TestShadow"; }
	bool wasInitialized() { return m_wasInitialized; }
protected:
	class InitEvent : public Event {
	public:
		InitEvent(shared_ptr<TestShadow> shadow) : m_shadow(shadow) { };
		virtual ~InitEvent() { cerr << "TestShadow::InitEvent was destroyed" << endl; }
		virtual void reallyRun() { cerr << "TestShadow::InitEvent::reallyRun()" << endl; m_shadow->handleInit(); }
	protected:
		shared_ptr<TestShadow> m_shadow;
	};
	void handleInit() { m_wasInitialized = true; }
	bool m_wasInitialized = false;

	virtual Event* Init(shared_ptr<EventLoop> loop, shared_ptr<TestObject> obj) {
		return new InitEvent(shared_from_this());
	};

	friend TestObjectParent;
};

TEST(ShadowTest, InitAndDeleteInLoop) {
	shared_ptr<EventLoop> loop(new EventLoop);

	// Before we create the object/shadow pair,
	// schedule a Sleep task that will take a
	// few milliseconds to complete.  We do
	// this to verify that shadow initialization
	// does indeed occur in the event-loop...
	// see below.
	shared_ptr<Sleep> sleep1(new Sleep(5));
	loop->addTask(sleep1);
	sleep_for(milliseconds(5));

	// Create object/shadow pair.  For sanity,
	// verify that the shadow is available.
	shared_ptr<TestObject> obj;
	obj = TestObject::New(loop);
	weak_ptr<TestObject> weak = obj;
	weak_ptr<TestShadow> shadow = obj->shadow();

	// All weak-pointers should still be good,
	// and the shadow should not yet have been
	// initialized (the 5ms slumber continues...)
	EXPECT_EQ(weak.expired(), false);
	{
		shared_ptr<TestShadow> strong = shadow.lock();
		EXPECT_GE(strong.use_count(), 1);
		EXPECT_EQ(strong->wasInitialized(), false);
	}

	// Enqueue a fence and wait for it to finish...
	// by this time, the shadow should be fully
	// initialized, since its InitEvent was enqueued
	// before the fence.
	shared_ptr<Fence> fence1(new Fence());
	loop->addTask(fence1);
	fence1->get_future().wait();
	{
		shared_ptr<TestShadow> strong = shadow.lock();
		EXPECT_GE(strong.use_count(), 1);
		EXPECT_EQ(strong->wasInitialized(), true);
	}

	// Release pointer to object... shadow shouldn't
	// be destroyed immediately, because the event-loop
	// is waiting for the Sleep event to finish.
	shared_ptr<Sleep> sleep2(new Sleep(15));
	loop->addTask(sleep2);
	obj.reset();
	sleep_for(milliseconds(5));
	EXPECT_EQ(weak.expired(), true);
	EXPECT_EQ(shadow.expired(), false);

	// Enqueue a fence and wait for it to finish...
	// by this time, the shadow should have been
	// released within the event loop.
	shared_ptr<Fence> fence2(new Fence());
	loop->addTask(fence2);
	fence2->get_future().wait();
	EXPECT_EQ(shadow.expired(), true);
}
