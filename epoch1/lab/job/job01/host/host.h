//
//  host.h
//  schwa::job01::host
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//

// TODO: instead of always disabling, instead only disable for CMake builds
#if 1
#error Use the version generated by CMake from host.h.in
#else

#ifndef __schwa__job01__host__host__
#define __schwa__job01__host__host__


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace host {


const int CACHE_LINE_SIZE = 64;
const int NUM_CPUS = 4;

// TODO: configure this in CMake (probably CACHE_LINE_SIZE too... but
//       NUM_CPUs should be determined at run-time)
#define SCHWA_64_BIT true
#define SCHWA_32_BIT false

#error
}}}  // schwa::job01::host ====================================================
#endif  // #ifndef __schwa__job01__host__host__

#endif
