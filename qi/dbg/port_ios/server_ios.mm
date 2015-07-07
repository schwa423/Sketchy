#include "qi/dbg/port_ios/server_ios.h"

#include <iostream>

#include "qi.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace qi {
namespace dbg {
namespace port {

// TODO: add synchronization
class Server_iOS::Impl {
 public:
  Impl(int port);
  void Start();
  void Stop();
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

Server_iOS::Impl::Impl(int port) {

  lua_State* lua = luaL_newstate();
  luaL_openlibs(lua);

  int error = luaL_loadstring(lua,
      "days = {'Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday'}  \n"
      "for k,v in pairs(days) do                                                  \n"
      "  print ('The next day is: ' .. v)                                         \n"
      "end                                                                        \n"
  ) || lua_pcall(lua, 0, 0, 0);

  if (error) {
    std::cerr << "************** LUA ERROR: " << lua_tostring(lua, -1);
  }

  lua_close(lua);
}

void Server_iOS::Impl::Start() {
  std::cerr << "Starting server........." << std::endl;
}

void Server_iOS::Impl::Stop() {
  std::cerr << "Stopping server........." << std::endl;
}

}  // namespace port
}  // namespace dbg
}  // namespace qi
