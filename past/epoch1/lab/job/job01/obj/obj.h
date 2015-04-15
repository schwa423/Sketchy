//
//    obj.h
//    schwa::job01::obj
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Defines a trio of classes which work closely together:
//      - Obj
//      - ObjRef
//      - ObjMaker
//
//    More details about these classes can be found in obj_defs.h
//
//    The purpose of this header file is to allow compile-time specification of
//    multiple, disjoint object systems... in other words, the type-system
//    statically prevents objects from one system to leak into another.
//    
//    This isn't possible quite yet (see TODO below), but here's an example
//    scenario describing how it would work.
//    - The programmer decides to create an in-process debugger which can
//      control the application's job-system.  This functionality will live
//      in schwa::job::debugger.
//    - To support this, a clone of this object-system (schwa::job01::obj) is
//      desired... it will live in schwa::job01::debugger::obj.
//    - To make this happen, a "debugger_obj.h" header is created which is very
//      similar to this one, except instead of wrapping "obj_defs.h" in 
//      namespace schwa::job01::obj, it is instead wrapped in the namespace
//      schwa::job01::debugger::obj.
//
//    TODO:
//    -  cannot yet wrap the contents of obj.cc in different namespaces
//       
///////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__obj__obj_______________________________________________
#define __schwa__job01__obj__obj_______________________________________________


#include "job01/obj/obj_includes.h"

namespace schwa { namespace job01 { namespace obj {

#include "job01/obj/obj_defs.h"
    
}}} 


#endif  // __schwa__job01__obj__obj____________________________________________
