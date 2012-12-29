//
//  WorkerTest.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 7/7/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#include <iostream>
using std::cerr;
using std::endl;

#include "gtest/gtest.h"

#include "tasks/Worker.h"
#include "tasks/Queue.h"

using namespace Sketchy::Task;

class TestTask : public Task {
 public:
    TestTask() : Task(), shouldYield(false) { }

    static TaskPtr New(std::vector<TaskPtr>& prereqs) {
        TaskPtr ptr = std::make_shared<TestTask>();
        dynamic_cast<TestTask*>(ptr.get())->Init(prereqs);
        return ptr;
    }

    static TaskPtr New(int i) {
        TaskPtr ptr = std::make_shared<TestTask>();
        std::dynamic_pointer_cast<TestTask>(ptr)->Init(i);
        return ptr;
    }

    virtual ~TestTask() { }

    virtual void run() {
        if (shouldYield) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            shouldYield = false;
            yield();
        } else {
            done();
        }
    }

    int index;

 protected:
    void Init(std::vector<TaskPtr>& prereqs) {
        index = -1;
        int count = prereqs.size();
        Task::Init(prereqs);
    }
    void Init(int i) {
        index = i;
    }

    bool shouldYield;
};

TEST(WorkerTest, SimpleCreation) {
    std::shared_ptr<Queue> queue(new Queue());

    Worker w(queue, 1);

    std::vector<TaskPtr> prereqs;
    for (int i = 0; i < 10000; i++) {
        prereqs.push_back(TestTask::New(i));
        queue->add(prereqs[i]);
    }

    TaskPtr final = TestTask::New(prereqs);

    queue->add(final);

    while(!final->isSettled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}


