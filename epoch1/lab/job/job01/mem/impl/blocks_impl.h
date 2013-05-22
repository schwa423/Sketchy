//
//    blocks_impl.h
//    schwa::job01::mem::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Parts of Block/BlockArray implementation which are not for public use.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__mem__blocks__impl__
#define __schwa__job01__mem__blocks__impl__


// schwa::job01::mem ==========================================================
namespace schwa { namespace job01 { namespace mem {
using std::nullptr_t;


// schwa::job01::mem::impl ====================================================
namespace impl {
using namespace std;


// Opaque handle to a BlockArray.
// TODO: more explanation of role and responsibility.
// 
// But, why not just "typedef uint8_t BlockArrayRef"?
// The aim is to make it difficult for users to shoot themselves in the foot 
// by accidentally creating a bogus ref... instead, only friends may create
// BlockArrayRefs.
class BlockArrayRef {
    friend class BlockArrayManager;
    friend class BlockRefImpl;

 public:
    // BlockArrayRef variables can be assigned to.
    BlockArrayRef& operator=(const BlockArrayRef& other) {
    	_index = other._index;
    }

 private:
    BlockArrayRef(uint8_t index) : _index(index) { }
    operator uint8_t() const { return _index; }

    uint8_t _index;
};


// Reference which can be used to obtain a raw pointer to a Block.
// TODO: generic wrapper to adapt to types other than Block...
//       - do away with "class Block" altogether?
class BlockRefImpl {
 public:
 	// Anyone can create a invalid/null ref.  This is handy, for example,
 	// for creating unintialized variables that will later be stored into.
    BlockRefImpl() : _array(0), _block(kNullBlock) { }
    BlockRefImpl(nullptr_t ptr) : _array(0), _block(kNullBlock) { }

    static const uint16_t kNullBlock = numeric_limits<uint16_t>::max();

    // Refs can be assigned to each other.
    BlockRefImpl& operator=(const BlockRefImpl& ref) {
        _array = ref._array;
        _block = ref._block;
        return *this;   
    }

    // Comparison with null.
    bool operator== (nullptr_t p) const {
        return _block == kNullBlock;
    }
    bool operator!= (nullptr_t p) const {
        return _block != kNullBlock;
    }

    // Comparison with other refs.
    bool operator== (const BlockRefImpl& ref) const {
    	return _array == ref._array && _block == ref._block
    		? true
    		: _array == kNullBlock && ref._array == kNullBlock;  // both null?  
    }
    bool operator!= (const BlockRefImpl& ref) const {
    	return !(*this == ref);
    }    

 protected:
 	// Only BlockArrayImpls can create valid/non-null refs to a block.
    friend class BlockArrayImpl; 
 	BlockRefImpl(BlockArrayRef array_ref, uint16_t block_index)
    : _array(array_ref), _block(block_index) { }

 protected:
 	uint8_t		  _reserved;  // we have an extra byte... reserve for later.
    BlockArrayRef _array;     // identifies the array which contains the block.
    uint16_t      _block;     // identifies the block within the containing array.

 protected:
    // Return raw pointer to the referenced Block (or nullptr).
    // Delegates to BlockArrayManager::GetBlock()... since this hasn't yet been
    // declared, the implementation is found below.
    void* GetBlock() const;

    friend class BlockArrayManager;
}; // class BlockRefImpl

// Current design intends BlockRefImpl to be 4 bytes in size.  We only
// use/need 3 for addressing, but since these will probably alignof() == 4
// anyway, we explicitly reserve one byte for unspecified future use.
static_assert(sizeof(BlockRefImpl) == 4, "ref is wrong size");


// Base class for all block-arrays.
// TODO: document subclass responsibilities.
class BlockArrayImpl {

    friend class BlockArrayManager;

 public: 
    // Users who create custom sub-classes may wish to use virtual functions.
    virtual ~BlockArrayImpl() { }

    // TODO: this can probably be deleted, right?
    // Return a ref to the block at the specified index
	// (or a null-ref if the index is invalid).
	BlockRefImpl operator[](int block_index) const {
		if (block_index >= 0 && block_index < count) {
			return BlockRefImpl(id, block_index);
		} else {
			return BlockRefImpl(nullptr);
		}
	}

 public:
    const BlockArrayRef id;           // refers to this BlockArray
    const int           count;        // number of blocks in this BlockArray
    const size_t        stride;       // byte-increment between adjacent Blocks
    void* const         first_block;  // base address to which index is added

    // Maximum number of blocks that a BlockArray can be instantiated with.
    // The determining factor is the number of bits BlockRefImpl has available
    // to encode the index into the BlockArray.
    static const uint16_t kMaxBlocks = BlockRefImpl::kNullBlock - 1; 

 protected:
    // TODO: document 
    BlockArrayImpl(const BlockArrayRef& ref, // newly-minted ref to this array
    		       void* first,              // first in contiguous array of blocks
    		       int block_count,          // number of blocks in the array 
    		       size_t block_stride)      // stride between consecutive blocks
    : id(ref), count(block_count), stride(block_stride), first_block(first) { 
    	SCHWASSERT(block_stride == 64, "bad stride");
      	SCHWASSERT(count <= kMaxBlocks, "maximum block count exceeded");
    }

    // Allow subclasses to create specialized BlockRefs without needing to be
    // a friend of that type of BlockRef.
    template <typename BlockRefT>
    BlockRefT CreateRef(BlockArrayRef array_ref, uint16_t block_index) const {
    	return BlockRefT(array_ref, block_index);
    }
};


// TODO: move to impl namespace
class BlockArrayManager {
    friend class BlockArrayImpl;
    friend class BlockRefImpl;

 public:
    static const int kMaxArrays = numeric_limits<uint8_t>::max() - 1;

    // Instantiate a BlockArray of the specified type and size.
    // TODO: throw exception if full, or otherwise unable to create array.        
    template <typename BlockArrayT>
    static BlockArrayT* Create() {
        SCHWASSERT(_num_pools < kMaxArrays, "exceeded maximum number of arrays");

        // Create pool with the next available ID.
        BlockArrayRef id = _count++;
        BlockArrayT* array = new BlockArrayT(id);

        // Record the info necessary to find a Block* from only
        // the BlockArrayRef and object-index within the bool.  Storing the
        // object-size here avoids pointer-chasing... if we were to obtain it
        // from an instance-variable in Block, we would have to read both that
        // cache-line, and the one(s) containing the block.  Instead, we get it
        // for free from the same cache-line containing the "block_mem" address.
        ArrayInfo& info = _info[id];
        info.array = array;
        info.block_mem = reinterpret_cast<uint8_t*>(array->first_block);
        info.stride = array->stride;

        // The BlockArray is ready to use!
        return array;
    }

 protected:
    // Free all allocated BlockArrays.  You must ensure that existing BlockRefs
    // are never again dereferenced, lest madness and chaos erupt.
    static void reset() {
        for (uint8_t id = 0; id < _count; ++id) {
            delete(_info[id].array);
        }
        _count = 0;
    }

    // Return a raw pointer to the specified block.  No bounds-checking, 
 	// in order to minimize the overhead of BlockRef compared to raw pointers.
    static void* GetBlock(const BlockRefImpl& ref) {
    	// Find the BlockArray that the block lives in.
        ArrayInfo& info = _info[ref._array];

        // Compute the byte-offset of the block within the array.
        size_t byte_index = ref._block * info.stride;

        // Compute and return a raw pointer to the block.
        uint8_t* block = &(info.block_mem[byte_index]);
        return block;
    }

 private:
    // No constructor: everything is static/global.
    BlockArrayManager() {}

    // TODO: pad to 1/2 cacheline size?
    struct ArrayInfo {
        BlockArrayImpl* array;
        uint8_t*        block_mem;
        size_t          stride;
    };

    static ArrayInfo _info[kMaxArrays];
    static uint8_t   _count;
};
// class BlockArrayManager


//
// Implementations of functions which were forward-declared above.
//

// Return raw pointer to the referenced Block (or nullptr).
inline void* BlockRefImpl::GetBlock() const {
    if (_block == kNullBlock) {
        return nullptr;  // not a valid ref
    }
    return BlockArrayManager::GetBlock(*this);
}


}}}}  // schwa::job01::mem::impl ==============================================

#endif  // #ifndef __schwa__job01__mem__blocks__impl__