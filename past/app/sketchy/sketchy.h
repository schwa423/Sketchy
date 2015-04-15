//
//  sketchy.h
//  Sketchy
//
//  Created by Josh Gargus on 12/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//
//  Although Sketchy Pages and Books will eventually be self-contained media objects
//  which may be embedded in a generalized collaborative hypermedia environment,
//  we're not at that point yet... we need a standalone app for Pages to live in.
//
//  Sketchy is that app.
//
//  It also represents my initial exploration into the eventual global architecture
//  of Schwa.  Broadly speaking (which is all I know enough to do at this point),
//  the app is will be responsible for managing such concerns as:
//  - scheduling computation
//  - orchestrating communication
//  - controlling global memory usage
//  - responding to app-lifecycle events from the host OS
//
//  Since I've recently been focused on schwa::job, the first responsibility is
//  holding the job-scheduling machinery.
//
//

#ifndef __schwa__app__sketchy__sketchy__
#define __schwa__app__sketchy__sketchy__


#include "boss.h"
#include "pool.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


class Sketchy {
 public:
    Sketchy();

    // Singleton.
    static Sketchy& App();

    // Obtain a reference to the various job-queues.
    job::Queue& highPriorityQueue();
    job::Queue& lowPriorityQueue();
    job::Queue& ioQueue();

 protected:
    job::JobPool _pool;
    job::Boss    _cpuBoss;

    // Replace this with an IOManager that layers a more typical
    // async-IO API over the Job API.
    job::Boss    _ioBoss;
};


}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__sketchy__


