//
//  mem.h
//  Sketchy
//
//  Created by Josh Gargus on 12/30/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__mem__mem__
#define __schwa__mem__mem__


#include "stddef.h"


// namespace schwa::mem
namespace schwa {namespace mem {


// Answer the cache-line size for the current platform.
// May be more convenient to use CACHE_LINE_SIZE, but be
// aware that it is not a compile-time constant, but
// dynamically queried.
size_t cacheLineSize();
    

}}  // namespace schwa::mem


#define CACHE_LINE_SIZE schwa::mem::cacheLineSize()


#endif  // #ifndef __schwa__mem__mem__
