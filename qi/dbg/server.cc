#include "qi/dbg/server.h"

#include "qi.h"

namespace qi {
namespace dbg {

static unique_ptr<Server>& GetPtr() {
  static unique_ptr<Server> s_ptr;
  return s_ptr;
}

void Server::Set(std::unique_ptr<Server> server) {
  ASSERT(!GetPtr().get());
  GetPtr() = move(server);
}

void Server::Start() { GetPtr()->StartServer(); }
void Server::Stop() { GetPtr()->StopServer(); }

}  // namespace dbg
}  // namespace qi
