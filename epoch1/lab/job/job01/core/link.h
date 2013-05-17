//
//  link.h
//  schwa::job01
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//


#ifndef __schwa__job01__link__
#define __schwa__job01__link__


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


template <class LinkT>
class Linker;

template <class LinkT, typename LinkPtr = LinkT*>
class Link {
    friend class Linker<LinkT>;

public:
    typedef LinkPtr Ptr;

    Link() : _next(nullptr) { }
    LinkPtr nextLink() { return _next; }

private:
    LinkPtr _next;
};


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


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__link__