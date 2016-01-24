#ifndef _QI_Qi_h_
#define _QI_Qi_h_

#include "qi/base/types.h"

#include <memory>
#include <mutex>
#include <string>

#define QI_DEBUG 1

#ifdef QI_DEBUG
#include <assert.h>
#define ASSERT(pred) assert(pred)
#else
#define ASSERT(pred)
#endif

namespace qi {

class Qi {
 public:
  static void Init();
  static void Shutdown();

  static int RunAllTests();

  // Run |closure| asynchronously on a background thread.  The work done by
  // |closure| should be CPU-intensive; it shouldn't block waiting for timers,
  // synchronous I/O, etc.
  static void Run(std::function<void()> closure);

  // Valid values for |desc|:
  // - "docs"
  // TODO: better documentation.
  static std::string GetDirectoryPath(std::string desc);
};

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::make_shared;
using std::make_unique;
// TODO: find/replace "std::move" with "move".
using std::move;

using Mutex = std::mutex;
using LockGuard = std::lock_guard<std::mutex>;

template <class T, class U>
T safe_cast(U val) {
  ASSERT(dynamic_cast<T>(val));
  return static_cast<T>(val);
}

}  // namespace qi

#endif  // _QI_Qi_h_
