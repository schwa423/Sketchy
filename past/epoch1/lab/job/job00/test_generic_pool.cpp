#include "job00/generic_pool.h"
#include "job00/link.h"
using namespace schwa::job00;

#include <assert.h>

#include <iostream>
#include <vector>
using namespace std;


// There is a JobPool::CACHE_LINE_SIZE, but we're testing GenericPool here.
const int CACHE_LINE_SIZE = 64;


// A pool starting with 10 free objects, each the size of (at most) a single cache-line.
template <class LinkT>
class SimplePool : public GenericPool<LinkT, CACHE_LINE_SIZE, 10> { };

// By construction, FooLink should be one byte smaller than the maximum size
// that will fit in a cache-line.
class Foo {
 protected:
    char foo[CACHE_LINE_SIZE - sizeof(Link<Foo>) - 1];
};
class FooLink : public Foo, public Link<FooLink> { };

// By construction, BarLink should fit perfectly into a single cache-line.
class Bar: public Foo {
 protected:
    char bar;
};
class BarLink : public Bar, public Link<BarLink> { };

// By construction, BazLink is one byte larger than a cache-line.
class Baz: public Bar {
 protected:
     char baz;
};
class BazLink : public Baz, public Link<BazLink> { };


// Test allocations and deallocations for the target pool.
template <class LinkT>
void validateAllocationsAndDeallocations(SimplePool<LinkT>& pool) {
    std::vector<LinkT*> v;
    LinkT* link;

    // Ensure that we can allocate the expected number of links.
    for (int i=0; i < pool.object_count(); i++) {
        link = pool.alloc();
        assert(link != nullptr);
        v.push_back(link);
    }

    // Ensure that no more links can be allocated (the pool should now be empty).
    link = pool.alloc();
    assert(link == nullptr);

    // Release one link back to the pool.  Then, we should be able to allocate
    // one more link, but not two.
    pool.dealloc(v.back());
    v.pop_back();
    link = pool.alloc();
    assert(link != nullptr);
    link = pool.alloc();
    assert(link == nullptr);
}


int main(void) {
    SimplePool<FooLink> foo_pool;
    validateAllocationsAndDeallocations(foo_pool);

    SimplePool<BarLink> bar_pool;
    validateAllocationsAndDeallocations(bar_pool);

#ifdef BREAK_COMPILE
    // BazLink is one byte larger than a cache-line, so this specialization
    // of SimplePool will result in failure static_assert in GenericPool.
    SimplePool<BazLink> baz_pool;
    validateAllocationsAndDeallocations(baz_pool);
#endif

  cerr << "successfully passed pool tests" << endl;

  return 0;
}