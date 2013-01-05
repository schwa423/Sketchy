//
//  mem.cpp
//  Sketchy
//
//  Created by Josh Gargus on 12/30/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "mem.h"
#include "schwassert.h"


///////////////////////////////////////////////////////////////////////////////////////////
// Adapted from Nick Strupat's blog post of December 17, 2012
// (http://nickstrupat.blogspot.ca/2012/12/i-wrote-this-function-for-cache-line.html )
// He states "do whatever you want with it"... thanks Nick!

static size_t initializeCacheLineSize();

#if defined(__APPLE__) && defined(TARGET_OS_MAC)  // OS X

#include <sys/sysctl.h>
size_t initializeCacheLineSize() {
    size_t line_size;
    size_t sizeof_line_size = sizeof(line_size);
    sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
    return line_size;
}


#elif defined(__APPLE__)  // iOS

// TODO: how can we get this value on iOS?
size_t initializeCacheLineSize() {
    return 64;
}


#elif defined(_WIN32)

#include <stdlib.h>
#include <windows.h>
size_t initializeCacheLineSize() {
    size_t line_size = 0;
    DWORD buffer_size = 0;
    DWORD i = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;
    
    GetLogicalProcessorInformation(0, &buffer_size);
    buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
    GetLogicalProcessorInformation(&buffer[0], &buffer_size);
    
    for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
            line_size = buffer[i].Cache.LineSize;
            break;
        }
    }
    
    free(buffer);
    return line_size;
}


#elif defined(linux)

// TODO: Commenter on StackOverflow suggested using this instead:
#error Look into using sysconf(_SC_LEVEL1_DCACHE_LINESIZE)

#include <stdio.h>
size_t initializeCacheLineSize() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int i = 0;
    if (p) {
        fscanf(p, "%d", &i);
        fclose(p);
    }
    return i;
}

#else
#error Unrecognized platform
#endif

// End of Nick Strupat's code.
///////////////////////////////////////////////////////////////////////////////////////////


// namespace schwa::mem
namespace schwa {namespace mem {

static size_t cachedCacheLineSize = 0;

size_t cacheLineSize() {
    if (!cachedCacheLineSize) {
        cachedCacheLineSize = initializeCacheLineSize();
        SCHWASSERT(cachedCacheLineSize > 0, "could not obtain cache-line size");
    }
    return cachedCacheLineSize;
}


}}  // namespace schwa::mem