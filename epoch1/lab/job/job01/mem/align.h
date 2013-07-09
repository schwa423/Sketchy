//
//    align.h
//    schwa::job01::mem
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Macros and utilities related to memory-alignment.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__mem__align__
#define __schwa__job01__mem__align__


#include "job01/host/host.h"


// schwa::job01::mem ==========================================================
namespace schwa { namespace job01 { namespace mem {


// TODO: decide whether to use these, or the ones in schwa::mem00::align.h
#ifndef DISABLE_ALIGNMENT
#define e__align64      alignas(64)
#define e__cache_align  alignas(host::CACHE_LINE_SIZE)
#define class__cache_align  class e__cache_align
#define align_64        alignas(64)
#define align_cacheline alignas(host::CACHE_LINE_SIZE)
#else
#define e__align64
#define e__cache_align
#define class__cache_align  class
#define align_64
#define align_cacheline
#endif


}}}  // schwa::job01::mem =====================================================

#endif  // #ifndef __schwa__job01__mem__align__
