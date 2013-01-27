#ifndef __schwa__job__link__
#define __schwa__job__link__


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


#endif  // #ifndef __schwa__job__link__