//
//  Task_QueueTest.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 7/14/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "tasks/Queue.h"
#include "tasks/Worker.h"

using namespace Sketchy::Task;

class ValueTask : public Task {
 public:
    ValueTask(int v, std::vector<int>& r) : _value(v), _results(r) { }

    virtual void run() {
        _results.push_back(_value);
        done();
    }

    // TODO: these should have defaults implemented in Task, dealing with prereq
    // fulfillment and so-forth.
    virtual void taskDone(const TaskPtr& task) { }
    virtual void taskCancel(const TaskPtr& task) {  }
    virtual void taskError(const TaskPtr& task) { }

 private:
    int _value;
    std::vector<int>& _results;
};


TEST(Task_QueueTest, SimpleRoundRobin) {
    Worker w(0);

    auto q1 = std::make_shared<Queue>();
    auto q2 = std::make_shared<Queue>();
    auto q3 = std::make_shared<Queue>();

    // TODO: revisit when Apple's Clang supports initializer lists.
    // std::vector<GeneratorPtr> queues = {q1, q2, q3};
    // ... or maybe even pass initializer list to RoundRobinQueue constructor
    std::vector<QueueOutPtr> queues;
    queues.push_back(q1);
    queues.push_back(q2);
    queues.push_back(q3);
    auto rr = std::make_shared<RoundRobinQueue>(queues);

    std::vector<int> results;
    TaskPtr t0(new ValueTask(0, results));
    TaskPtr t1(new ValueTask(1, results));
    TaskPtr t2(new ValueTask(2, results));
    TaskPtr t3(new ValueTask(3, results));
    TaskPtr t4(new ValueTask(4, results));
    TaskPtr t5(new ValueTask(5, results));
    TaskPtr t6(new ValueTask(6, results));
    TaskPtr t7(new ValueTask(7, results));
    TaskPtr t8(new ValueTask(8, results));
    TaskPtr t9(new ValueTask(9, results));

    q1->add(t0);
    q2->add(t1);
    q3->add(t2);    
    q1->add(t3);    
    q2->add(t4);
    q3->add(t5);    
    q1->add(t6);
    q2->add(t7);
    q1->add(t8);    
    q1->add(t9);

    for (int i = 0; i <= 9; i++) {
        w.runTask(rr->next());
        EXPECT_EQ(results.size(), i+1);
        EXPECT_EQ(results[i], i);
    }
}
