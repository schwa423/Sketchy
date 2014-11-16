//
//  base_test.cc
//  om_testrunner
//
//  Created by Josh Gargus on 10/11/14.
//  Copyright (c) 2014 schwaftwarez. All rights reserved.
//

#include <gtest/gtest.h>
#include "om/base/libom_base.h"

TEST(Base, DummyTest1) {
    EXPECT_EQ(1, 1);
    EXPECT_NE(2, 1);
    
    om::Foo foo;
    foo.Touch();
    foo.Touch();
    EXPECT_EQ(2, foo.GetTouchCount());
    foo.Touch();
    EXPECT_EQ(3, foo.GetTouchCount());
    
}

