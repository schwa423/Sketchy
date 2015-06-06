#ifndef _QI_DBG_PORT_Server_iOS_h_
#define _QI_DBG_PORT_Server_iOS_h_

#include "qi/dbg/server.h"

namespace qi {
namespace dbg {
namespace port {

class Server_iOS : public Server {
 public:
  Server_iOS(int port);
  ~Server_iOS() override;

 protected:
  void StartServer() override;
  void StopServer() override;

  class Impl;
  Impl* const impl_;
};

}  // namespace port
}  // namespace dbg
}  // namespace qi

#endif  // _QI_DBG_PORT_Server_iOS_h_
