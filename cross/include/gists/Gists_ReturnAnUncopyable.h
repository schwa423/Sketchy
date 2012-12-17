//
//  Gists_ReturnAnUncopyable.h
//  schwa::gist
//
//  Created by Josh Gargus on 12/14/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//  The purpose of this gist is to demonstrate how we can
//  trigger a behavior in the destructor of an object, and
//  force it to happen only once by disallowing copying of
//  the object.  However, rvalue-assignment is allowed, via
//  a move-constructor, which enables us to ensure that the
//  desired behavior still happens only once (by changing
//  the state of the original object so that its destructor
//  knows not to invoke that behavior.
//
//  Usage:
//  Uncomment one of the defines:
//    __FOO_MOVE_CONSTRUCTOR__
//    __FOO_RETURN_VALUE_OPTIMIZATION_1__
//    __FOO_RETURN_VALUE_OPTIMIZATION_2__
//    __FOO_RETURN_VALUE_OPTIMIZATION_3__
//
//  (note that the compiler may decided to use the move-constructor in
//   some cases where it could potentially use RVO, such as Clang 1.0
//   for the case __FOO_RETURN_VALUE_OPTIMIZATION_1__)
//
//  Uncomment the define:
//    __FOO_FORCE_ERROR__
//  ... to see that the disallowed cases cause compiler errors, as intended
//


#include <utility>
#include <iostream>
using std::cerr;
using std::endl;


// PICK ONE OF THESE /////////////////////////////////////
//#define __FOO_MOVE_CONSTRUCTOR__
//#define __FOO_RETURN_VALUE_OPTIMIZATION_1__
#define __FOO_RETURN_VALUE_OPTIMIZATION_2__
//#define __FOO_RETURN_VALUE_OPTIMIZATION_3__

// ENABLE THIS OR DON'T //////////////////////////////////
//#define __FOO_FORCE_ERROR__


// namespace schwa::gist
namespace schwa {namespace gist {

class Foo {
 public:
    Foo(Foo&& other)
        : _movedCount(other._movedCount), _unmovedCount(other._unmovedCount), _wasMoved(false)  {
        cerr << "called move constructor" << endl;
        other._wasMoved = true;
    }

    ~Foo() {
        if (_wasMoved)
            ++ _movedCount;
        else
            ++ _unmovedCount;

        cerr << "called destructor: "
             << _wasMoved << "  " << _movedCount << "  " << _unmovedCount << endl;
    }

 private:
    int& _movedCount;
    int& _unmovedCount;

    bool _wasMoved;

    // Not strictly necessary... if we don't declare a copy-constructor, it
    // would be implicitly deleted by the definition of the move-constructor.
    Foo(const Foo& other);

    // Only allow fooFactory() to instantiate Foos.
    Foo(int& movedCount, int& unmovedCount)
    : _movedCount(movedCount), _unmovedCount(unmovedCount), _wasMoved(false) {
        cerr << "called regular constructor" << endl;
    }

    friend Foo fooFactory(int&, int&, bool);
};



Foo fooFactory(int& movedCount, int& unmovedCount, bool useFirst);
Foo fooFactory(int& movedCount, int& unmovedCount, bool useFirst) {
#if defined(__FOO_MOVE_CONSTRUCTOR__)
    cerr << "fooFactory() forcing use of move constructor" << endl;

    Foo foo(movedCount, unmovedCount);
    Foo foo2(movedCount, unmovedCount);

    if (useFirst)
        return foo;
    else
        return foo2;

#elif defined(__FOO_RETURN_VALUE_OPTIMIZATION_1__)
    cerr << "fooFactory() allowing use of RVO... case 1" << endl;

    if (useFirst) {
        Foo foo(movedCount, unmovedCount);
        return foo;
    else
        return Foo(movedCount, unmovedCount);

#elif defined(__FOO_RETURN_VALUE_OPTIMIZATION_2__)
    cerr << "fooFactory() allowing use of RVO... case 2" << endl;

    Foo foo(movedCount, unmovedCount);
    return foo;
#elif defined(__FOO_RETURN_VALUE_OPTIMIZATION_3__)
    cerr << "fooFactory() allowing use of RVO... case 3" << endl;

    return Foo(movedCount, unmovedCount);
#else
#error This cannot be... make sure exactly one is defined.
#endif
}


void fooFactoryCaller();
void fooFactoryCaller() {
    int movedCount = 0;
    int unmovedCount = 0;

    Foo foo = fooFactory(movedCount, unmovedCount, true);
    cerr << "called fooFactory() once" << endl;
    Foo foo2 = fooFactory(movedCount, unmovedCount, false);
    cerr << "called fooFactory() twice" << endl;


#if defined(__FOO_FORCE_ERROR__)
    // Error... copy constructor is explicitly private.
    Foo foo3(foo);

    // Error... assignment would like to use copy constructor,
    // but it is private;
    Foo foo4 = foo2;
#endif

    cerr << "moved: " << movedCount << "   unmoved: " << unmovedCount << endl;
    cerr << "OH YEAH!" << endl;
}


}}  // namespace schwa::gist


#undef __FOO_MOVE_CONSTRUCTOR__
#undef __FOO_RETURN_VALUE_OPTIMIZATION_1__
#undef __FOO_RETURN_VALUE_OPTIMIZATION_2__
#undef __FOO_RETURN_VALUE_OPTIMIZATION_3__
#undef __FOO_FORCE_ERROR__
