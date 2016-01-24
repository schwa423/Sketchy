#include "qi/disk/test/inmemorycache.h"

namespace qi {
namespace disk {
namespace test {

InMemoryCache::InMemoryCache() {}
InMemoryCache::~InMemoryCache() {}

void InMemoryCache::RunQueries(std::vector<ReadRequest> reads,
                               std::unordered_map<Key, Data> writes,
                               std::unordered_set<Key> erases) {
  for (auto& req : reads) {
    auto it = stored_data_.find(req.first);
    if (it != stored_data_.end()) {
      req.second(kSuccess, it->second);
    } else {
      req.second(kNotFound, Data());
    }
  }
  for (auto& req : writes) {
    stored_data_[move(req.first)] = move(req.second);
  }
  for (auto& req : erases) {
    stored_data_.erase(req);
  }
}

}  // namespace test
}  // namespace disk
}  // namespace qi
