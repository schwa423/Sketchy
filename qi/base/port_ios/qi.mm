#include "qi/qi.h"

namespace qi {

std::string Qi::GetDirectoryPath(std::string desc) {
  if (desc == "docs") {
    ASSERT(false);
    return "hello";
  } else {
    return "";
  }
}

}  // namespace qi
