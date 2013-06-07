//
//    obj.cc
//    schwa::job01::obj
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//        
//////////////////////////////////////////////////////////////////////////////


#include "job01/obj/obj.h"


// schwa::job01::obj ==========================================================
namespace schwa { namespace job01 { namespace obj {


// Static variable definitions.
ObjMaker::Array ObjMaker::s_arrays[ObjMaker::kMaxArrayCount];
int             ObjMaker::s_array_count = 0;
UnbornQueue     ObjMaker::s_unborn[ObjMaker::kNumObjSizes];
std::mutex      ObjMaker::s_mutex;



// Answer the number of Unborn of the specified size.
int ObjMaker::UnbornCount(ObjSizeCode code) {
    return s_unborn[code].count();
}


// Allocate raw memory for some new Unborn objects,
// and add them to the appropriate free-list.
bool ObjMaker::MakeMoreUnborn(ObjSizeCode size_code) {
    // Find the next Array to allocate.
    if (s_array_count == kMaxArrayCount) {
        // We already have the maximum number of Arrays.
        return false;
    }

    // Allocate memory.
    size_t stride = host::CACHE_LINE_SIZE << size_code;
    Array& array = s_arrays[s_array_count];
    if (!array.Alloc(stride, kMaxArraySize)) {
        // Couldn't allocate memory for some reason.
        return false;
    }

    // Sucessfully allocated memory, now initialize as Unborn.
    uint16_t array_index = s_array_count++;
    auto& unborn_queue = s_unborn[size_code];
    for (int i = 0; i < kMaxArraySize; ++i) {
        // Use placement-new to instantiate new Unborns.
        // Link it to the next one in the array.
        uint8_t* ptr = array._pointer + (i * stride);
        new (ptr) Unborn(size_code, 
                         (i < kMaxArraySize-1)
                            ? UnbornRef(array_index, i+1)
                            : UnbornRef());
    }
    // TODO: this odd way of constructing a queue may be sufficient
    //       reason to swap the meanings of head vs. tail.
    unborn_queue.add(UnbornQueue(UnbornRef(array_index, kMaxArraySize-1),
                                 UnbornRef(array_index, 0),
                                 kMaxArraySize));

    return true;
}


// Set to initial state before use.
void ObjMaker::Array::Init() {
    _pointer = nullptr;
    _stride  = 0;
}


// Helper functions for Alloc and Free.
namespace impl {
typedef std::aligned_storage <host::CACHE_LINE_SIZE,
                              host::CACHE_LINE_SIZE>::type Aligned;
static uint8_t* malloc_cache_aligned(size_t size) {
    size_t cache_lines = size / host::CACHE_LINE_SIZE;
    if (0 != size % host::CACHE_LINE_SIZE) {
        cache_lines++;
    }
    Aligned* ptr = new Aligned[cache_lines];
    return reinterpret_cast<uint8_t*>(ptr);
}
static void free_cache_aligned(void* ptr) {
    delete[] static_cast<Aligned*>(ptr);
}
}  // schwa::job01::obj::impl

// Allocate enough space for "count" objects of size "stride".
// Return false if the memory could not be allocated, true otherwise.
bool ObjMaker::Array::Alloc(size_t stride, size_t count) {
    assert(_pointer == nullptr);
    _stride   = stride;
    _pointer  = impl::malloc_cache_aligned(stride * count);
    return _pointer != nullptr;
}


// Free pointer.
void ObjMaker::Array::Free() {
    assert(_pointer != nullptr);
    impl::free_cache_aligned(_pointer);
    Init();
}


}}}  // schwa::job01::obj =====================================================