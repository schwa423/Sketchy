//
//  WorkerTest.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 7/7/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
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
    static TaskPtr New(std::vector<TaskPtr>&& prereqs) {
        TaskPtr ptr = std::make_shared<TestTask>();
        // TODO:  aaaaargh!  It hurts my eyes!
        std::dynamic_pointer_cast<TestTask>(ptr)->Init(std::forward<std::vector<TaskPtr>>(prereqs));
        return ptr;
    }

    static TaskPtr New(int i) {
        TaskPtr ptr = std::make_shared<TestTask>();
        std::dynamic_pointer_cast<TestTask>(ptr)->Init(i);
        return ptr;
    }


    virtual void run() {
        done();
    }

    // TODO: these should have defaults implemented in Task, dealing with prereq
    // fulfillment and so-forth.
    virtual void taskDone(const TaskPtr& task) { }
    virtual void taskCancel(const TaskPtr& task) {  }
    virtual void taskError(const TaskPtr& task) { }

    int index;

 protected:
    void Init(std::vector<TaskPtr>&& prereqs) {
        Task::Init(std::forward<std::vector<TaskPtr>>(std::forward<std::vector<TaskPtr>>(prereqs)));
        index = -1;
    }
    void Init(int i) {
        index = i;
    }
};

TEST(WorkerTest, SimpleCreation) {
    std::shared_ptr<Queue> queue(new Queue());
    Worker w(queue, 3);

    std::vector<TaskPtr> prereqs;
    for (int i = 0; i < 1000; i++) {
        prereqs.push_back(TaskPtr(TestTask::New(i)));
        queue->add(prereqs[i]);
    }
    cerr << "FINISHED ENQUEUING PREREQ TASKS" << endl;
    TaskPtr final = TestTask::New(std::move(prereqs));
    cerr << "CREATED FINAL TASK" << endl;
    queue->add(final);
    cerr << "FINISHED ENQUEUING FINAL TASK" << endl;

    while(!final->isSettled()) {

    }
    cerr << "DONE" << endl;
}


