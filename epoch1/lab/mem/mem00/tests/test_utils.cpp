#include "mem00/util.h"
using namespace schwa;

#include <iostream>
using namespace std;

#include <assert.h>


unsigned test_align(unsigned num, size_t alignment) {
    char* ptr = reinterpret_cast<char*>(num);
    ptr = mem00::align(ptr, alignment);
    return reinterpret_cast<unsigned>(ptr);
}


int main(void) {
    for (size_t alignment = 1; alignment <= 16; alignment *= 2) {
        assert(16 == test_align(16, alignment));
    }
    assert(32 == test_align(16, 32));

    for (unsigned ptr = 17; ptr <= 32; ptr++) {
        assert(32 == test_align(ptr,16));
    }

    cerr << "mem00/test_utils...  PASSED!" << endl;
}

