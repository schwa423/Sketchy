//
//  blocks.cc
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#include "job01/mem/blocks.h"


// schwa::job01::mem ==========================================================
namespace schwa { namespace job01 { namespace mem {


// Define and initialize static variables.
uint8_t BlockArrayManager::_count = 0;
BlockArrayManager::ArrayInfo BlockArrayManager::_info[BlockArrayManager::kMaxArrays];



}}}  // schwa::job01::mem =====================================================