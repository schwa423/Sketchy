//
//    blocks_impl.cc
//    schwa::job01::mem::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////


#include "job01/mem/blocks.h"


// schwa::job01::mem::impl ====================================================
namespace schwa { namespace job01 { namespace mem { namespace impl {


// Define and initialize static variables.
uint8_t BlockArrayManager::_count = 0;
BlockArrayManager::ArrayInfo BlockArrayManager::_info [BlockArrayManager::kMaxArrays];


}}}}  // schwa::job01::mem::impl ==============================================