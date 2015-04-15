//
//    stack.h
//    schwa::job01::core
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__core__stack__
#define __schwa__job01__core__stack__


#include "job01/core/link.h"
#include "job01/core/queue.h"

#include <assert.h>


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {


template <class LinkT>
class Stack : protected Linker<LinkT> {
 public:
    typedef LinkT ElemT;
    typedef typename LinkT::Ptr ElemPtr;

    // Default constructor... create an empty stack.
    Stack() : _top(nullptr), _count(0) { }

/*
    // Constructor which takes values from another stack.
    Stack(Stack<ElemT>&& rval)
      : _top(rval._top), _count(rval._count) {
        rval._top = nullptr;
        rval._count = 0;
    }
*/

  // Number of enqueued elements.
    int count() { return _count; }

    // True if there are any enqueued elements.
    bool empty() { return 0 == _count; }

    // Add element to the stack.
    void add(ElemPtr elem);

    // Move all elements from the other stack to the back of this stack.
    void add(Queue<ElemT>* other);
    void add(Queue<ElemT>& other) { this->add(&other); }
    void add(Queue<ElemT>&& other) { this->add(&other); }

    // Pull the next element from the stack, or nullptr.
    ElemPtr next();

    // Attempt to make a new stack of the desired length by taking elements
    // from this stack.  If this stack has too few elements, return the biggest
    // chain possible (afterward, this stack will be empty).
//    Queue<ElemT> next(int length);

protected:
    // It is the caller's responsibility to ensure that the head is
    // transitively linked to the tail, and that the count is correct.
    // TODO: debug-mode verification of these conditions.
    Stack(ElemPtr top, int count)
      : _top(top), _count(count) { }

private:
    // Quick sanity check... ensure that _top and _tail values
    // are consistent with _count.
    void sanityCheck();

    ElemPtr _top;
    int   _count;
};


// Add element to the stack.
template <class ElemT>
inline void Stack<ElemT>::add(typename ElemT::Ptr elem) {
    assert(elem != nullptr);
    assert(elem->nextLink() == nullptr);

    this->link(elem, _top);
    _top = elem;
    _count++;
}


// Move all elements from the other stack to the back of this stack.
template <class ElemT>
inline void Stack<ElemT>::add(Queue<ElemT>* other) {
    assert(other != nullptr);

    if (other->_count == 0) {
        // The queue is empty; it has no elements to add to this stack.
        return;
    }

    // Move the elements from the queue into this one.
    this->link(other->_head, _top);
    _top = other->_tail;
    _count += other->_count;

    other->_top = other->_tail = nullptr;
    other->_count = 0;
}


// Pull the next element from the stack, or nullptr.
template <class ElemT>
inline typename ElemT::Ptr Stack<ElemT>::next() {
    switch(_count) {
        case 0: {
            assert(_top == nullptr);
            return ElemPtr(nullptr);
        }
        default: {
            assert(_top != nullptr);
            auto result = _top;
            _top = this->unlink(_top);
            --_count;
            return result;
        }
    }
}


/*
// Attempt to make a new stack of the desired length by taking elements
// from this stack.  If this stack has too few elements, return the biggest
// chain possible (afterward, this stack will be empty).
template <class ElemT>
inline Queue<ElemT> Stack<ElemT>::next(int desiredLength) {
    if(_count)

    assert(desiredLength > 0);

    int remaining = desiredLength;
    ElemPtr head, tail;



    switch(_count) {
        case 0: {
            assert(_top == nullptr && _tail == nullptr);
            return Stack<ElemT>(nullptr, nullptr, 0);
        }
        case 1: {
            assert(_top == _tail);
            assert(_top->nextLink() == nullptr);
            auto result = _tail;
            _tail = _top = nullptr;
            _count = 0;
            return Stack<ElemT>(result, result, 1);
        }
        default: {
            // If there won't be extra elements left over, just take 'em all.
            if (desiredLength >= _count) {
                auto head = _top;
                auto tail = _tail;
                int count = _count;

                _tail = _top = nullptr;
                _count = 0;

                return Stack<ElemT>(head, tail, count);
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

                return Stack<ElemT>(head, tail, count);
            }
        }
    }
}
*/

/*
// Quick sanity check... ensure that _top and _tail values
// are consistent with _count.
template <class ElemT>
void Stack<ElemT>::sanityCheck() {
    assert(_count >= 0);
    switch (_count) {
        case 0:
            assert(_top == nullptr && _tail == nullptr);
            break;
        case 1:
            assert(_top == _tail && _top != nullptr);
            break;
        default:
            assert(_top != nullptr && _tail != nullptr && _top != _tail);
    }
}
*/

}}}  // schwa::job01::core ====================================================


#endif  // #ifndef __schwa__job01__core__stack__
