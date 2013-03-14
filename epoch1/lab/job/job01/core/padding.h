//
//  padding.h
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  License: Apache v2.0
//
//  Generic wrappers to pad wrapped classes to the desired size.
//


#ifndef __schwa__job01__core__padding__
#define __schwa__job01__core__padding__


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


// TODO: use constant, like CACHE_LINE_SIZE or something, instead of 64.
template <class WrappedT>
class CacheLine : public Padded<WrappedT, 64> { };


}}}  // schwa::job01::core ====================================================
#endif  // #ifndef __schwa__job01__core__padding__