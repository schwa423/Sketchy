#include "qi/disk/sqlitecache.h"

#include <sqlite3.h>

namespace qi {
namespace disk {

SqliteCache::SqliteCache(std::string path, size_t flags) {
  int result = sqlite3_open_v2(
      path.c_str(),
      &db_,
      flags ? flags : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
      nullptr);
  ASSERT(result == SQLITE_OK);

  const char* create_table =
      "CREATE TABLE keyvalue"
      "\n ("
      "\n   key TEXT,"
      "\n   value BLOB,"
      "\n   PRIMARY KEY(key)"
      "\n )";
  result = sqlite3_exec(db_, create_table, nullptr, nullptr, nullptr);
  ASSERT(result == SQLITE_OK);

  const char* read_sql = "SELECT key, value FROM keyvalue WHERE key = ?1;";
  result = sqlite3_prepare_v2(db_, read_sql, -1, &read_stmt_, nullptr);
  ASSERT(result == SQLITE_OK);

  const char* write_sql = "INSERT INTO keyvalue (key, value) VALUES (?1, ?2);";
  result = sqlite3_prepare_v2(db_, write_sql, -1, &write_stmt_, nullptr);
  ASSERT(result == SQLITE_OK);

  const char* erase_sql = "DELETE FROM keyvalue WHERE key = ?1;";
  result = sqlite3_prepare_v2(db_, erase_sql, -1, &erase_stmt_, nullptr);
  ASSERT(result == SQLITE_OK);
}

SqliteCache::~SqliteCache() {
  if (db_) {
    sqlite3_close(db_);
  }
}

// TODO: invesigate uses of SQLITE_TRANSIENT; see if SQLITE_STATIC might work.
void SqliteCache::RunQueries(std::vector<ReadRequest> reads,
                             std::unordered_map<Key, Data> writes,
                             std::unordered_set<Key> erases) {
  for (auto& req : reads) {
    auto& key = req.first;
    auto& callback = req.second;
    int result = sqlite3_bind_text(read_stmt_, 1,
                                   key.c_str(), static_cast<int>(key.length()),
                                   SQLITE_TRANSIENT);
    ASSERT(result == SQLITE_OK);

    result = sqlite3_step(read_stmt_);
    ASSERT(result == SQLITE_ROW || result == SQLITE_DONE);
    if (result == SQLITE_ROW) {
      auto blob = static_cast<const byte*>(sqlite3_column_blob(read_stmt_, 1));
      int blob_size = sqlite3_column_bytes(read_stmt_, 1);
      callback(kSuccess, Data(blob, blob + blob_size));

      // Must be at most one value for each key.
      ASSERT(SQLITE_DONE == sqlite3_step(read_stmt_));
    } else {
      req.second(kNotFound, Data());
    }

    // Reset statement for next query.
    sqlite3_clear_bindings(read_stmt_);
    sqlite3_reset(read_stmt_);
  }

  for (auto& req : writes) {
    auto& key = req.first;
    auto& val = req.second;
    int result = sqlite3_bind_text(
       write_stmt_, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    ASSERT(result == SQLITE_OK);
    result = sqlite3_bind_blob(
        write_stmt_, 2, &(val[0]), static_cast<int>(val.size()), SQLITE_TRANSIENT);
    ASSERT(result == SQLITE_OK);
    result = sqlite3_step(write_stmt_);
    ASSERT(result == SQLITE_DONE);

    sqlite3_clear_bindings(write_stmt_);
    sqlite3_reset(write_stmt_);
  }

  for (auto& key : erases) {
    int result = sqlite3_bind_text(
       erase_stmt_, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    ASSERT(result == SQLITE_OK);
    result = sqlite3_step(erase_stmt_);
    ASSERT(result == SQLITE_DONE);

    sqlite3_clear_bindings(erase_stmt_);
    sqlite3_reset(erase_stmt_);
  }
}

}  // namespace disk
}  // namespace qi
