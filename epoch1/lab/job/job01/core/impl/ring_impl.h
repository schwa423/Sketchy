//
//  ring_impl.h
//  schwa::job01::core::impl
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//	Instead of struggling with the confusion of declaring template classes
//  as friends, instead use non-generic base classes.  End-users will never
//  need to touch these.

#ifndef __schwa__job01__core__ring__impl__
#define __schwa__job01__core__ring__impl__


// schwa::job01::core::impl ===================================================
namespace schwa { namespace job01 { namespace core { namespace impl {


// Base class of RingElement<>.
class BaseRingElement {
 public:
 	BaseRingElement() : _index(-1), _count(0) { }

 	int index() const { return _index; }
	int count() const { return _count; }

	// Answer the index of the ring element offset from the current
	// element by the specified amount (in either direction).
	int nextIndex() const {
		return (_index + 1) % _count;
	}
	int nextIndex(int steps) const {
		return (_index + steps) % _count;
	}
	int prevIndex() const {
		return prevIndex(1);
	}
	int prevIndex(int steps) const {
		int newIndex = (_index - steps) % _count;
		return (newIndex >= 0) 
			? newIndex
			: newIndex + _count;
	}

 private:
 	int _index;
 	int _count;

	friend class BaseRing; 	
};


// Base class of Ring<>.
class BaseRing {
 protected:
 	void setIndexAndCount(BaseRingElement& elem, int index, int count) {
 		elem._index = index;
		elem._count = count;
 	}
};


}}}}  // schwa::job01::core::impl =============================================
#endif  // #ifndef __schwa__job01__core__ring__impl__