//
//  padding.h
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  License: Apache v2.0
//
//  Generic wrappers to pad wrapped classes to the desired size.
//
//  *** DEPRECATED? ***
//      - maybe better to use std C++ functionality like alignas()
//  *** TODO ***
//      - move to schwa::job01::mem
//


#ifndef __schwa__job01__core__padding__
#define __schwa__job01__core__padding__


#include "job01/host/host.h"


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {


// No-op wrapper, to be used when no padding is desired.
template <class WrappedT>
class Unpadded : public WrappedT {

};


// Pad wrapped class to the desired size.
// Causes compilation error if wrapped class already exceeds the desired size.
template <class WrappedT, int PadToSize>
class Padded : public WrappedT {
	static_assert(sizeof(WrappedT) <= PadToSize, 
		          "WrappedT already exceeds desired padded size");

 private:
 	char _padding[PadToSize - sizeof(WrappedT)];
};



// *** EXPERIMENTAL / DEPRECATED? ***   not used anywhere
template <class WrappedT, int AlreadyUsed = 0>
class CacheLine : public Padded<WrappedT, schwa::job01::host::CACHE_LINE_SIZE - AlreadyUsed> { 
 public:
	CacheLine() {
//		assert(this % host::CACHE_LINE_SIZE == 0);
	}
};


}}}  // schwa::job01::core ====================================================
#endif  // #ifndef __schwa__job01__core__padding__