#include "qi/dbg/port_ios/server_ios.h"

#include "qi.h"

namespace qi {
namespace dbg {
namespace port {

class Server_iOS::Impl {
 public:
  Impl(int port) { ASSERT(false); }
  void Start() { ASSERT(false); }
  void Stop() { ASSERT(false); }
};

Server_iOS::Server_iOS(int port) : impl_(new Server_iOS::Impl(port)) {}

Server_iOS::~Server_iOS() {
  delete impl_;
}

void Server_iOS::StartServer() {
  impl_->Start();
}

void Server_iOS::StopServer() {
  impl_->Stop();
}

}  // namespace port
}  // namespace dbg
}  // namespace qi
