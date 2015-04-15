//
//    link.h
//    schwa::job01::core
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Low-level building blocks for intrusively-linked data structures.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__core__link__
#define __schwa__job01__core__link__


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {


template <class LinkT>
class Linker;

#pragma pack(push, 1)
template <class LinkT, typename LinkPtr = LinkT*>
class Link {
    friend class Linker<LinkT>;

public:
    typedef LinkPtr Ptr;

    Link() : _next(nullptr) { }
    Link(LinkPtr& ptr) : _next(ptr) { }
    LinkPtr nextLink() { return _next; }

    virtual ~Link() {}
private:
    LinkPtr _next;
};
#pragma pack(pop)

template <class LinkT>
class Linker {
 protected:
    typedef typename LinkT::Ptr LinkPtr;

    // Link target to its new next link, and return that next link.
    inline LinkPtr link(LinkPtr target, LinkPtr next) {
        target->_next = next;
        return next;
    }

    // Unlink target from its next link, and return that link.
    inline LinkPtr unlink(LinkPtr target) {
        auto next = target->_next;
        target->_next = nullptr;
        return next;
    }
};


}}}  // schwa::job01::core ====================================================

#endif  // #ifndef __schwa__job01__core__link__
