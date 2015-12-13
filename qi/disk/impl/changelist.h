#ifndef _QI_DISK_IMPL_ChangeList_h_
#define _QI_DISK_IMPL_ChangeList_h_

#include "qi/disk/cache.h"

namespace qi {
namespace disk {

// Cache that implements an in-memory queue to hold write/erase operations
// before they are applied to persistent storage.  This is probably not useful
// to end-users,
class ChangeList {
 public:
  enum Op { kNone, kWrite, kErase };
  struct Change {
    Op op;
    Cache::Key key;
    Cache::Data data;

    void Reset() {
      op = kNone;
      key = Cache::Key();
      data = Cache::Data();
    }
  };

  // Return the data corresponding to the most recent kWrite operation,
  // or empty data if none exists.
  Cache::Data Read(const Cache::Key& key) const;

  // Replaces any existing kWrite or kErase operation for the specified key.
  void Write(Cache::Key key, Cache::Data data);

  // Replaces any existing kWrite or kErase operation for the specified key.
  void Erase(Cache::Key key);

 private:
  std::vector<Change> changes_;
};

}  // namespace disk
}  // namespace qi

#endif   // _QI_DISK_IMPL_ChangeList_h_
