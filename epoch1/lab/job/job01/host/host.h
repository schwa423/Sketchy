//
//  host.h
//  schwa::job01::host
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


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


}}}  // schwa::job01::host ====================================================
#endif  // #ifndef __schwa__job01__host__host__
