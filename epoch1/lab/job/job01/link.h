// Josh Gargus 2013

#ifndef __schwa__job01__link__
#define __schwa__job01__link__


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


template <class LinkT>
class Linker;

template <class LinkT>
class Link {
    friend class Linker<LinkT>;

public:
    Link() : _next(nullptr) { }
    LinkT* nextLink() { return _next; }

private:
    LinkT* _next;
};


template <class LinkT>
class Linker {
 protected:
    // Link target to its new next link, and return that next link.
    inline LinkT* link(LinkT* target, LinkT* next) {
        target->_next = next;
        return next;
    }

    // Unlink target from its next link, and return that link.
    inline LinkT* unlink(LinkT* target) {
        auto next = target->_next;
        target->_next = nullptr;
        return next;
    }
};


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__link__