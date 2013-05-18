//
//  pool.cpp
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#include "job01/core/pool.h"


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {


// Define and initialize static variables.
uint8_t Pools::_num_pools = 0;
Pools::PoolInfo Pools::_pools[Pools::kMaxPools];



}}}  // schwa::job01::core ====================================================
