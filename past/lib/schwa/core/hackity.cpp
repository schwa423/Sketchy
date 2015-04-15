//
//  hackity.cpp
//  schwa
//
//  Created by Josh Gargus on 12/27/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "hackity.h"

#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::duration_cast;


namespace schwa {


uint32_t timeToRun(core::Thunk&& thunk) {
    return timeToRun(thunk);
}

uint32_t timeToRun(core::Thunk& thunk) {
    auto start = high_resolution_clock::now();
    thunk();
    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<microseconds>(end - start);
    return elapsed.count();
}


}  // namespace schwa