#ifndef _QI_DISK_CacheTypes_h_
#define _QI_DISK_CacheTypes_h_

#include "qi/base/types.h"

#include <string>
#include <vector>

namespace qi {
namespace disk {

typedef std::vector<byte> Data;
typedef std::string Key;

enum ReadStatus {
  kSuccess,
  kNotFound
};

}  // namespace disk
}  // namespace qi

#endif   // _QI_DISK_CacheTypes_h_
