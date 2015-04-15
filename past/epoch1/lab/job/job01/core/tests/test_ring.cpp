//
// test_ring.cpp
// schwa::job01::test
//
//  Copyright (c) 2013 Schwaftwarez
//  License: Apache v2.0
//
//

#include "job01/core/ring.h"
#include "job01/core/padding.h"
#include <iostream>

using namespace schwa::job01::core;


// Concrete RingElement<> subclass.
template <int PadToSize>
// TODO: can we avoid saying "PadToSize" twice?
class Foo : public RingElement<Foo<PadToSize>, PadToSize> {
 public:
 	Foo() {
 		const char* secretNumber = "forty-two";
 		for (int i = 0; i < 10; i++) {
 			theSecretNumber[i] = secretNumber[i];
 		}
 		theSecretNumber[10] = 0;
 	}

 	std::string whatIsTheSecretNumber() {
        std::string result("the secret number is: ");
        result += theSecretNumber;
 		return result;
 	}

 	int bar;
 	char baz;
 	char theSecretNumber[30];
};


// Test ring of specified size.
template <int RingSize>
void testFooRing() {
	// Parameters.
	const int PaddedSize = 64;
	// Typedefs.
	typedef Foo<PaddedSize> FooT;
	typedef Ring<FooT, RingSize> RingT;
	// Static assertions.
	static_assert(sizeof(RingT) == PaddedSize*RingSize, "unexpected ring size");

	RingT ring;
	FooT *foo, *next;

	// Element access outside range [0..RingSize-1] returns nullptr.
	foo = ring.elementAt(-1);
	assert(foo == nullptr);
	foo = ring.elementAt(RingSize);
	assert(foo == nullptr);

	// Repeat the following test for each ring-element,
	// incrementing the start-element each time.
	for (int i = 0; i < RingSize; i++) {

		// Element access within range [0..RingSize-1] returns pointer to element.
		foo = ring.elementAt(i);
		assert(foo != nullptr);

		// Since the ring has more than one element, an element's 
		// next/previous element is not the same as the first.
		assert(foo != foo->next());
		assert(foo != foo->prev());

		// Going forward or back a multiple of the ring-size
		// gives you the same element back.
		assert(foo == foo->next(3*RingSize));
		assert(foo == foo->prev(3*RingSize));

		// The same holds true when going forward one element at a time...
		next = foo;
		for (int j = 0; j < 3*RingSize; j++) {
			next = next->next();
		}
		assert(foo == next);

		// ... or going backward.
		next = foo;
		for (int j = 0; j < 3*RingSize; j++) {
			next = next->prev();
		}

		// If two iterators start in the same place and move around the ring
		// in opposite directions, they meet whenever the relative distance
		// they've travelled is some multiple of RingSize.
		for (int j = 1; j < RingSize; j++) {
			assert(foo->prev(j) == foo->next(RingSize-j));
		}
		assert(foo->next(2*RingSize + 5) == foo->next(5*RingSize + 5));
		assert(foo->prev(2*RingSize + 5) == foo->prev(5*RingSize + 5));
	}
}


// Test corner-case: ring with single element.
void testSingleElementFooRing() {
	// Parameters.
	const int PaddedSize = 64;
	const int RingSize = 1;
	// Typedefs.
	typedef Foo<PaddedSize> FooT;
	typedef Ring<FooT, RingSize> RingT;
	// Static assertions.
	static_assert(sizeof(RingT) == PaddedSize, "unexpected ring size");

	RingT ring;
	FooT *foo, *next;

	// Element access outside range [0..RingSize-1] returns nullptr.
	foo = ring.elementAt(-1);
	assert(foo == nullptr);
	foo = ring.elementAt(1);
	assert(foo == nullptr);

	// Element access within range [0..RingSize-1] returns pointer to element.
	foo = ring.elementAt(0);
	assert(foo != nullptr);

	// Since this is a 1-element ring, next() and prev()
	// always give back the same element.
	assert(foo == foo->next());
	assert(foo == foo->prev());
	assert(foo == foo->next(101));
	assert(foo == foo->prev(192));
}


// Weird test.  Ensure that we're not messing up pointers somehow
// by ensuring that we can access an element properly both via
// elementAt() and next()/prev().
void testThatDataIsNotMessedUp() {
	// Parameters.
	const int PaddedSize = 64;
	const int RingSize = 100;
	// Typedefs.
	typedef Foo<PaddedSize> FooT;
	typedef Ring<FooT, RingSize> RingT;

	RingT ring;
	FooT* foo;

	std::string expected("the secret number is: forty-two");

	// First access all the elements via elementAt().
	for (int i = 0; i < RingSize; i++) {
		foo = ring.elementAt(i);
		assert(foo->whatIsTheSecretNumber() == expected);
	}

	// Now, access all the elements via prev().
	FooT* current = foo->prev();
	while (current != foo) {
		assert(foo->whatIsTheSecretNumber() == expected);
		current = current->prev();
	}
}


int main(void) {
	// Test rings of various sizes.
	testSingleElementFooRing();
	testFooRing<2>();
	testFooRing<3>();
	testFooRing<4>();
	testFooRing<5>();
	testFooRing<6>();
	testFooRing<7>();	
	testFooRing<8>();
	testFooRing<9>();	
	testFooRing<10>();
	testFooRing<11>();
	testFooRing<22>();
	testFooRing<33>();
	testFooRing<44>();
	testFooRing<55>();
	testFooRing<66>();
	testFooRing<77>();
	testFooRing<88>();
	testFooRing<99>();

	// Misc tests.
	testThatDataIsNotMessedUp();

	std::cerr << "job01/core/test_ring...  PASSED!" << std::endl << std::endl;
}