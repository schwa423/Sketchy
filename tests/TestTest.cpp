//
//  TestTest.cpp
//  Sketchy Unit Tests
//
//  Created by Joshua Gargus on 12/18/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#include "gtest/gtest.h"
#include <thread>

std::mutex mut;

typedef std::unique_lock<std::mutex> unique_lock;

void callFromThread(const char* threadName);
void callFromThread(const char* threadName) {
	unique_lock(mut);
    std::cout << "Hello, World! from " << threadName << std::endl;
}


TEST(ExampleTest1, CanSucceed) {
	std::thread t1(callFromThread, "THREAD 1");
	std::thread t2(callFromThread, "THREAD 2");
	std::thread t3(callFromThread, "THREAD 3");
	
	t1.join();
	t2.join();
	t3.join();
	
	EXPECT_EQ(1, 1);
}

TEST(ExampleTest1, CanFail) {
	EXPECT_EQ(32, 1);
}

TEST(ExampleTest2, AlsoPasses) {
	EXPECT_EQ(1, 1);
}