#ifndef _QI_Qi_h_
#define _QI_Qi_h_

#include <memory>
#include <mutex>

#ifdef QI_DEBUG
#define ASSERT(pred) assert(pred)
#else
#define ASSERT(pred)
#endif

namespace qi {

class Qi {};

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::make_shared;

using Mutex = std::mutex;
using LockGuard = std::lock_guard<std::mutex>;

template <class T, class U>
T safe_cast(U val) {
  ASSERT(dynamic_cast<T>(val));
  return static_cast<T>(val);
}

}  // namespace qi

#endif  // _QI_Qi_h_
