#include "qi/disk/impl/changelist.h"

namespace qi {
namespace disk {

Cache::Data ChangeList::Read(const Cache::Key& key) const {
  for (auto& change : changes_) {
    if (change.op != kNone && change.key == key) {
      return change.data;
    }
  }
  return Cache::Data();
}

void ChangeList::Write(Cache::Key key, Cache::Data data) {
  for (auto& change : changes_) {
    if (change.op != kNone && change.key == key) {
      change.op = kWrite;
      change.data = std::move(data);
      return;
    }
  }
  changes_.push_back({kWrite, std::move(key), std::move(data)});
}

void ChangeList::Erase(Cache::Key key) {
  for (auto& change : changes_) {
    if (change.op != kNone && change.key == key) {
      change.op = kErase;
      change.data.clear();
      return;
    }
  }
  changes_.push_back({kErase, std::move(key), Cache::Data()});
}

}  // namespace disk
}  // namespace qi
