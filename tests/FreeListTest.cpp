//
//  FreeListTest.cpp
//  Sketchy
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "gtest/gtest.h"
#include "free_list.h"


namespace schwa {


TEST(FreeListTest, SimpleCreation) {
    struct Element : private mem::FreeList::Link {
        float foo;
        float bar;
        const char* baz = "HI, I'M BAZ!!";
    };

    Element elements[100];
    mem::FreeList list(&elements, 100, sizeof(Element));

    EXPECT_EQ(list.freeCount(), 100);

    std::vector<Element*> unfree;

    for (int i=0; i < 60; ++i) {
        unfree.push_back(reinterpret_cast<Element*>(list.obtain()));
        EXPECT_EQ(unfree.back(), &(elements[i]));
    }

    EXPECT_EQ(list.freeCount(), 40);

    for (int i=0; i < 20; ++i) {
        list.release(unfree.back());
        unfree.pop_back();
    }

    EXPECT_EQ(list.freeCount(), 60);

    for (int i=0; i < 60; ++i)
        unfree.push_back(reinterpret_cast<Element*>(list.obtain()));

    EXPECT_EQ(list.freeCount(), 0);

    EXPECT_EQ(list.obtain(), nullptr);

    for (int i=0; i < 100; ++i)
        EXPECT_NE(unfree[i], nullptr);
}


}  // namespace schwa