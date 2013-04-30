//
//  align.h
//  schwa::mem00
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#ifndef __schwa__mem00__align__
#define __schwa__mem00__align__


const int CACHE_LINE_SIZE = 64;


#ifndef DISABLE_ALIGNMENT
#define ALIGNED(x) alignas(x)
#define CACHE_ALIGNED alignas(CACHE_LINE_SIZE)
#else
#define ALIGNED(x)
#define CACHE_ALIGNED
#endif


#endif  // #ifndef __schwa__mem00__align__