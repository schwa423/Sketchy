//
//  util.h
//  schwa::job01::mem
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//

#ifndef __schwa__job01__mem__util__
#define __schwa__job01__mem__util__

#include <cstdint>


// schwa::job01::mem ==========================================================
namespace schwa { namespace job01 { namespace mem {


template <typename T1, typename T2>
inline ptrdiff_t PointerDifference(const T1* p1, const T2* p2) {

	return reinterpret_cast<const uint8_t*>(p2) -
	       reinterpret_cast<const uint8_t*>(p1);
}


}}}  // schwa::job01::mem =====================================================

#endif  // #ifndef __schwa__job01__mem__util__