//
//  ring.h
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//
//  Notes:
//  - efficiently compute address of neighbors at any number of steps in either direction.
//  - potentially add/remove links from the ring (not implemented), 
//    but this is not thread-safe.
//     - how can we test/enforce this in debug builds via template
//		 metaprogramming?  Put lock around every "normal use" operation, and a
//		 special type of lock around the "add/remove link" APIs... a try-lock
//		 check can raise an error if a "normal use" operation has the ring
//		 locked.  Optimized builds will make both of these locks no-ops
//		 (default no-op type).
//  - PROBLEM! Related to ability the following two conflicting capabilities:
//     - add/remove links on-the-fly
//     - memory alignment of Ring::EntryT
//     - note that in order for a Ring::Entry to compute the address of its
//       n-neighbors, it must know:
//        - the address of the index-0 entry
//        - the index of the desired neighbor
//     - a naive implementation might give Ring::Element two extra fields,
//       to store its index in... then sizeof() and modular math would
//       give the answer.  But...
//     - But what if the entries were already specially fit to eg: cache-line
//       size?  Then adding a field would fuck shit up.
//     - So, better if we use template metaprogramming to use a separate class 
//       for each ring entry to add/remove links...
//		 might make that capability infeasible.
//


#ifndef __schwa__job01__core__ring__
#define __schwa__job01__core__ring__


#include <type_traits>
#include <assert.h>

#include "job01/core/padding.h"
#include "job01/core/impl/ring_impl.h"


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {


// Base class for all application-specific ring elements (see test_ring.cpp).
template <class ElemT, int PadToSize>
class RingElement : public impl::BaseRingElement {
 public:
 	static const int PaddedSize = PadToSize;

 	// Return pointer to some other element in the ring.
 	ElemT* next() {
 		return elementAt(nextIndex());
 	}
 	ElemT* next(int steps) {
 		return elementAt(nextIndex(steps));
 	}
 	ElemT* prev() {
 		return elementAt(prevIndex());
 	}
 	ElemT* prev(int steps) {
 		return elementAt(prevIndex(steps));
 	}

 private:
 	// Return the element at the specified offset from this element.
 	// Assumes that the specified offset will result in a valid pointer;
 	// it is the caller's responsibilty to ensure this.
 	// TODO: would be nice to not reinterpret_cast()... perhaps make
 	//       PaddedElemT a proper subclass, and static_cast() to it?
 	ElemT* elementAt(int elemIndex) {
 		int byteDiff = (elemIndex - index()) * PaddedSize;
 		char* ptr = &(reinterpret_cast<char*>(this)[byteDiff]);
 		return reinterpret_cast<ElemT*>(ptr);
 	}
};


// Generic ring class, parameterized by the concrete element-type,
// and by the number of elements in the ring.
template <class ElemT, int RingSize>
class Ring : public impl::BaseRing {
 public:
	Ring() {
		for (int i = 0; i < RingSize; i++) {
			setIndexAndCount(_elements[i], i, RingSize);
		}
	}

 	ElemT* elementAt(int index) {
 		return (index >= 0 && index < RingSize)
 			? &(_elements[index])
 			: nullptr;
 	}

 private:
 	typedef Padded<ElemT, ElemT::PaddedSize> PaddedElemT;

 	PaddedElemT _elements[RingSize];
};


}}}  // schwa::job01::core ====================================================
#endif  // #ifndef __schwa__job01__core__ring__