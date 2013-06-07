//
//    obj_defs.h
//    schwa::job01::obj
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Defines a trio of classes which work closely together:
//      - Obj
//      - ObjRef
//      - ObjMaker
//
//    Obj is a virtual base-class... MORE TO FOLLOW.
//
//
//
//    TODO:
//      - more documentation
//
///////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__obj__obj_defs__________________________________________
#define __schwa__job01__obj__obj_defs__________________________________________


// Allocated objects are one of a handful of fixed sizes.  Two bits in the
// object header record the size, to facilitate finding the correct free-list
// to store the object for later reuse (this could be accomplished without
// explicitly storing size information, but it would be error-prone, and not
// amenable to sanity-checking).
enum ObjSizeCode { k64Bytes = 0, k128Bytes, k256Bytes, k512Bytes };


//  A reference to an object which trades off a small amount of performance
//  compared to a raw pointer, but in return gains a number of advantages.
//  The main ones are:
//
//    - relocatable objects: a pool of objects can be relocated in memory,
//        yet any refs to these objects will remain valid.  The caveat is that
//        that the programmer must ensure that they are not dereferenced,
//        lest undefined behavior occur.
//
//    - small size: 3 bytes is typical, but can vary depending on how each type
//        of ref is parameterized or composed with other refs.  This will be
//        particularly useful on upcoming 64-bit mobile devices... you can't
//        fit too many 8-byte pointers to other objects in a cacheline-sized
//        (i.e. 64 byte) object with a 16-byte header (8 for the pointer to the
//        virtual function table, and 8 for an intrusive link pointer).
template <typename ObjT>
class ObjRef {
    friend class ObjMaker;

 public: 
    // The only public constructors make a null-ref...
    // only ObjMaker can create valid references to an Obj.
    ObjRef() : _array(0), _obj(kNull) { }
    ObjRef(std::nullptr_t ptr) : _array(0), _obj(kNull) { }

    // Obtain a raw ObjT* pointer to the referenced object...
    ObjT* ptr();
    // ... or a const one...
    const ObjT* ptr() const;
    // ... or do the same things with operator->
    // NOTE: I'd rather use ptr() explicitly, but there are some
    //       places, such as core::Queue, which are designed to 
    //       use both raw pointers, and pointer-like refs.
    ObjT* operator->() { return ptr(); }
    const ObjT* operator->() const { return ptr(); }    

    // Assign another ref to this one (so they both refer to the same Obj)...
    ObjRef& operator=(const ObjRef& other);
    // ...or reset it to be a null-ref.
    ObjRef& operator=(std::nullptr_t ptr);

    // Do the two refs refer to the same object?
    bool operator==(const ObjRef<ObjT>& other) const;
    // Are they different?
    bool operator!=(const ObjRef<ObjT>& other) const;
    // Is the ref a null-ref?
    bool operator==(std::nullptr_t ptr) const;
    bool operator!=(std::nullptr_t ptr) const;


 private:
    uint8_t  _array;  // Index of memory-block the object resides in.
    uint16_t _obj;    // Index of object within the memory block.
    // Called by ObjMaker
    ObjRef(uint8_t ar, uint16_t obj) : _array(ar), _obj(obj) { }    

    enum { kNull = std::numeric_limits<uint16_t>::max() };
};


// Base class for objects which:
// - have an intrusive ref to link to other objects
// - TODO: more to follow
template <typename ObjT>
class Obj : public core::Link<ObjT, ObjRef<ObjT>> {
    friend class Unborn;

 public:
    enum Status { kUnborn, kPool, kAlloced, kReservedStatus };

    virtual ~Obj() { _status = kPool; }

    // Required in order to act as a core::Link LinkT.
    // TODO: can't we make Link do this for us?
    // TODO: rename to ref_type
    typedef ObjRef<ObjT> Ptr;

 protected:
    Obj() : _status(kAlloced) { }

 private:
    typedef core::Link<ObjT, Ptr> LinkT;
    Obj(ObjSizeCode code, Status status, Ptr next) 
        : LinkT(next), _status(status), _size_code(code) { }

    ObjSizeCode    _size_code  : 2;
    Status         _status     : 2;
    unsigned       _reserved   : 4;
};


// TODO: put into namespace impl?  Or do we want to make an
//       ObjPool<ObjT> type that would also use Unborn internally?
//       (even if we add ObjPool<>, Unborn might still belong in ::impl)
class Unborn : public Obj<Unborn> {
    friend class ObjMaker;
 private:

    Unborn(ObjSizeCode code, ObjRef<Unborn> ref) : Obj<Unborn>(code, kUnborn, ref) { }

    // TODO: not used yet, but will be as soon as
    //       ObjMaker::UnmakeObjects() is implemented
    Unborn() { _status = kUnborn; }
};

typedef ObjRef<Unborn> UnbornRef;


// Would like to use a simple typedef, except we need to expose the
// core::Queue constructor which takes head/tail/count args.
// TODO: perhaps make this constructor public in core::Queue.
class UnbornQueue : public core::Queue<Unborn> {
 public:    
    UnbornQueue() : core::Queue<Unborn>() { }

    UnbornQueue(UnbornRef head, UnbornRef tail, int count)
        : core::Queue<Unborn>(head, tail, count) { }
};


// ObjMaker is responsible for allocation/deallocation of Objs, and for
// dereferencing ObjRefs.  These capabilities are provided by static functions
// and variables, which enables more efficient implementation of the Lookup()
// function.
//
// ObjMaker is therefore not intended to be instantiated directly, but it is
// permitted to have instantiable subclasses.
//
// ObjMaker's public interface is thread-safe. 
class ObjMaker {
 public:
    // Obtain raw pointer (const or non-const) to the referenced object.
    // These are helper functions used by ObjRef<>::ptr().  It's OK for
    // them to be public, since if you have an ObjRef, you can just call
    // ptr() on it directly.
    template <typename ObjT>
    static const ObjT* Lookup(const ObjRef<ObjT>& ref);
    template <typename ObjT>
    static ObjT* Lookup(ObjRef<ObjT>& ref);

    // Answer the number of Unborn of the specified size.
    static int UnbornCount(ObjSizeCode code);

 protected:
    // ObjMaker should not be instantiated directly,
    // but instantiable subclasses are allowed.
    ObjMaker() { }

    // Return a chain of objects of the specified type.
    // If there are not enough unborn objects of the correct size,
    // allocate more raw memory to create new ones.
    //
    // NOTE: This function is protected since it has the power to
    //       create any kind of objects... we don't want to give
    //       this power to everyone.  Instead, subclasses can 
    //       provide more limited access to this functionality.
    template <typename ObjT>
    static core::Queue<ObjT> MakeObjects(int desiredCount);

    // Return objects to their maker.
    //
    // NOTE: not yet implemented.
    template <typename ObjT>
    static void UnmakeObjects(core::Queue<ObjT>& objects);

 private:
    // Allocate raw memory for some new Unborn objects,
    // and add them to the appropriate free-list.
    static bool MakeMoreUnborn(ObjSizeCode code);

    // Holds information used to dereference an ObjRef.
    struct Array {
        uint8_t* _pointer;
        size_t   _stride;

        // Set to initial state before use.
        void Init();

        // Allocate enough space for "count" objects of size "stride".
        // Return false if the memory could not be allocated, true otherwise.
        bool Alloc(size_t stride, size_t count);

        // Free the pointer.
        void Free();
    };

    // TODO: un-hardwire kNumObjSizes and kMaxArrayCount.
    //       The former corresponds to the 2-bit ObjSizeCode,
    //       and the latter is what fits in a uint8_t.
    enum { kNumObjSizes = 4,
           kMaxArrayCount = 256,
           kMaxArraySize = UnbornRef::kNull };


    static Array         s_arrays[kMaxArrayCount];
    static int           s_array_count;
    static UnbornQueue   s_unborn[kNumObjSizes];
    static std::mutex    s_mutex;

    typedef std::lock_guard<std::mutex> lock_guard;

    // Compile-time computation of size-code for the specified object-type.
    template <typename ObjT>
    static constexpr ObjSizeCode SizeCodeFor(SCHWA_ONLY_IF_SIZE_BETWEEN(ObjT,
                                                                        0,
                                                                        64)) {
        return k64Bytes;
    }
    template <typename ObjT>
    static constexpr ObjSizeCode SizeCodeFor(SCHWA_ONLY_IF_SIZE_BETWEEN(ObjT,
                                                                        64,
                                                                        128)) {
        return k128Bytes;
    }
    template <typename ObjT>
    static constexpr ObjSizeCode SizeCodeFor(SCHWA_ONLY_IF_SIZE_BETWEEN(ObjT,
                                                                        128,
                                                                        256)) {
        return k256Bytes;
    }
    template <typename ObjT>
    static constexpr ObjSizeCode SizeCodeFor(SCHWA_ONLY_IF_SIZE_BETWEEN(ObjT,
                                                                        256,
                                                                        512)) {
        return k512Bytes;
    }

};




// ObjRef DEFINITIONS /////////////////////////////////////////////////////////

// Obtain a raw ObjT* pointer to the referenced object...
template <typename ObjT>
inline const ObjT* ObjRef<ObjT>::ptr() const {
    return ObjMaker::Lookup(*this);
}
// ... or a const one.
template <typename ObjT>
inline ObjT* ObjRef<ObjT>::ptr() {
    return ObjMaker::Lookup(*this);
}


// Assign another ref to this one (so they both refer to the same Obj)...
template <typename ObjT>
inline ObjRef<ObjT>& ObjRef<ObjT>::operator=(const ObjRef<ObjT>& other) {
    _array = other._array;
    _obj   = other._obj;
    return *this;
}
// ...or reset it to point at nullptr.
template <typename ObjT>
inline ObjRef<ObjT>& ObjRef<ObjT>::operator=(std::nullptr_t ptr) {
    _array = 0;
    _obj = kNull;
    return *this;
}


// Do the two refs refer to the same object?
template <typename ObjT>
inline bool ObjRef<ObjT>::operator==(const ObjRef<ObjT>& other) const {
    return _obj == other._obj && _array == other._array;
}
// Are they different?
template <typename ObjT>
inline bool ObjRef<ObjT>::operator!=(const ObjRef<ObjT>& other) const {
    return _obj != other._obj || _array != other._array;
}
// Is the ref a null-ref?
template <typename ObjT>
inline bool ObjRef<ObjT>::operator==(std::nullptr_t ptr) const {
    return _obj == kNull;
}
template <typename ObjT>
inline bool ObjRef<ObjT>::operator!=(std::nullptr_t ptr) const {
    return _obj != kNull;
}




// ObjMaker DEFINITIONS ///////////////////////////////////////////////////////

// Obtain raw pointer (const or non-const) to the referenced object.
// These are helper functions used by ObjRef<>::ptr().  It's OK for
// them to be public, since if you have an ObjRef, you can just call
// ptr() on it directly.
template <typename ObjT>
const ObjT* ObjMaker::Lookup(const ObjRef<ObjT>& ref) {
    if (ref._obj == ObjRef<ObjT>::kNull)
        return nullptr;
    Array& array = s_arrays[ref._array];
    uint8_t* ptr = array._pointer + (array._stride * ref._obj);
    return reinterpret_cast<ObjT*>(ptr);
}
template <typename ObjT>
ObjT* ObjMaker::Lookup(ObjRef<ObjT>& ref) {
    if (ref._obj == ObjRef<ObjT>::kNull)
        return nullptr;
    Array& array = s_arrays[ref._array];
    uint8_t* ptr = array._pointer + (array._stride * ref._obj);
    return reinterpret_cast<ObjT*>(ptr);
}


// Return a chain of objects of the specified type.
// If there are not enough unborn objects of the correct size,
// allocate more raw memory to create new ones.
template <typename ObjT>
core::Queue<ObjT> ObjMaker::MakeObjects(int desiredCount) {
    // Ensure we don't accidentally make a non-Obj.
    static_assert(std::is_base_of<Obj<ObjT>, ObjT>::value,
                  "ObjT must be a sublass of Obj<ObjT>");

    lock_guard lock(s_mutex);

    ObjSizeCode size_code = SizeCodeFor<ObjT>();
    assert(size_code < kNumObjSizes);

    // Get the free-list of unborn objects of the correct size.
    auto& unborn = s_unborn[size_code];

    // If there aren't enough unborn objects to meet the demand,
    // so make some more until there are.
    while (unborn.count() < desiredCount) {
        if (!MakeMoreUnborn(size_code)) {
            // We couldn't make any more unborn objects, so continue on...
            // we'll have to make do with what we have.
            assert(false);
            break;
        }
    }

    // Determine how many object we will actually make...
    // we'll return the desired number unless we can't.
    int returnedCount = (unborn.count() < desiredCount)
                                            ? unborn.count()
                                            : desiredCount;

    // Collect a list of newly-constructed ObjT.
    core::Queue<ObjT> result;
    UnbornRef ref;
    for (int i = 0; i < returnedCount; ++i) {
        // Get a reference to the next unborn object.
        ref = unborn.next();
        // Dereference it to obtain a raw pointer.
        Unborn* ptr = ref.ptr();
        // Use placement-new to create an ObjT in its place.
        // NOTE: there no need to call ~Unborn() since it is a no-op.
        new (ptr) ObjT();
        // The reference now points to an ObjT instead of an Unborn,
        // but it doesn't know that.  Force it to recognize its new
        // type, and add it to the result-queue.
        result.add(*reinterpret_cast<ObjRef<ObjT>*>(&ref));
    }

    return result;
}

#endif  // __schwa__job01__obj__obj_defs_______________________________________
