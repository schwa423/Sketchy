//
//  schwassert.h
//  schwa::job01::core
//
//  Created by Josh Gargus on 12/15/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//  Assertion macros.
//
//  USE_SCHWASSERT
//    Define this before including "schwassert.h" to force assertions to be used,
//    even in release builds.
//
//  DISABLE_SCHWASSERT
//    Define this before including "schwassert.h" to disable use of assertions,
//    even in debug builds.
//
//  SCHWASSERT(COND, MSG)
//    Assert that the condition evaluates to true, otherwise print the message and break.
//
//  SCHWASSERT_CODE(CODE)
//    Allow insertion of arbitrary code, only if assertions are enabled.  For example,
//    add instance variables to track and verify invariants that should be implicitly
//    maintained in release builds.
//
//  SCHWASSERT_BREAK()
//    Currently just calls assert(false).  Want to use platform-specific hook to
//    break in debugger, if connected to one.
//
//  SCHWASSERT_LOCKED(MUTEX, MSG)
//    Assert that the mutex is currently locked by the active thread.
//
//  *** TODO ***
//      - implement SCHWASSERT_LOCKED (it's not clear how)
//


#ifndef __schwa__job01__core__schwassert__
#define __schwa__job01__core__schwassert__


// By default, assertions are enabled when debugging, and disabled
// in release builds.  However, they can also be:
// - enabled in release builds  (by explicitly pre-defining USE_SCHWASSERT)
// - disabled in debug builds   (by defining DISABLE_SCHWASSERT)
#if defined(DEBUG) && !defined(DISABLE_SCHWASSERT) && !defined(USE_SCHWASSERT)
#define USE_SCHWASSERT true
#endif


// If assertions are disabled, the macros compile to nothing.
// Otherwise, provide suitable implementations for the macros.
#if !defined(USE_SCHWASSERT)
    // Assertions are disabled.
    #define SCHWASSERT_BREAK()
    #define SCHWASSERT(COND, MSG)
    #define SCHWASSERT_CODE(CODE)
    #define SCHWASSERT_LOCKED(MUTEX, MSG)
#else
    // Assertions are enabled.

// TODO: for now, assertions only log to cerr.  Later we can make them fancier.
#include <iostream>

// We use standard assert() so that we can break in the debugger.
#if !defined(NDEBUG)
#include <assert.h>
#else
// Since we want to allow assertions even in release mode,
// we first undefine NDEBUG, then redefine it afterward.
#define SCHWASSERT_NDEBUG NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG SCHWASSERT_NDEBUG
#undef SCHWASSERT_NDEBUG
#endif

// Don't crash release builds on assertion failures, just log.
// Eventually, we'll use platform-specific calls to break in
// the debugger (if we're connected to one).
#if defined(DEBUG)
#define SCHWASSERT_BREAK() assert(false)
#else
#define SCHWASSERT_BREAK()
#endif

// Allow conditional inclusion of arbitrary code to facilitate assertions,
// for example, to add additional state to a class to verify implicit invariants.
#define SCHWASSERT_CODE(CODE) CODE

// Assert that the mutex is already locked by the current thread
// *** TODO ***   unimplemented... how to implement this?
#define SCHWASSERT_LOCKED(MUTEX, MSG)

// Test if the condition is true.  If not, log it, and possibly trigger breakpoint.
#define SCHWASSERT(COND, MSG)                                         \
    if (!(COND)) {                                                    \
        std::cerr << "SCHWASSERT("#COND"):  " << (MSG) << std::endl;  \
        SCHWASSERT_BREAK();                                           \
    }
#endif  // #if !defined(USE_SCHWASSERT) #else


#endif  // #ifndef __schwa__core__schwassert__
