//
//  LoopTest.cpp
//  Sketchy Unit Tests
//
//  Created by Joshua Gargus on 12/20/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#include "gtest/gtest.h"
#include "Loop.h"

class SimpleTask {
public:
	SimpleTask(Loop<SimpleTask>* loop) :
		m_loop(loop),
		m_wasRun(false),
		m_wasAborted(false),
		m_wasCurrent(false)
	{ }
	void run() { m_wasRun = true; m_wasCurrent = m_loop->isCurrent(); }
	void abort() { m_wasAborted = true; m_wasCurrent = m_loop->isCurrent(); }
	bool wasRunOrAborted() { return m_wasRun || m_wasAborted; }
	bool wasCurrent() { return m_wasCurrent; }
private:
	bool m_wasAborted;
	bool m_wasRun;
	bool m_wasCurrent;
	Loop<SimpleTask>* m_loop;
};

TEST(LoopTest, SimpleCreation) {
	auto loop = new Loop<SimpleTask>();
	
	SimpleTask task(loop);
	loop->addTask(&task);
	delete loop;
		
	EXPECT_EQ(task.wasRunOrAborted(), true);
	EXPECT_EQ(task.wasCurrent(), true);
}