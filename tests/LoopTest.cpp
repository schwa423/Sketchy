//
//  LoopTest.cpp
//  Sketchy Unit Tests
//
//  Created by Joshua Gargus on 12/20/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#include "gtest/gtest.h"

#include "Loop.h"
#include "Event.h"
#include "Fence.h"
#include "Sleep.h"

using std::chrono::milliseconds;
using std::this_thread::sleep_for;

// Forward-declarations.
class SimpleTask;
typedef std::shared_ptr<SimpleTask> SimpleTaskPtr;

typedef std::shared_ptr<Event> EventPtr;
typedef std::shared_ptr<Fence> FencePtr;
typedef std::shared_ptr<Sleep> SleepPtr;

typedef Loop<EventPtr> EventLoop;

// Simple task for testing purposes.
class SimpleTask : public Event
{
public:
	SimpleTask(EventLoop* loop) :
		m_loop(loop),
		m_wasRun(false),
		m_wasAborted(false),
		m_wasCurrent(false)
	{ }

	bool wasRun() { return m_wasRun; }
	bool wasAborted() { return m_wasAborted; }
	bool wasRunOrAborted() { return m_wasRun || m_wasAborted; }
	bool wasCurrent() { return m_wasCurrent; }

	virtual void abort() {
		m_wasAborted = true;
		m_wasCurrent = m_loop->isCurrent();
		Event::abort();		
	}
	
protected:
	virtual void reallyRun() {
		m_wasRun = true;
		m_wasCurrent = m_loop->isCurrent();
	}

private:
	
	bool m_wasAborted;
	bool m_wasRun;
	bool m_wasCurrent;
	EventLoop* m_loop;
};


TEST(LoopTest, SimpleCreation) {
	auto loop1 = new EventLoop();
	auto loop2 = new EventLoop();
	
	SimpleTaskPtr task1(new SimpleTask(loop1));
	SimpleTaskPtr task2(new SimpleTask(loop1));
	SimpleTaskPtr task3(new SimpleTask(loop1));
	SimpleTaskPtr task4(new SimpleTask(loop1));
	SimpleTaskPtr task5(new SimpleTask(loop1));
	
	loop1->addTask(task1);
	loop1->addTask(task2);
	loop1->addTask(task3);
	
	sleep_for(milliseconds(5));
	
	EXPECT_EQ(task1->wasRunOrAborted(), true);
	EXPECT_EQ(task2->wasRunOrAborted(), true);
	EXPECT_EQ(task3->wasRunOrAborted(), true);
	EXPECT_EQ(task4->wasRunOrAborted(), false);
	EXPECT_EQ(task1->wasCurrent(), true);
	EXPECT_EQ(task2->wasCurrent(), true);
	EXPECT_EQ(task3->wasCurrent(), true);
	EXPECT_EQ(task4->wasCurrent(), false);
	
	delete loop1;
	delete loop2;
}

TEST(LoopTest, SleepTakesTime) {
	auto loop = new EventLoop();
	steady_clock clock;
	steady_clock::time_point start, finish;
	SleepPtr sleep(new Sleep(15));
	SimpleTaskPtr simple1(new SimpleTask(loop));
	SimpleTaskPtr simple2(new SimpleTask(loop));
	FencePtr fence(new Fence());
	
	start = clock.now();
	loop->addTask(simple1);
	loop->addTask(sleep);
	loop->addTask(simple2);
	loop->addTask(fence);
	
	sleep_for(milliseconds(5));
	EXPECT_EQ(simple1->wasRun(), true);
	EXPECT_EQ(simple2->wasRun(), false);
	fence->get_future().wait_for(milliseconds(500));
	finish = clock.now();
	EXPECT_EQ(simple2->wasRun(), true);	
	EXPECT_LT(finish-start, milliseconds(30));	
	EXPECT_GE(finish-start, milliseconds(15));	
	
	delete loop;
}