//
//    test_meta.cc
//    schwa::job01::core
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Test the functionality of the utilities in core/meta.h
//
//////////////////////////////////////////////////////////////////////////////


#include "job01/core/meta.h"

#include <assert.h>
#include <iostream>
using std::cerr;
using std::endl;

    
// One of these is enabled for types between 1 and 64 bytes,
// and the other is enabled for types between 65 and 128 bytes.
template <typename ARG_TYPE>
int sizeUpperBound(ARG_TYPE& arg, 
                   SCHWA_ONLY_IF_SIZE_BETWEEN(ARG_TYPE, 0, 64)) {
    return 64;
}
template <typename ARG_TYPE>
int sizeUpperBound(ARG_TYPE& arg, 
                   SCHWA_ONLY_IF_SIZE_BETWEEN(ARG_TYPE, 64, 120)) {  
    return 128;
}
// A 60-byte type.
class Sixty {
    char bytes[60];
};
// A 120-byte type.
class OneTwenty {
    char bytes[120];
};
// Test that the correct version of sizeUpperBound() is selected,
// based on the size of the argument.
void testSizeBetween() {
    Sixty s1;
    OneTwenty s2;

    assert(64 == sizeUpperBound(s1));
    assert(128 == sizeUpperBound(s2));

}


// Two disjoint class hierarchies.
class A { };
class B : public A { 
 public:
    B(int v) : val(v) { }    
    const int val;
};
class C { };
class D : public C {
 public:
    D(int v) : val(v) { }
    const int val;
};
// Instantiate instance with value of 11, but only if it is a subclass of A.
template <typename ARG_TYPE>
ARG_TYPE instantiate(SCHWA_USE_IF_SUBCLASS(ARG_TYPE, A)) {
    return ARG_TYPE(11);
}
// Instantiate instance with value of 22, but only if it is a subclass of C.
template <typename ARG_TYPE>
ARG_TYPE instantiate(SCHWA_USE_IF_SUBCLASS(ARG_TYPE, C)) {
    return ARG_TYPE(22);
}
// Test that correct version of instantiate() is selected,
// based on the base-class of the instantiated type.
void testUseIfSubclass() {
    auto b = instantiate<B>();
    auto d = instantiate<D>();

    assert(11 == b.val);
    assert(22 == d.val);
}


int main(void) {
    testSizeBetween();
    testUseIfSubclass();

    cerr << "job01/core/test_meta...  PASSED!" << endl << endl;
}