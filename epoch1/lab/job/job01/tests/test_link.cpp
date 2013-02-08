#include "job01/link.h"

using namespace schwa;

using job01::Link;
using job01::Linker;

#include <assert.h>

class TestLink : public Link<TestLink> {

};


template <class LinkT>
class GenericTestQueue : public Linker<LinkT> {
public:
    GenericTestQueue() : _head(nullptr), _tail(nullptr) { }

    // Links enter at the head (i.e. "eating")
    void add(LinkT* next) {
        assert(this->unlink(next) == nullptr);

        if (_tail == nullptr) {
            _head = _tail = next;
        } else {
            this->link(_head, next);
            _head = next;
        }
    }


    // Links exit at the tail (i.e. "pooping")
    LinkT* next() {
        if (_tail == nullptr) return nullptr;
        auto result = _tail;
        _tail = this->unlink(_tail);
        if (_tail == nullptr)
            _head = nullptr;
        return result;
    }

private:
    LinkT* _head;
    LinkT* _tail;
};


typedef GenericTestQueue<TestLink> TestQueue1;


typedef Linker<TestLink> TestLinker;

class TestQueue2 : public TestLinker {
public:
    TestQueue2() : _head(nullptr), _tail(nullptr) { }

    // Links enter at the head (i.e. "eating")
    void add(TestLink* next) {
        assert(this->unlink(next) == nullptr);

        if (_tail == nullptr) {
            _head = _tail = next;
        } else {
            this->link(_head, next);
            _head = next;
        }
    }


    // Links exit at the tail (i.e. "pooping")
    TestLink* next() {
        if (_tail == nullptr) return nullptr;
        auto result = _tail;
        _tail = this->unlink(_tail);
        if (_tail == nullptr)
            _head = nullptr;
        return result;
    }

private:
    TestLink* _head;
    TestLink* _tail;
};


int main(void) {
    TestLink link1, link2, link3;
    TestQueue1 q1;
    TestQueue2 q2;

    assert(q1.next() == nullptr);
    q1.add(&link1);
    assert(q1.next() == &link1);
    assert(q1.next() == nullptr);
    q1.add(&link2);
    q1.add(&link3);
    assert(q1.next() == &link2);
    assert(q1.next() == &link3);
    assert(q1.next() == nullptr);

    assert(q2.next() == nullptr);
    q2.add(&link1);
    assert(q2.next() == &link1);
    assert(q2.next() == nullptr);
    q2.add(&link2);
    q2.add(&link3);
    assert(q2.next() == &link2);
    assert(q2.next() == &link3);
    assert(q2.next() == nullptr);
}