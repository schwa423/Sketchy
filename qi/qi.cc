#include "qi/qi.h"
#include "qi/dbg/port_ios/server_ios.h"

namespace qi {

void Qi::Init() {
  dbg::Server::Init<dbg::port::Server_iOS>(12345);
}

}  // namespace qi
