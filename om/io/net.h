#include "om/base/libom_base.h"

namespace om {
namespace io {

class FooToucher {
 public:
  FooToucher() {}
  void TouchFoo(Foo* foo, int times);
};

}  // namespace io
}  // namespace om
