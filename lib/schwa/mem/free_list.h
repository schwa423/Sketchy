//
//  free_list.h
//  schwa::mem
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__mem__free_list__
#define __schwa__mem__free_list__

#include <cstddef>

// namespace schwa::mem
namespace schwa {namespace mem {


// Free-list for rapid allocation of fixed-size objects.
// Inspired by "Molecular Musings" blog.
class FreeList {
 public:
    FreeList(void* start, void* end, size_t elementSize);
    FreeList(void* start, int elementCount, size_t elementSize);

    // Obtain pointer to unused element, or nullptr if none are available.
    void* obtain();

    // Add the element back to the free-list, so it can be reused.
    void release(void* ptr);

    // Return the current / maximum number of elements in the free-list.
    int freeCount() const { return _freeCount; }
    int maxCount() const { return _maxCount; }

    // Elements in free-list must be subclasses of Link.
    // TODO: will this break in cases where there is multiple-inheritance?
    struct Link {
        friend class FreeList;
     private:
        Link* next;
    };

 protected:
    Link* _head;

    // Free-list could operate without these values,
    // but it doesn't hurt to keep track of them.
    int _freeCount;
    int _maxCount;
    size_t _elementSize;
};


}}  // namespace schwa::mem


#endif  // #ifndef __schwa__mem__free_list__

