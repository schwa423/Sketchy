//
//    benchmark_madvise.cc
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
//    Results:
//    - Discovered that memory bandwidth is a limiting factor.  The objects
//      originally used alignas(64) to mimic the layout of the job system's
//      pools.  This put them at an unfair disadvantage compared to function-
//      pointers.  We fix this by putting the pointers into FooHolder structs,
//      and giving that the same alignment as the objects.
//
//    - Some difference was made by varying the order of the subclasses used
//      to instantiate the objects, more for virtual functions than function
//      pointers.  Specifically, we tried the following:
//      - cycling through each subclass in turn... every 10 consecutive objects
//        includes one instance of each subclass
//      - allocating 10000 instances of one type, then 10000 of the next, etc.
//
//    - Timings:
//      Alignment   virtual/fptr      virtual/fptr
//                   (cycling)        (consecutive)
//      4:             20/21              21/21
//    16:            21/18              21/18
//      64:            37/30              35/28
//      128:           71/57              68/57
//
//      As you can see, with small objects there is no difference between
//      function pointers and virtual methods.  I conjecture this is because
//      the lower memory requirements mean that the virtual function table
//      can remain in cache (a bit of a guess, I admit).  On the other end,
//      by the time you get up to 64 and 128 bit objects, there is a clear
//      win for function-pointers.
//
//      Ordering of instances makes a small difference for virtual functions.
//      This is good news for us, because we can't rely on adjacent jobs having
//      any predictable ordering.
//
//    Conclusion:
//    - I'm happy to conclude that virtual functions are more than fast enough
//      for our purposes!!
//    - I'm glad I didn't prematurely optimize by using function pointers...
//      I was tempted to follow the lead of Molecular Musings, but this will
//      be cleaner.
//
///////////////////////////////////////////////////////////////////////////////
