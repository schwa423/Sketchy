//
// test_boss.cpp
// schwa::job01::test
//
//  Copyright (c) 2013 Schwaftwarez
//  License: Apache v2.0
//
//


#include "job01/boss.h"

using namespace schwa::job01;

#include <type_traits>
#include <iostream>
using namespace std;

#include <assert.h>


int main(void) {
	Boss<4> boss;

//	assert(alignment_of<Boss<4>>::value == host::CACHE_LINE_SIZE);
	cerr << "ALIGNMENT IS: " << alignment_of<Boss<4>>::value << endl;
	cerr << "ALIGNMENT OF BossImpl: " << alignment_of<impl::BossImpl>::value << endl;

    cerr << "job01/test_boss...  PASSED!" << endl << endl;
}