#ifndef __om_base_DCHECK__
#define __om_base_DCHECK__

inline void

#define DCHECK(expression)                           \
    if (!static_cast<bool>(expression))              \
      std::cerr << "DCHECK FAILED: " << #expression;

#define DCHECK_CURRENT_THREAD(t) DCHECK(t.get_id() == std::this_thread::get_id())
#define DCHECK_LOCK_OWNS(lock, mutex) \
    DCHECK(lock.owns_lock());         \
    DCHECK(lock.mutex() == &mutex)

#endif  // __om_base_DCHECK__
