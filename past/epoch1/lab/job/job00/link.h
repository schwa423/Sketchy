#ifndef __schwa__job00__link__
#define __schwa__job00__link__


// schwa::job00 ===============================================================
namespace schwa { namespace job00 {


template <class LinkT>
class Link {
public:
    Link() : link_(nullptr) { }
    LinkT* link(LinkT* link) {
      link_ = link;
      return link;
    }
    LinkT* unlink() {
      auto old_link = link_;
      link_ = nullptr;
      return old_link;
    }
    LinkT* next() const { return link_; }
private:
    LinkT* link_;
};


}}  // schwa::job00 ===========================================================


#endif  // #ifndef __schwa__job00__link__