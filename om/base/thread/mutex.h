#ifndef __om_base_Mutex__
#define __om_base_Mutex__

#include <mutex>

namespace om {

typedef std::mutex Mutex;
typedef std::lock_guard<Mutex> Lock;
typedef std::unique_lock<Mutex> UniqueLock;

}

#endif  // __om_base_Mutex__
