//
//  WorkerTest.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 7/7/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include "gtest/gtest.h"

#include "tasks/Worker.h"
#include "tasks/Queue.h"


namespace Sketchy {
namespace Task {
	
	
} // namespace Task
} // namespace Sketchy

using namespace Sketchy::Task;

TEST(WorkerTest, SimpleCreation) {
    Worker w(3);
}


