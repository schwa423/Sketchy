#ifndef _QI_DISK_Cache_h_
#define _QI_DISK_Cache_h_

#include "qi/base/runner.h"
#include "qi/disk/cachetypes.h"

#include <functional>
#include <future>
#include <unordered_map>
#include <unordered_set>

namespace qi {
namespace disk {

// Simple interface for persistent key-value storage.
// Public interface is virtual to support mocking in tests.
class Cache {
 public:
  typedef std::function<void(ReadStatus, Data)> ReadCallback;
  Cache();
  virtual ~Cache();

  // Thread-safe.  |callback| may be invoked on another thread.
  virtual void Read(Key key, ReadCallback callback);

  std::future<Data> Read(Key key);

  // Thread-safe.
  // Size of |data| must be non-zero.
  virtual void Write(Key key, Data data);

  // Thread-safe.
  virtual void Erase(Key key);

  // Wait for any scheduled work to be completed.
  void Flush();

  // For debugging.
  IRunner* GetRunner() { return runner_.get(); }

 protected:
  typedef std::pair<Key, ReadCallback> ReadRequest;

   // Subclasses query/mutate the persistent storage in response to the specified
   // read/write/erase requests.  In order to provide consistent results to the
   // user, subclass implementations must perform reads first.  Then, writes and
   // and erases can be performed in any order.
   //
   // Called on a background thread to guarantee that there are never multiple
   // simultaneous invocations.
   virtual void RunQueries(std::vector<ReadRequest> reads,
                           std::unordered_map<Key, Data> writes,
                           std::unordered_set<Key> erases) = 0;

 private:
  // Called by Read()/Write()/Erase() to schedule work on a background thread.
  void DoRunQueries();

  std::unique_ptr<IRunner> runner_;
  std::mutex mutex_;
  std::vector<ReadRequest> reads_;
  std::unordered_map<Key, Data> writes_;
  std::unordered_set<Key> erases_;
  bool work_scheduled_;
  // TODO: consider having |shutdown_|, and failing reads/etc. appropriately.
};

}  // namespace disk
}  // namespace qi

#endif   // _QI_DISK_Cache_h_
