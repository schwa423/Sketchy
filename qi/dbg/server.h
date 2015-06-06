#ifndef _QI_DBG_Server_h_
#define _QI_DBG_Server_h_

#include <memory>
#include <utility>

namespace qi {
namespace dbg {

class Server {
 public:
  virtual ~Server() {}

  template <typename ServerT, typename ...ArgsT>
  static void Init(ArgsT&&... args);

  static void Start();
  static void Stop();

 protected:
  virtual void StartServer() = 0;
  virtual void StopServer() = 0;


 private:
  static void Set(std::unique_ptr<Server> server);
 };

template <typename ServerT, typename ...ArgsT>
void Server::Init(ArgsT&&... args) {
  Server::Set(std::make_unique<ServerT>(std::forward<ArgsT>(args)...));
//  Server::Set(std::make_unique<ServerT>(args...));
}

}  // namespace dbg
}  // namespace qi

#endif  // _QI_DBG_Server_h_


template<class ...Us> void f(Us... pargs) {}
