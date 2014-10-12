#include "om/base/libom_base.h"

#include <iostream>

namespace om {

Foo::Foo() {}

Foo::~Foo() {
  std::cerr << "Foo was detroyed after being touched " << touch_count_ << " times." << std::endl;
}

void Foo::Touch() {
  ++touch_count_;
}

}  // namespace om
