#include "qi/base/test/testutils.h"
#include "qi/base/timetypes.h"
#include "qi/disk/test/inmemorycache.h"
#include "qi/disk/sqlitecache.h"

#include "gtest/gtest.h"

namespace qi {
namespace disk {
namespace test {

using namespace qi::test;

TEST(InMemoryCache, ReadWriteErase) {
  Key key_1("key_1");
  Data empty{};
  Data data_1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Data data_2{10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

  auto cache = make_unique<InMemoryCache>();
  auto future = cache->Read(key_1);
  future.wait();
  EXPECT_EQ(empty, future.get());
}

TEST(SqliteCache, ReadWriteErase) {
  Key key_1("key_1");
  Key key_2("key_2");
  Data empty{};
  Data data_1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Data data_2{10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
  Data data_3{20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

  auto cache = make_unique<SqliteCache>();
  auto unblocker = BlockRunnerThreads(cache->GetRunner(), 1);

  // This read cannot occur until the background thread is unblocked.
  auto read_1a = cache->Read(key_1);

  // After writing/erasing, subsequent reads can succeed even though the background
  // thread is blocked, since they check the currently-queued operations.
  cache->Write(key_1, data_1);
  auto read_1b = cache->Read(key_1);
  cache->Write(key_1, data_2);
  auto read_1c = cache->Read(key_1);
  cache->Erase(key_1);
  auto read_1d = cache->Read(key_1);

  // Write and read data for another key.
  cache->Write(key_2, data_3);
  auto read_2a = cache->Read(key_2);

  EXPECT_EQ(data_1, read_1b.get());
  EXPECT_EQ(data_2, read_1c.get());
  EXPECT_EQ(empty, read_1d.get());
  EXPECT_EQ(data_3, read_2a.get());

  // The first read is still waiting for the background thread to be unblocked.
  EXPECT_EQ(std::future_status::timeout, read_1a.wait_for(10ms));

  // After unblocking the background thread, blocking it again will guarantee
  // that DoRunQueries() has run, and therefore the first read will now be
  // resolved.
  unblocker.set_value();
  unblocker = BlockRunnerThreads(cache->GetRunner(), 1);
  EXPECT_EQ(std::future_status::ready, read_1a.wait_for(0ms));
  EXPECT_EQ(empty, read_1a.get());

  // Now that data has been committed to the DB, reads will again need to be
  // scheduled on the background thread.
  auto read_1e = cache->Read(key_1);
  auto read_2b = cache->Read(key_2);

  // However, after other operations are enqueued, reads can succeed even
  // though the background thread is blocked.
  cache->Write(key_1, data_1);
  auto read_1f = cache->Read(key_1);
  cache->Erase(key_2);
  auto read_2c = cache->Read(key_2);

  // Verify the above two sections.
  EXPECT_EQ(data_1, read_1f.get());
  EXPECT_EQ(empty, read_2c.get());
  EXPECT_EQ(std::future_status::timeout, read_1e.wait_for(10ms));
  EXPECT_EQ(std::future_status::timeout, read_2b.wait_for(10ms));
  unblocker.set_value();
  EXPECT_EQ(empty, read_1e.get());
  EXPECT_EQ(data_3, read_2b.get());
}

}  // namespace test
}  // namespace disk
}  // namespace qi
