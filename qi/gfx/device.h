#ifndef _QI_GFX_Device_h_
#define _QI_GFX_Device_h_

#include "qi.h"

namespace qi {
namespace gfx {

class Buffer;

class Device : public std::enable_shared_from_this<Device> {
 public:
  static shared_ptr<Device> GetDefault();

  // TODO: change to return unique_ptr
  virtual shared_ptr<Buffer> NewBuffer(size_t length) = 0;

 protected:
  virtual ~Device() {}
};

}  // namespace gfx
}  // namespace qi

#endif  // _QI_GFX_Device_h_
