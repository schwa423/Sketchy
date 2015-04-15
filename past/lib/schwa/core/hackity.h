//
//  hackity.h
//  schwa
//
//  Created by Josh Gargus on 12/27/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//  Miscellaneous helpful stuff that doesn't have a better place to live right now.
//  Lives in top-level schwa:: namespace for convenience.
//
//


#ifndef __schwa__util__hackity__
#define __schwa__util__hackity__


#include <stdint.h>
#include <functional>


namespace schwa {

namespace core {
    typedef std::function<void ()> Thunk;
}


uint32_t timeToRun(core::Thunk&& thunk);
uint32_t timeToRun(core::Thunk& thunk);


}  // namespace schwa


#endif  // #ifndef __schwa__util__hackity__