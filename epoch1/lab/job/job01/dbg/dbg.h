//
//    dbg.h
//    schwa::job01::dbg
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Utilities for debugging.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__dbg__dbg__
#define __schwa__job01__dbg__dbg__

#include "job01/time/time_util.h"


// schwa::job01::dbg ==========================================================
namespace schwa { namespace job01 { namespace dbg {


// Convenient ways to print an expression and what it evaluates to.
#define SCHWA_OSTREAM(x) #x << " == " << x
#define SCHWA_PRINT(x) cerr << SCHWA_OSTREAM(x) << endl;


// ScopeTimer starts timing when it is created, and executes a callback
// with the elapsed time when it is destroyed.
class ScopeTimer {
 public:
 	ScopeTimer(std::function<void(time::msecs)>&& callback) 
 	: _callback(callback), _start(time::clock::now()) { }

 	~ScopeTimer() {
 		time::msecs elapsed = time::duration(time::clock::now() - _start);
 		_callback(elapsed);
 	}

 private:
 	std::function<void(time::msecs)> _callback;
 	time::moment                     _start;
};


}}}  // schwa::job01::dbg =====================================================


#endif  // #ifndef __schwa__job01__dbg__dbg__