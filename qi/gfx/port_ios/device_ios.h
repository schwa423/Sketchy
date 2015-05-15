#ifndef _QI_GFX_PORT_Device_iOS_h_
#define _QI_GFX_PORT_Device_iOS_h_

#include "qi/gfx/device.h"

#include <Metal/Metal.h>

namespace qi {
namespace gfx {
namespace port {

class Device_iOS : public Device {
 public:
  explicit Device_iOS(id<MTLDevice> device);
  virtual ~Device_iOS() {}
  shared_ptr<Buffer> NewBuffer(size_t length) override;

 private:
  id<MTLDevice> device_;
};

}  // namespace port
}  // namespace gfx
}  // namespace qi

#endif  // _QI_GFX_PORT_Device_iOS_h_
