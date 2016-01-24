#ifndef _QI_DISK_SqliteCache_h_
#define _QI_DISK_SqliteCache_h_

#include "qi/disk/cache.h"

struct sqlite3;
struct sqlite3_stmt;

namespace qi {
namespace disk {

class SqliteCache : public Cache {
 public:
  SqliteCache();
  ~SqliteCache() override;

 protected:
  void RunQueries(std::vector<ReadRequest> reads,
                  std::unordered_map<Key, Data> writes,
                  std::unordered_set<Key> erases) override;

 private:
  sqlite3* db_;
  sqlite3_stmt* read_stmt_;
  sqlite3_stmt* write_stmt_;
  sqlite3_stmt* erase_stmt_;
};

}  // namespace disk
}  // namespace qi

#endif   // _QI_DISK_SqliteCache_h_
