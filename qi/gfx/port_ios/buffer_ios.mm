#include "qi/gfx/port_ios/buffer_ios.h"

#include "qi/gfx/port_ios/device_ios.h"

namespace qi {
namespace gfx {
namespace port {

Buffer_iOS::Buffer_iOS(shared_ptr<Device> device, id<MTLBuffer> buffer)
    : Buffer(device), buffer_(buffer) {
  ASSERT(buffer);
}

auto Buffer_iOS::GetLength() -> size_t {
  return buffer_.length;
}

auto Buffer_iOS::GetContents() -> void* {
  return [buffer_ contents];
}

}  // namespace port
}  // namespace gfx
}  // namespace qi
