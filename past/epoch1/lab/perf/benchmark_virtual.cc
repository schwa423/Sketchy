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
//    - There is essentially no difference in performance between invoking
//      virtual functions vs. invoking a function-pointer held in a struct.
//    - There is also no significant difference when there are long runs of
//      the same class, vs. a randomish order.  This was a bit surprising.
//
//    - Timings:
//      Alignment   virtual/fptr      virtual/fptr
//                   (cycling)        (consecutive)
//      8:             22/17              21/17
//      16:            23/21              21/21
//      64:            22/21              21/21
//      128:           22/21              21/21
//      256:           23/21              23/22
//      512:           23/21              23/22
//
//    Conclusion:
//    - I'm happy to conclude that virtual functions are more than fast enough
//      for our purposes!!
//    - I'm glad I didn't prematurely optimize by using function pointers...
//      I was tempted to follow the lead of Molecular Musings, but this will
//      be cleaner.
//
///////////////////////////////////////////////////////////////////////////////


const int NUM_OBJECTS = 10000;
const int ALIGNMENT = 256;

#include <iostream>
using namespace std;

#include <sys/mman.h>

#include "job01/dbg/dbg.h"
using namespace schwa::job01;
using dbg::ScopeTimer;

typedef int (*FooT)(int input);
struct alignas(ALIGNMENT) FooHolder {
	FooT func;
};

class alignas(ALIGNMENT) Base {
 public:
 	virtual ~Base() { }
	virtual int foo(int input) {}
	virtual void stash(FooHolder& ptr) {}
};

#define CLASS(NAME, VAL)                                       \
class NAME : public Base {                                     \
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
int compare(Base* objects, FooHolder* functions) {
	int val = 0;

	{	// Virtual functions.
		ScopeTimer t([](time::msecs elapsed) {
			cerr << "virtual function calls took: " << elapsed << endl;
		});
		for (int loop = 0; loop < 1000; ++loop) {
			for (int i = 0; i < NUM_OBJECTS; ++i) {
				val = objects[i].foo(val);
			}
		}
	}

	{	// Function pointers.
		ScopeTimer t([](time::msecs elapsed) {
			cerr << "function pointer calls took: " << elapsed << endl;
		});
		for (int loop = 0; loop < 1000; ++loop) {
			for (int i = 0; i < NUM_OBJECTS; ++i) {
				val = functions[i].func(val);
			}
		}
	}

	// Should be zero.
	return val;
}


int main(void) {
	Base      objects[NUM_OBJECTS];
	FooHolder functions[NUM_OBJECTS];

	// Cycle through all subclasses before creating
	// the next instance of the first subclass.
	cerr << "CYCLING THROUGH DIFFERENT CLASSES..." << endl;
  // Use placement-new to instantiate the objects, to avoid pointer-chasing
  // which would give the FooHolders an unfair advantage.
	for (int i = 0; i < NUM_OBJECTS; i += 10) {
		new(&(objects[i])) P1;
		new(&(objects[i+1])) P2;
		new(&(objects[i+2])) P3;
		new(&(objects[i+3])) P4;
		new(&(objects[i+4])) P5;
		new(&(objects[i+5])) M1;
		new(&(objects[i+6])) M2;
		new(&(objects[i+7])) M3;
		new(&(objects[i+8])) M4;
		new(&(objects[i+9])) M5;
	}
	// Stash the corresponding static functions.
	for (int i = 0; i < NUM_OBJECTS; ++i) {
		objects[i].stash(functions[i]);
	}

	// Compare timings.
	int value = compare(objects, functions);
	cerr << "SHOULD BE ZERO: " << value << endl;

	// Allocate a large number of instances of each subclass
	// before moving on to the next subclass.
	cerr << endl << "LONG RUNS OF CLASSES..." << endl;
	int j = 0;
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) P1; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) M1; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) P2; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) M2; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) P3; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) M3; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) P4; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) M4; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) P5; }
	for (int i = 0; i < NUM_OBJECTS / 10; ++i, ++j) { new(objects+j) M5; }

	// Stash the corresponding static functions.
	for (int i = 0; i < NUM_OBJECTS; ++i) {
		objects[i].stash(functions[i]);
	}

	// Compare timings.
	value = compare(objects, functions);
	cerr << "SHOULD BE ZERO: " << value << endl;

	// Clean up after ourselves.
	for (int i = 0; i < NUM_OBJECTS; ++i) {
//		delete objects[i];
	}

	return 0;
}
