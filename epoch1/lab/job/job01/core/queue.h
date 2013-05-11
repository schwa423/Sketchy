//
//  queue.h
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#ifndef __schwa__job01__core__queue__
#define __schwa__job01__core__queue__


#include "job01/core/link.h"
#include <assert.h>


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {


template <class LinkT>
class Queue : protected Linker<LinkT> {
 public:
    typedef LinkT ElemT;

    // Default constructor... create an empty queue.
    Queue() : _head(nullptr), _tail(nullptr), _count(0) { }

    // Constructor which takes values from another queue.
    Queue(Queue<ElemT>&& rval)
      : _head(rval._head), _tail(rval._tail), _count(rval._count) {
        rval._head = rval._tail = nullptr;
        rval._count = 0;
    }

	// Number of enqueued elements.
    int count() { return _count; }

    // Add element to the queue.
    void add(ElemT* elem);

    // Move all elements from the other queue to the back of this queue.
    void add(Queue<ElemT>* other);
    void add(Queue<ElemT>& other) { this->add(&other); }

    // Pull the next element from the queue, or nullptr.
    ElemT* next();

    // Attempt to make a new queue of the desired length by taking elements
    // from this queue.  If this queue has too few elements, return the biggest
    // chain possible (afterward, this queue will be empty).
    Queue<ElemT> next(int length);

protected:
    Queue(ElemT* head, ElemT* tail, int count)
      : _head(head), _tail(tail), _count(count) { }

private:
    // Quick sanity check... ensure that _head and _tail values
    // are consistent with _count.
    void sanityCheck();

    ElemT* _head;
    ElemT* _tail;
    int   _count;
};


// Add element to the queue.
template <class ElemT>
inline void Queue<ElemT>::add(ElemT* elem) {
    assert(elem != nullptr);
    assert(elem->nextLink() == nullptr);

    if (_count > 0) {
        // The queue is not empty, so link in the new element.
        this->link(_head, elem);
        _head = elem;
        _count++;
    } 
    else {
        // The queue is empty, so the new element is both the head and tail.
        _head = _tail = elem;
        _count = 1;
    }
}


// Move all elements from the other queue to the back of this queue.
template <class ElemT>
inline void Queue<ElemT>::add(Queue<ElemT>* other) {
    assert(other != nullptr);

    if (other->_count == 0) {
        // The other queue is empty; it has no elements to add to this queue.
        return;
    }

    // Move the elements from the other queue into this one.
    if (_count == 0) {
        // This queue is empty, so copy the head, tail, and count directly.
        _head = other->_head;
        _tail = other->_tail;
        _count = other->_count;
    }
    else {
        this->link(_head, other->_tail);
        _head = other->_head;
        _count += other->_count;
    }
    other->_head = other->_tail = nullptr;
    other->_count = 0;
}


// Pull the next element from the queue, or nullptr.
template <class ElemT>
inline ElemT* Queue<ElemT>::next() {
    switch(_count) {
        case 0: {
            assert(_head == nullptr && _tail == nullptr);
            return nullptr;
        }
        case 1: {
            assert(_head == _tail);
            assert(_head->nextLink() == nullptr);
            auto result = _tail;
            _tail = _head = nullptr;
            _count = 0;
            return result;
        }
        default: {
            assert(_head != _tail);
            auto result = _tail;
            _tail = this->unlink(_tail);
            --_count;
            return result;
        }
    }
}


// Attempt to make a new queue of the desired length by taking elements
// from this queue.  If this queue has too few elements, return the biggest
// chain possible (afterward, this queue will be empty).
template <class ElemT>
inline Queue<ElemT> Queue<ElemT>::next(int desiredLength) {
    assert(desiredLength > 0);
    switch(_count) {
        case 0: {
            assert(_head == nullptr && _tail == nullptr);
            return Queue<ElemT>(nullptr, nullptr, 0);
        }
        case 1: {
            assert(_head == _tail);
            assert(_head->nextLink() == nullptr);
            auto result = _tail;
            _tail = _head = nullptr;
            _count = 0;
            return Queue<ElemT>(result, result, 1);
        }
        default: {
            // If there won't be extra elements left over, just take 'em all.
            if (desiredLength >= _count) {
                auto head = _head;
                auto tail = _tail;
                int count = _count;

                _tail = _head = nullptr;
                _count = 0;

                return Queue<ElemT>(head, tail, count);
            }
            // Otherwise, there are more than enough enqueued elements,
            // so walk through them and detach the desired number.
            else {
                int count = 1;
                auto head = _tail;
                auto tail = _tail;

                while (count < desiredLength) {
                    head = head->nextLink();
                    count++;
                }

                _count -= count;
                _tail = head->nextLink();
                this->unlink(head);

                return Queue<ElemT>(head, tail, count);
            }
        }
    }
}


// Quick sanity check... ensure that _head and _tail values
// are consistent with _count.
template <class ElemT>
void Queue<ElemT>::sanityCheck() {
    assert(_count >= 0);
    switch (_count) {
        case 0:
            assert(_head == nullptr && _tail == nullptr);
            break;
        case 1:
            assert(_head == _tail && _head != nullptr);
            break;
        default:
            assert(_head != nullptr && _tail != nullptr && _head != _tail);
    }
}


}}}  // schwa::job01::core ====================================================


#endif  // #ifndef __schwa__job01__core__queue__
