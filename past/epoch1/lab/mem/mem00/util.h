// Josh Gargus 2013

#ifndef __schwa__mem00__util__
#define __schwa__mem00__util__

#include <cstddef>

// schwa::mem01 ===============================================================
namespace schwa { namespace mem00 {


char* align(const char* ptr, size_t alignment) {
    const size_t arg = reinterpret_cast<size_t>(ptr);

    size_t offset = (alignment - (arg % alignment)) % alignment;
    return const_cast<char*>(ptr + offset);
}


}}  // schwa::mem01 ===========================================================

#endif  // #ifndef __schwa__mem00__util__

