#ifndef _QI_GFX_PORT_Buffer_iOS_h_
#define _QI_GFX_PORT_Buffer_iOS_h_

#include "qi/gfx/buffer.h"

#include <Metal/Metal.h>

namespace qi {
namespace gfx {
namespace port {

class Buffer_iOS : public Buffer {
 public:
  explicit Buffer_iOS(shared_ptr<Device> device, id<MTLBuffer> buffer);
  virtual ~Buffer_iOS() {}

  size_t GetLength() override;
  void* GetContents() override;

 private:
  id<MTLBuffer> buffer_;
};

}  // namespace port
}  // namespace gfx
}  // namespace qi

#endif  // _QI_GFX_PORT_Buffer_iOS_h_
