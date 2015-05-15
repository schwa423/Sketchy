#include "qi/gfx/port_ios/device_ios.h"

#include "qi/gfx/port_ios/buffer_ios.h"

namespace qi {
namespace gfx {

// Implmentation of static method from device.h
shared_ptr<Device> Device::GetDefault() {
  static weak_ptr<Device> s_weak;
  static Mutex s_mutex;

  LockGuard lock(s_mutex);
  auto ptr(s_weak.lock());
  if (!ptr) {
    ptr = make_shared<port::Device_iOS>(MTLCreateSystemDefaultDevice());
  }
  return ptr;
}

namespace port {

Device_iOS::Device_iOS(id<MTLDevice> device) : device_(device) {
  ASSERT(device);
}

auto Device_iOS::NewBuffer(size_t length) -> shared_ptr<Buffer> {
  id<MTLBuffer> buffer = [device_ newBufferWithLength: length options: 0];
  return make_shared<Buffer_iOS>(shared_from_this(), buffer);
}

}  // namespace port
}  // namespace gfx
}  // namespace qi
