//
// test_link.cpp
// schwa::job01::core::test
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#include "job01/core/link.h"

using namespace schwa;

using job01::Link;
using job01::Linker;

#include <iostream>
using namespace std;

#include <assert.h>


// Simple link, points to next element via SimpleLink*.
class SimpleLink : public Link<SimpleLink> {
public:
    SimpleLink() : _val(42) { }
    int fortyTwo() const { return _val; }
private:
    int _val;
};


// Fancier link, doesn't use a raw pointer to point to the next element.
class FancyLink;
class FancyLinkPtr {
public:
    explicit FancyLinkPtr(FancyLink* p) : _ptr(p) { }
    FancyLinkPtr& operator= (nullptr_t p) {
        _ptr = nullptr;
        return *this;
    }
    FancyLinkPtr& operator= (const FancyLinkPtr& p) {
        _ptr = p._ptr;
        return *this;
    }
    const FancyLink* operator->() const {
        return _ptr;
    }
    FancyLink* operator->() {
        return _ptr;
    }
    bool operator== (const FancyLinkPtr& p) const {
        return _ptr == p._ptr;
    }
    bool operator== (nullptr_t p) const {
        return _ptr == nullptr;
    }
    bool operator!= (const FancyLinkPtr& p) const {
        return _ptr != p._ptr;
    }
private:
    FancyLink* _ptr;
};

class FancyLink : public Link<FancyLink, FancyLinkPtr> {
public:
    FancyLink() : _val(42) { }
    int fortyTwo() const { return _val; }
private:
    int _val;
};


// Generic queue that can hold LinkTs. 
template <class LinkT>
class GenericTestQueue : public Linker<LinkT> {
public:
    typedef typename LinkT::Ptr LinkPtr;

    GenericTestQueue() : _head(nullptr), _tail(nullptr) { }

    // Links enter at the head (i.e. "eating")
    void add(LinkPtr next) {
        assert(this->unlink(next) == nullptr);

        if (_tail == nullptr) {
            _head = _tail = next;
        } else {
            this->link(_head, next);
            _head = next;
        }
    }

    // Links exit at the tail (i.e. "pooping")
    LinkPtr next() {
        if (_tail == nullptr) return _tail;
        auto result = _tail;
        _tail = this->unlink(_tail);
        if (_tail == nullptr)
            _head = _tail;
        return result;
    }

private:
    LinkPtr _head;
    LinkPtr _tail;
};


// Test function which creates a few links ands tests
// whether they can be enqueued/dequeued properly.
template <class LinkT>
void testEnqueueAndDequeue(void) {
    LinkT link1, link2, link3;
    typedef typename LinkT::Ptr PtrT;

    // For SimpleLinks we could compare below against &link1 etc.,
    // but not for FancyLinks because we chose to make the
    // FancyLinkPtr constructor explicit.
    PtrT p1(&link1), p2(&link2), p3(&link3);

    GenericTestQueue<LinkT> q1;
    GenericTestQueue<LinkT> q2;

    assert(q1.next() == nullptr);
    q1.add(p1);
    assert(q1.next() == p1);
    assert(q1.next() == nullptr);
    q1.add(p2);
    q1.add(p3);
    assert(q1.next() == p2);
    assert(q1.next() == p3);
    assert(q1.next() == nullptr);

    assert(q2.next() == nullptr);
    q2.add(p1);
    assert(q2.next() == p1);
    assert(q2.next() == nullptr);
    q2.add(p2);
    q2.add(p3);
    assert(q2.next() == p2);
    assert(q2.next() == p3);
    assert(q2.next() == nullptr);
}


int main(void) {
    testEnqueueAndDequeue<SimpleLink>();
    testEnqueueAndDequeue<FancyLink>();

    cerr << "job01/core/test_link...  PASSED!" << endl << endl;
}