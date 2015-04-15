//
//  free_list.h
//  schwa::mem
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__mem__free_list__
#define __schwa__mem__free_list__

#include <cstddef>
#include <stdint.h>


// namespace schwa::mem
namespace schwa {namespace mem {


struct Link {
    friend class Linker;
    const Link* nextLink() const { return _next; }
 private:
    Link* _next;
};


class Linker {
 protected:
    // Link target to its new next link, and return the target.
    // TODO: perhaps we should return "next" instead of "target", since
    //       we probably have "target" in our hand, but maybe not "next"?
    inline Link* link(Link* target, Link* next) const {
        target->_next = next;
        return target;
    }

    // Unlink target from its next link, and return that link.
    inline Link* unlink(Link* target) const {
        auto next = target->_next;
        target->_next = nullptr;
        return next;
    }
};


// Free-list for rapid allocation of fixed-size objects.
// Inspired by "Molecular Musings" blog.
class FreeList : public Linker {
 public:
    FreeList(void* start, void* end, size_t elementSize);
    FreeList(void* start, uint32_t elementCount, size_t elementSize);

    // Obtain pointer to unused element, or nullptr if none are available.
    Link* obtain();

    // Add the element back to the free-list, so it can be reused.
    void release(Link* ptr);

    // Return the current / maximum number of elements in the free-list.
    uint32_t freeCount() const { return _freeCount; }
    uint32_t maxCount() const { return _maxCount; }

 protected:
    Link* _head;

    // Free-list could operate without these values,
    // but it doesn't hurt to keep track of them.
    uint32_t _freeCount;
    uint32_t _maxCount;
    size_t _elementSize;
};


}}  // namespace schwa::mem


#endif  // #ifndef __schwa__mem__free_list__

