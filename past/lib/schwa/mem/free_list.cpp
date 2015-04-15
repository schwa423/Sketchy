//
//  free_list.cpp
//  schwa::mem
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "free_list.h"


// namespace schwa::mem
namespace schwa {namespace mem {


// TODO: test that this constructor works
FreeList::FreeList(void* start, void* end, size_t elementSize)
    : FreeList(start,
               (static_cast<char*>(end) - static_cast<char*>(start)) / elementSize,
               elementSize) {

}


FreeList::FreeList(void* start, uint32_t elementCount, size_t elementSize)
    : _freeCount(elementCount), _maxCount(elementCount), _elementSize(elementSize) {
    union {
        void* as_void;
        char* as_char;
        Link* as_link;
    } tail;

    // Set up pointer to first element of free-list.
    tail.as_void = start;
    _head = tail.as_link;

    // Link remaining elements of free-list to the head.
    for (size_t i=1; i<elementCount; ++i) {
        auto next = reinterpret_cast<Link*>(tail.as_char + elementSize);
        link(tail.as_link, next);
        tail.as_link = next;
    }

    // Finally, the last element doesn't have a next element to link to.
    unlink(tail.as_link);
}


Link* FreeList::obtain() {
    Link* next = _head;
    if (_head != nullptr) {
        _head = unlink(_head);
        _freeCount--;
    }
    return next;
}


void FreeList::release(Link* newHead) {
    _head = link(newHead, _head);
    _freeCount++;
}


}}  // namespace schwa::mem