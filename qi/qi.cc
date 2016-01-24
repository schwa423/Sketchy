#include "qi/qi.h"
#include "qi/base/port_ios/runner_ios.h"
#include "qi/dbg/port_ios/server_ios.h"

#include "gtest/gtest.h"

namespace qi {

namespace {

struct QiState {
  Mutex mutex;
  bool is_initialized;
  std::unique_ptr<IRunner> runner;

  QiState() : is_initialized(false) {}
};

QiState& GetQiState() {
  static QiState s_state;
  return s_state;
}

}  // anonymous namespace

void Qi::Init() {
  auto& state = GetQiState();
  LockGuard lock(state.mutex);
  ASSERT(!state.is_initialized);
  state.is_initialized = true;

  state.runner = std::make_unique<port::Runner_iOS>();
  dbg::Server::Init<dbg::port::Server_iOS>(12345);
}

void Qi::Shutdown() {
  auto& state = GetQiState();

  LockGuard lock(state.mutex);
  ASSERT(state.is_initialized);
  state.is_initialized = false;

  state.runner->Shutdown();
  state.runner.reset();

  // TODO: Shutdown debug server completely (not sure if the following line is
  // sufficient?).
  qi::dbg::Server::Stop();
}

void Qi::Run(std::function<void()> closure) {
  GetQiState().runner->Run(std::move(closure));
}

int Qi::RunAllTests() {
  int argc = 1;
  char* name = const_cast<char*>("Qi::RunAllTests()");
  testing::InitGoogleTest(&argc, &name);
  return RUN_ALL_TESTS();
}

}  // namespace qi
