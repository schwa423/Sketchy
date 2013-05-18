//
//  pool.h
//  schwa::job01::core
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//

/*
How should this be layered with managing free lists?
*/


#ifndef __schwa__job01__core__pool__
#define __schwa__job01__core__pool__

#include <limits>
#include <cstdint>

#include "job01/core/link.h"
#include "job01/core/schwassert.h"


// schwa::job01::core =========================================================
namespace schwa { namespace job01 { namespace core {

using namespace std;


// Opaque handle to a Pool.
// TODO: more explanation of role and responsibility.
// 
// But, why not just "typedef uint8_t PoolID"?
// The aim is to make it difficult for users to shoot themselves in the foot 
// by accidentally creating a bogus PoolID... instead, PoolIDs can only be
// created by friends.
// TODO: This may not be necessary... if the public schwa::job API doesn't
//       include any functions which accept PoolIDs, then no harm can be done
//       by bogus ones (right?).  However, if we want to expose pools as part
//       of schwa::core, something like this will be necessary.
class PoolID {
    friend class Pools;
    friend class PoolObjRef;

 public:
    // PoolID variables can be assigned to.
    PoolID& operator=(const PoolID& other) { _id = other._id; }

 private:
    PoolID(uint8_t id) : _id(id) { }
    operator uint8_t() const { return _id; }

    uint8_t _id;
};


// Base class for all pool-objects.
struct PoolObj {

};


class PoolObjRef {

 public:
 	// Anyone can create a invalid/null ref.  This is handy, for example,
 	// for creating unintialized variables that will later be stored into.
    PoolObjRef() : _pool_id(0), _object_index(kNullObject) { }
    PoolObjRef(nullptr_t n) : _pool_id(0), _object_index(kNullObject) { }

    // Refs can be assigned to each other.
    PoolObjRef& operator=(const PoolObjRef& ref) {
        _pool_id = ref._pool_id;
        _object_index = ref._object_index;        
    }

    // Return raw pointer to the referenced PoolObj (or nullptr).
    // Delegates to Pools::GetObject()... since this hasn't yet been
    // declared, the implementation is found below.
    PoolObj* GetObject() const;

 protected:
 	// Only Pools can create valid/non-null refs to an object.
    friend class Pool; 
 	PoolObjRef(PoolID pool_id, uint16_t obj_index)
    : _pool_id(pool_id), _object_index(obj_index) { }

 private:
    PoolID   _pool_id;
    uint16_t _object_index;

    friend class Pools;  // uses kNullObject.
    static const uint16_t kNullObject = numeric_limits<uint16_t>::max();
};


// Base class for all pools. 
class Pool {
    friend class Pools;

 public:
    PoolID id() const { return _id; }
    virtual ~Pool() { }

 protected:
    // TODO: document 
    Pool(PoolID id, void* objs, size_t obj_size)
    : _id(id), _objects(objs), _object_size(obj_size) { }


    // Defined below, after declaration of Pools.
    template <typename PoolT>
    static PoolT* create();

    const PoolID _id;
    const void*  _objects;
    const size_t _object_size;
};




class Pools {
    friend class Pool;
    friend class PoolObjRef;

 public:
    static const int kMaxPools = numeric_limits<uint8_t>::max() - 1;
    static const int kMaxJobsPerPool = PoolObjRef::kNullObject - 1;


 protected:

    static void reset() {
        // Free all allocated pools.
        for (uint8_t id = 0; id < _num_pools; ++id) {
            delete(_pools[id].ptr);
        }
        _num_pools = 0;
    }

    static PoolObj* GetObject(const PoolObjRef& ref) {
        PoolInfo& pool = _pools[ref._pool_id];
        size_t byte_index = ref._object_index * pool.object_size;
        uint8_t* obj = &(pool.objects[byte_index]);
        return reinterpret_cast<PoolObj*>(obj);
    }


    template <typename PoolT>
    static PoolT* create() {
        // TODO: throw exception if full, or otherwise can't create the pool.        
        SCHWASSERT(_num_pools < kMaxPools, "exceeded maximum number of pools");

        // Create pool with the next available ID.
        PoolID id = _num_pools++;
        PoolT* pool = new PoolT(id);

        // Record the info necessary to find a PoolObj* from only
        // the PoolID and object-index within the bool.  Storing the
        // object-size here avoids pointer-chasing... if we stored it
        // in the Pool header, we would have to read both the cache-lines
        // containing the header and the object.
        PoolInfo& info = _pools[id];
        info.ptr = pool;
        info.objects = 
        	// TODO: Debletcherize... but how?
        	static_cast<uint8_t*>(const_cast<void*>(pool->_objects));
        info.object_size = pool->_object_size;

        // The pool is ready to use!
        return pool;
    }

 private:
    // No constructor: everything is static/global.
    Pools() {}

    struct PoolInfo {
        Pool*            ptr;
        uint8_t*         objects;
        size_t           object_size;
    };

    static PoolInfo _pools[kMaxPools];
    static uint8_t  _num_pools;
};
// class Pools



//
// Implementations of functions which were forward-declared above.
//

// Instantiate a new pool of the specified type.
template <typename PoolT>
inline PoolT* Pool::create() {
    return Pools::create<PoolT>();
}

// Return raw pointer to the referenced PoolObj (or nullptr).
inline PoolObj* PoolObjRef::GetObject() const {
    if (_object_index == kNullObject) {
        return nullptr;  // not a valid ref
    }
    return Pools::GetObject(*this);
}


}}}  // schwa::job01::core ====================================================

#endif  // #ifndef __schwa__job01__core__pool__