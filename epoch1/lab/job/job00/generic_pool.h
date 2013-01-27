#ifndef __schwa__job__generic_pool__
#define __schwa__job__generic_pool__

#include <type_traits>
#include <cstddef>
#include <stdint.h>
#include <new>
#include <functional>

// Generic pool of linked objects which support the following interface:
//  class LinkT {
//     LinkT* link(LinkT*);   // link to a new link, and return that link.
//     LinkT* unlink();       // unlink from next link, if any, and return it,
//  }

template <class LinkT>
class LinkedPool {
public:
    LinkedPool() : free_(nullptr) { }

    // Return a pointer to a free object, or nullptr.
    LinkT* alloc() {
        auto ptr = free_;
        if (free_ != nullptr) {
           free_ = free_->unlink();
        }
        return ptr;
    }

    // Return object to the free-list.
    void dealloc(LinkT* obj) {
        obj->link(free_);
        free_ = obj;
    }

protected:
    LinkedPool(const LinkedPool& pool);  // no copy constructor allowed

    // Simple way to do something for all currently-free objects.
    // Useful for initialization and debugging, but not for general use.
    void for_each(std::function<void(LinkT*)> func) {
        auto current = free_;
        while (current) {
            func(current);
            current = current->next();
        }
    }

    LinkT* free_;
};


template <class LinkT, size_t OBJECT_SIZE, int OBJECT_COUNT>
class GenericPool : public LinkedPool<LinkT> {
    static_assert (sizeof(LinkT) <= OBJECT_SIZE, "LinkT is too big!");
    using LinkedPool<LinkT>::free_;

public:
    GenericPool() {
        // Currently no free objects... we'll add them one-by-one.
        free_ = nullptr;

        // Initialize list of free-objects.
        auto ptr = mem_;
        for (int i = 0; i < OBJECT_COUNT; i++) {
            LinkT* next = new (ptr) LinkT();
            next->link(free_);
            free_ = next;
            ptr += OBJECT_SIZE;
        }
    }

    ~GenericPool() {
        // TODO: assert that there are no allocated objects... either maintain a count,
        // or iterate through list and ensure that there are OBJECT_COUNT free items.
    }

    size_t object_size() const { return OBJECT_SIZE; }
    size_t object_count() const { return OBJECT_COUNT; }

private:
    // Raw bytes where linked objects are stored.
    uint8_t mem_[OBJECT_SIZE * OBJECT_COUNT];
};


#endif  // #ifndef __schwa__job__generic_pool__