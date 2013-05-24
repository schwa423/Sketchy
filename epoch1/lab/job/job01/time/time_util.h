//
//    time_util.h
//    schwa::job01::time
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Handy time-related utilities.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__time__util__
#define __schwa__job01__time__util__

#include <iostream>
#include <chrono>


// schwa::job01::time =========================================================
namespace schwa { namespace job01 { namespace time {

	typedef std::chrono::high_resolution_clock clock;
	typedef clock::time_point moment;	
	typedef std::chrono::milliseconds msecs;

	// This might seem scary, but the argument-type is what you
	// get when you subtract one moment from another.  For example:
	//    moment start_time = clock::now();
	//    // do other stuff
	//    msecs elapsed = duration(clock::now() - start_time)
	//    cerr << "other stuff took " << elapsed::
	template <class Rep, class Period>
	inline msecs duration(const std::chrono::duration<Rep,Period>& dur) {
		return std::chrono::duration_cast<msecs>(dur);
	}


}  // schwa::job01::time ======================================================
   // schwa::job01 ============================================================

// Print out time::msecs on ostream.
inline std::ostream& operator<< (std::ostream &out, time::msecs& millis) {
		out << millis.count() << "ms";
		return out;
}


}}  // schwa::job01 ===========================================================

#endif  // #ifndef __schwa__job01__time__util__