#ifndef _QI_GFX_Buffer_h_
#define _QI_GFX_Buffer_h_

#include "qi.h"

namespace qi {
namespace gfx {

class Device;

class Buffer {
 public:
  virtual size_t GetLength() = 0;
  virtual void* GetContents() = 0;

 protected:
  explicit Buffer(shared_ptr<Device> device = nullptr) : device_(device) {}
  virtual ~Buffer() {}

  shared_ptr<Device> device_;
};

}  // namespace gfx
}  // namespace qi

#endif  // _QI_GFX_Buffer_h_
