//
//  mem_FreeListTest.cpp
//  schwa::mem::test
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "gtest/gtest.h"
#include "free_list.h"


// namespace schwa::mem
namespace schwa {namespace mem {


TEST(mem_FreeListTest, SimpleCreation) {
    struct Element : public mem::Link {
        float foo;
        float bar;
        const char* baz = "HI, I'M BAZ!!";
    };

    const uint32_t MAX_ELEMENTS = 100;

    Element elements[MAX_ELEMENTS];
    mem::FreeList list(&elements, MAX_ELEMENTS, sizeof(Element));

    EXPECT_EQ(list.freeCount(), MAX_ELEMENTS);

    std::vector<Element*> unfree;

    for (int i=0; i < 60; ++i) {
        unfree.push_back(reinterpret_cast<Element*>(list.obtain()));
        EXPECT_EQ(unfree.back(), &(elements[i]));
    }

    EXPECT_EQ(list.freeCount(), MAX_ELEMENTS - 60);

    for (int i=0; i < 20; ++i) {
        list.release(unfree.back());
        unfree.pop_back();
    }

    EXPECT_EQ(list.freeCount(), MAX_ELEMENTS - 40);

    for (int i=0; i < MAX_ELEMENTS - 40; ++i)
        unfree.push_back(reinterpret_cast<Element*>(list.obtain()));

    EXPECT_EQ(list.freeCount(), 0);

    EXPECT_EQ(list.obtain(), nullptr);

    for (int i=0; i < MAX_ELEMENTS; ++i)
        EXPECT_NE(unfree[i], nullptr);
}


}}  // namespace schwa::mem