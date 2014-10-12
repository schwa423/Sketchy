#include "om/io/net.h"

#include <iostream>

namespace om {
namespace io {

void FooToucher::TouchFoo(Foo* foo, int times) {
  std::cerr << "About to touch Foo: " << foo << "  " << times << "times." << std::endl;
  for (int i = 0; i < times; ++i) {
    foo->Touch();
  }
}

}  // namespace io
}  // namespace om
