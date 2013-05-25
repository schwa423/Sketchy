//
//    benchmark_virtual.cc
//    schwa::perf
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Experiment with costs of virtual function calls vs. function-pointers.
//    The methodology is:
//    - create 10 subclasses of a base class, each with a different foo()
//    - give each subclass a static method sfoo() with the same code as foo()
//    - create an array consisting of (various orderings) of instances 
//      of these subclasses
//    - create a corresponding array of function pointers by telling each
//      instance to use stash() to save a pointer to their version of sfoo()
//    - time how long it takes to:
//      - iterate several times through the objects, calling foo() on each
//      - iterate several times through the function pointers, invoking each
//
//	  Results:
//    - Discovered that memory bandwidth is a limiting factor.  The objects
//      originally used alignas(64) to mimic the layout of the job system's
//      pools.  This put them at an unfair disadvantage compared to function-
//      pointers.  We fix this by putting the pointers into FooHolder structs,
//      and giving that the same alignment as the objects.
//
//    - Some difference was made by varying the order of the subclasses used
//      to instantiate the objects, more for virtual functions than function
//      pointers.  Specifically, we tried the following:
//      - cycling through each subclass in turn... every 10 consecutive objects
//        includes one instance of each subclass
//      - allocating 10000 instances of one type, then 10000 of the next, etc.
//      
//    - Timings:
//      Alignment   virtual/fptr      virtual/fptr
//                   (cycling)        (consecutive)
//      4:             20/21              21/21
//		16:            21/18              21/18
//      64:            37/30              35/28
//      128:           71/57              68/57
//
//      As you can see, with small objects there is no difference between
//      function pointers and virtual methods.  I conjecture this is because
//      the lower memory requirements mean that the virtual function table
//      can remain in cache (a bit of a guess, I admit).  On the other end,
//      by the time you get up to 64 and 128 bit objects, there is a clear
//      win for function-pointers.
//
//      Ordering of instances makes a small difference for virtual functions.
//      This is good news for us, because we can't rely on adjacent jobs having
//      any predictable ordering.
//
//    Conclusion:
//    - I'm happy to conclude that virtual functions are more than fast enough
//      for our purposes!!
//    - I'm glad I didn't prematurely optimize by using function pointers...
//      I was tempted to follow the lead of Molecular Musings, but this will
//      be cleaner.
//
///////////////////////////////////////////////////////////////////////////////


const int NUM_OBJECTS = 100000;
const int ALIGNMENT = 64;

#include <iostream>
using namespace std;

#include "job01/dbg/dbg.h"
using namespace schwa::job01;
using dbg::ScopeTimer;

typedef int (*FooT)(int input);
struct alignas(ALIGNMENT) FooHolder {
	FooT func;
};

class Base {
 public:
 	virtual ~Base() { }
	virtual int foo(int input) = 0;
	virtual void stash(FooHolder& ptr) = 0;
};

#define CLASS(NAME, VAL)                                       \
class alignas(ALIGNMENT) NAME : public Base {                  \
 public:                                                       \
 	static int sfoo(int input) { return input + VAL; }         \
 	virtual int foo(int input) { return input + VAL; }         \
	virtual void stash(FooHolder& hold) { hold.func = sfoo; }  \
};


// Declare some classes with different implementations of foo() / sfoo().
CLASS(P1, 1);
CLASS(P2, 2);
CLASS(P3, 3);
CLASS(P4, 4);
CLASS(P5, 5);
CLASS(M1, -1);
CLASS(M2, -2);
CLASS(M3, -3);
CLASS(M4, -4);
CLASS(M5, -5);


// Loop through the arrays, timing how long it takes
// to invoke virtual functions vs. function-pointers.
int compare(Base** objects, FooHolder* functions) {
	int val = 0;

	{	// Virtual functions.
		ScopeTimer t([](time::msecs elapsed) {
			cerr << "virtual function calls took: " << elapsed << endl;
		});
		for (int loop = 0; loop < 100; ++loop) {
			for (int i = 0; i < NUM_OBJECTS; ++i) {
				val = objects[i]->foo(val);
			}
		}
	}

	{	// Function pointers.
		ScopeTimer t([](time::msecs elapsed) {
			cerr << "function pointer calls took: " << elapsed << endl;
		});
		for (int loop = 0; loop < 100; ++loop) {
			for (int i = 0; i < NUM_OBJECTS; ++i) {
				val = functions[i].func(val);
			}
		}
	}

	// Should be zero.
	return val;
}


int main(void) {
	Base*     objects[NUM_OBJECTS];
	FooHolder functions[NUM_OBJECTS];

	// Cycle through all subclasses before creating
	// the next instance of the first subclass.
	cerr << "CYCLING THROUGH DIFFERENT CLASSES..." << endl;
	for (int i = 0; i < NUM_OBJECTS; i += 10) {
		objects[i] = new P1;
		objects[i+1] = new P2;
		objects[i+2] = new P3;
		objects[i+3] = new P4;
		objects[i+4] = new P5;
		objects[i+5] = new M1;
		objects[i+6] = new M2;
		objects[i+7] = new M3;
		objects[i+8] = new M4;
		objects[i+9] = new M5;
	}
	// Stash the corresponding static functions.
	for (int i = 0; i < NUM_OBJECTS; ++i) {
		objects[i]->stash(functions[i]);
	}

	// Compare timings.
	int value = compare(objects, functions);
	cerr << "SHOULD BE ZERO: " << value << endl;

	// Prepare for next test by deleting all objects.
	for (int i = 0; i < NUM_OBJECTS; ++i) {
		delete objects[i];
	}

	// Allocate a large number of instances of each subclass
	// before moving on to the next subclass.
	cerr << endl << "LONG RUNS OF CLASSES..." << endl;
	int j = 0;
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new P1; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new M1; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new P2; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new M2; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new P3; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new M3; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new P4; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new M4; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new P5; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { objects[j] = new M5; }

	// Stash the corresponding static functions.		
	for (int i = 0; i < NUM_OBJECTS; ++i) {
		objects[i]->stash(functions[i]);
	}

	// Compare timings.
	value = compare(objects, functions);
	cerr << "SHOULD BE ZERO: " << value << endl;

	// Clean up after ourselves.
	for (int i = 0; i < NUM_OBJECTS; ++i) {
		delete objects[i];
	}

	return 0;
}