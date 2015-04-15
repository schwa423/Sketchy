//
//    test_obj.cc
//    schwa::job01::obj
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//////////////////////////////////////////////////////////////////////////////

#include "job01/obj/obj.h"
#include "job01/dbg/dbg.h"

using namespace schwa::job01;
using namespace schwa::job01::obj;

#include <iostream>
using std::cerr;
using std::endl;


class Foo : public Obj<Foo> {

};
typedef ObjRef<Foo> FooRef;


class Bar : public Obj<Bar> {
    char padding[100];
};
typedef ObjRef<Bar> BarRef;


class FooBarMaker : public ObjMaker {
    friend class FooPool;
    friend class BarPool;    

    static core::Queue<Foo> MakeSomeFoos(int desiredFoos) {
        return ObjMaker::MakeObjects<Foo>(desiredFoos);
    }

    static core::Queue<Bar> MakeSomeBars(int desiredBars) {
        return ObjMaker::MakeObjects<Bar>(desiredBars);
    }
};


class FooPool : public core::Queue<Foo> {
 public:
    FooRef next() {
        if (count() == 0)
            GetSomeFoos();
        return core::Queue<Foo>::next();
    }

    core::Queue<Foo> next(int desiredCount) {
        while(count() < desiredCount && GetSomeFoos()) { }
        return core::Queue<Foo>::next(desiredCount);
    }

 private:
    // Try to get 100 Foos.  Return true if successful, 
    // and false if we can't get that many.
    bool GetSomeFoos() {
        auto newFoos = FooBarMaker::MakeSomeFoos(100);
        bool result = newFoos.count() == 100;
        add(newFoos);
        return result;
    }
};


class BarPool : public core::Queue<Bar> {
 public:
    BarRef next() {
        if (count() == 0)
            GetSomeBars();
        return core::Queue<Bar>::next();
    }

    core::Queue<Bar> next(int desiredCount) {
        while(count() < desiredCount && GetSomeBars()) { }
        return core::Queue<Bar>::next(desiredCount);
    }

 private:
    // Try to get 100 Bars.  Return true if successful, 
    // and false if we can't get that many.
    bool GetSomeBars() {
        auto newBars = FooBarMaker::MakeSomeBars(100);
        bool result = newBars.count() == 100;
        add(newBars);
        return result;
    }
};


void AssertUnbornCount(int count, ObjSizeCode size_code) {
    assert(FooBarMaker::UnbornCount(size_code) == count);    
}


int main() {
    dbg::ScopeTimer t([](time::msecs elapsed) {
        cerr << "job01/obj/test_obj...  PASSED! ("
             << elapsed << ")" << endl << endl;
    });

    core::Queue<Foo> temp_foos;
    core::Queue<Bar> temp_bars;
    FooPool foo_pool;
    BarPool bar_pool;    

    // Unborn objects are lazily allocated, upon demand.
    AssertUnbornCount(0, k64Bytes);
    AssertUnbornCount(0, k128Bytes);
    AssertUnbornCount(0, k256Bytes);
    AssertUnbornCount(0, k512Bytes);

    temp_foos.add(foo_pool.next(10));
    temp_bars.add(bar_pool.next(50));

    // We've now allocated some Foos (64-byte objects),
    // and some Bars (128-byte objects).  FooPool and
    // BarPool ask for 100 objects at a time, whereas
    // ObjMaker allocates batches of 65535.  So, there
    // will be 100 less than that in ObjMaker's free-list
    // of both 64- and 128-byte unborn objects.
    const int max_count = 65535;  // aka ObjMaker::kMaxArraySize
    AssertUnbornCount((max_count - 100), k64Bytes);
    AssertUnbornCount((max_count - 100), k128Bytes);
    AssertUnbornCount(0, k256Bytes);
    AssertUnbornCount(0, k512Bytes);

    // Grab 50 more from BarPool... this won't make any difference
    // to ObjMaker because BarPool still has 50 in reserve.
    temp_bars.add(bar_pool.next(50));
    AssertUnbornCount((max_count - 100), k128Bytes);

    // However, if we add pull one more from BarPool, it will
    // trigger 100 more to be pulled from ObjMaker.
    temp_bars.add(bar_pool.next());
    AssertUnbornCount((max_count - 200), k128Bytes);

    // Foos are smaller than Bars, so let's pull a total of
    // 65500 from the ObjMaker.
    temp_foos.add(foo_pool.next(65490));  // 10 were pulled earlier.
    AssertUnbornCount(35, k64Bytes);

    // Pulling 1 more Foo will trigger the FooPool to get 100 more
    // from the ObjMaker.  However, since it has only 35 left, another
    // 65535 will be allocated (total of 35 + 65535 - 100 = 65470).
    temp_foos.add(foo_pool.next());
    AssertUnbornCount(65470, k64Bytes);

    // As before, pulling another 99 won't change the number of unborn
    // objects in the ObjMaker, because the FooPool has 99 in reserve.
    temp_foos.add(foo_pool.next(99));
    AssertUnbornCount(65470, k64Bytes);
}
