//
//  hackity.cpp
//  schwa
//
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
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