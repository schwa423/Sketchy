//
//    boss2.cc
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/boss2.h"

// TODO: remove this
#include <iostream>
using std::cerr;
using std::endl;

// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

// static variable
ThreadLocal<Worker2> Boss2::current_worker_;


}}  // schwa::job01 ===========================================================
