//
// test_padding.cpp
// schwa::job01::test
//
//  Copyright (c) 2013 Schwaftwarez
//  License: Apache v2.0
//
//


#include "job01/core/padding.h"

using namespace schwa::job01::core;

#include <iostream>
using namespace std;

#include <assert.h>



template <int UnpaddedSize>
class BeforePadding {

private:
	unsigned char _data[UnpaddedSize];
};

template <int UnpaddedSize, int PaddedSize>
class AfterPadding 
: public Padded<BeforePadding<UnpaddedSize>, PaddedSize> {

};

template<int UnpaddedSize, int PaddedSize>
void testPadding() {
	BeforePadding<UnpaddedSize> unpadded;
	AfterPadding<UnpaddedSize, PaddedSize> padded;

	assert(sizeof(unpadded) == UnpaddedSize);
	assert(sizeof(padded) == PaddedSize);

}



int main(void) {

	testPadding<0,64>();
	testPadding<1,64>();
	testPadding<2,64>();
	testPadding<3,64>();
	testPadding<4,64>();
	testPadding<5,64>();
	testPadding<6,64>();
	testPadding<7,64>();
	testPadding<8,64>();
	testPadding<9,64>();
	testPadding<10,64>();
	testPadding<11,64>();
	testPadding<12, 64>();
	testPadding<13,64>();
	//  ... etc ...
	testPadding<60,64>();
	testPadding<61,64>();
	testPadding<62,64>();
	testPadding<63,64>();
	testPadding<64,64>();

	// This would be a compile error:
	// testPadding<65,64>();
	// ... but these aren't.
	testPadding<65,66>();
	testPadding<66,66>();

    cerr << "job01/core/test_padding...  PASSED!" << endl << endl;
}