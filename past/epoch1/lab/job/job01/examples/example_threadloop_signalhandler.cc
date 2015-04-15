//
//    example_threadloop_signalhandler.cc
//    schwa::job01 example
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/thread/threadloop.h"
using namespace schwa;

#include <iostream>
using namespace std;

class ThreadLoopSignalHandler : public job01::thread::ThreadLoop {



};


class KeyboardHandler {

  KeyboardHandler(ThreadLoopSignalHandler& tlsh1,
                  ThreadLoopSignalHandler& tlsh2) : state_(kNone),
                                                    loop1_(tlsh1),
                                                    loop2_(tlsh2) {}
  void processInput() {


  }

  char nextChar() {
    c = getchar();
    if (kReturnKey == c)
      exit(0);
    return c;
  }


 private:
  enum State { kNone, kSignal };

  State state_;
  ThreadLoopSignalHandler& loop1_;
  ThreadLoopSignalHandler& loop2_;

  const char kReturnKey = 10;

  const char kS

};



int main(void) {

  ThreadLoopSignalHandler loop1;
  ThreadLoopSignalHandler loop2;

  KeyboardHandler handler(loop1, loop2);
  handler.processInput();


  const char ret = 10;

  int c;
  while (EOF != (c = getchar())) {
    if (c != ret) {
      cerr << "received char: " << c << endl;
    }
  }
}
