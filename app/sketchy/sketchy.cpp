//
//  sketchy.cpp
//  Sketchy
//
//  Created by Josh Gargus on 12/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "sketchy.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {
using namespace schwa::job;


// TODO: use some API to determine this at runtime.
static const int NUM_CPUS = 2;


// Singleton access to the app.
static Sketchy* TheSketchyApp = nullptr;
Sketchy& Sketchy::App() {
    if (!TheSketchyApp)
        TheSketchyApp = new Sketchy();
    return *TheSketchyApp;
}


Sketchy::Sketchy() : _cpuBoss(_pool, 2, NUM_CPUS), _ioBoss(_pool, 1, 6) {

}


Queue& Sketchy::highPriorityQueue() {
    return _cpuBoss.queueAt(0);
}


Queue& Sketchy::lowPriorityQueue() {
    return _cpuBoss.queueAt(1);
}


Queue& Sketchy::ioQueue() {
    return _ioBoss.queueAt(0);
}


}}}  // namespace schwa::app::sketchy
