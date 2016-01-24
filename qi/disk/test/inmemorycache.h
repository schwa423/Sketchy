#ifndef _QI_DISK_TEST_InMemoryCache_h_
#define _QI_DISK_TEST_InMemoryCache_h_

#include "qi/disk/cache.h"

namespace qi {
namespace disk {
namespace test {

// TODO: delete this, since Sqlite has an in-memory DB mode.
// Either that, or generalize cache_test.cc so that we test against
// both DB implementations.
class InMemoryCache : public Cache {
 public:
  InMemoryCache();
  ~InMemoryCache() override;

 protected:
  void RunQueries(std::vector<ReadRequest> reads,
                  std::unordered_map<Key, Data> writes,
                  std::unordered_set<Key> erases) override;

 private:
  std::unordered_map<Key, Data> stored_data_;
};

}  // namespace test
}  // namespace disk
}  // namespace qi

#endif   // _QI_DISK_TEST_InMemoryCache_h_
