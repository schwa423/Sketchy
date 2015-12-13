#ifndef _QI_DISK_Cache_h_
#define _QI_DISK_Cache_h_

#include <functional>
#include <string>
#include <vector>

namespace qi {
namespace disk {

enum ReadStatus {
  kSuccess,
  kNotFound
};

// Simple interface for persistent key-value storage.
class Cache {
 public:
  typedef std::vector<uint8_t> Data;
  typedef std::string Key;

  typedef std::function<void(ReadStatus, Data)> ReadCallback;

  // Thread-safe.
  virtual void Read(Key key, ReadCallback callback) = 0;

  // Thread-safe.
  // Size of |data| must be non-zero.
  virtual void Write(Key key, Data data) = 0;

  // Thread-safe.
  virtual void Erase(Key key) = 0;
};

}  // namespace disk
}  // namespace qi

#endif   // _QI_DISK_Cache_h_
