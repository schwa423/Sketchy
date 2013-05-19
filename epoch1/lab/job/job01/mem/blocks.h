//
//    blocks.h
//    schwa::job01::mem
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Support for allocating BlockArrays (arrays of fixed-size Blocks).
//    TODO 
//
//    ==== Usage ====
//  
//
//    ==== Features ====
// 
//    - address blocks via opaque handles, with advantages over raw pointers:
//        - allow later addition of relocatable BlockArrays.
//        - lower memory usage (3 bytes, currently)
//
//    - aspirations to idiot-proof API design
//        - impossible for user to generate invalid block-refs
//
//
//    ==== TODO ====
//
//    - template wizardry to allow coexistence of multiple BlockArrayMangers
//    
//    - generic wrapper for BlockRef to allow customization for various use-cases
//
//    - add an impl namespace to hide unnecessary stuff
//
//    - split into multiple files if it helps clarity.
//
//    - documentation
//
//    - verify imposibility of user creating a invalid block-ref
//
//    - pretty ostream printing of public types
//
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__mem__blocks__
#define __schwa__job01__mem__blocks__

#include <limits>
#include <cstdint>

#include "job01/core/link.h"
#include "job01/core/schwassert.h"
#include "job01/mem/util.h"


// schwa::job01::mem ==========================================================
namespace schwa { namespace job01 { namespace mem {

using namespace std;


// Base class for all blocks.
class Block {

};


// Opaque handle to a BlockArray.
// TODO: more explanation of role and responsibility.
// 
// But, why not just "typedef uint8_t BlockArrayRef"?
// The aim is to make it difficult for users to shoot themselves in the foot 
// by accidentally creating a bogus ref... instead, only friends may create
// BlockArrayRefs.
class BlockArrayRef {
    friend class BlockArrayManager;
    friend class BlockRef;

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
//       - avoid ugliness like JobRef in test_blocks.cc
//       - do away with "class Block" altogether?
class BlockRef {
 public:
 	// Anyone can create a invalid/null ref.  This is handy, for example,
 	// for creating unintialized variables that will later be stored into.
    BlockRef() : _array(0), _block(kNullBlock) { }
    BlockRef(nullptr_t ptr) : _array(0), _block(kNullBlock) { }

    // Refs can be assigned to each other.
    BlockRef& operator=(const BlockRef& ref) {
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
    bool operator== (const BlockRef& ref) const {
    	return _array == ref._array && _block == ref._block
    		? true
    		: _array == kNullBlock && ref._array == kNullBlock;  // both null?  
    }
    bool operator!= (const BlockRef& ref) const {
    	return !(*this == ref);
    }    

    // Return raw pointer to the referenced Block (or nullptr).
    // Delegates to BlockArrayManager::GetBlock()... since this hasn't yet been
    // declared, the implementation is found below.
    Block* GetBlock() const;

 protected:
 	// Only BlockArrays can create valid/non-null refs to a block.
    friend class BlockArray; 
 	BlockRef(BlockArrayRef array_ref, uint16_t block_index)
    : _array(array_ref), _block(block_index) { }

 private:
    BlockArrayRef _array;  // identifies the array which contains the block.
    uint16_t      _block;  // identifies the block within the containing array.

    friend class BlockArrayManager;   // uses kNullBlock.
    static const uint16_t kNullBlock = numeric_limits<uint16_t>::max();
};


// Base class for all block-arrays.
// TODO: document subclass responsibilities.
class BlockArray {
    friend class BlockArrayManager;

 public: // functions

    // Defined below, after declaration of BlockArrayManager.
    template <typename BlockArrayT>
    static BlockArrayT* create();

    // Users who create custom sub-classes may wish to use virtual functions.
    virtual ~BlockArray() { }

    // Return a ref to the block at the specified index
	// (or a null-ref if the index is invalid).
	BlockRef operator[](int block_index) const {
		if (block_index >= 0 && block_index < count) {
			return BlockRef(id, block_index);
		} else {
			return BlockRef(nullptr);
		}
	}

 public:  // variables and constants

    const BlockArrayRef id;           // refers to this BlockArray
    const int           count;        // number of blocks in this BlockArray
    const size_t        stride;       // byte-increment between adjacent Blocks
    Block* const        first_block;  // base address to which index is added

    // Maximum number of blocks that a BlockArray can be instantiated with.
    // The determining factor is the number of bits BlockRef has available
    // to encode the index into the BlockArray.
    static const uint16_t kMaxBlocks = BlockRef::kNullBlock - 1; 

 protected:

    // TODO: document 
    BlockArray(const BlockArrayRef& ref, // newly-minted ref to this array
    		   Block* first,             // first in contiguous array of blocks
    		   int block_count,          // number of blocks in the array 
    		   size_t block_stride)      // stride between consecutive blocks
    : id(ref), count(block_count), stride(block_stride), first_block(first) { 
    	SCHWASSERT(block_stride == 64, "bad stride");
      	SCHWASSERT(count <= kMaxBlocks, "maximum block count exceeded");
    }
};


// BlockArray where the block-type is specified by the user (but
// must be a subclass of Block).  TypedBlockArray is intended to
// satisfy most use-cases, but users are free to implement their
// own subclasses of BlockArray.
template <typename BlockT, int NumBlocks = BlockArray::kMaxBlocks>
class TypedBlockArray : public BlockArray {
	friend class BlockArrayManager;

 protected:
 	// "id" is a newly-minted ref to this array.
 	// This is the only way
 	TypedBlockArray(const BlockArrayRef& id)
 	: BlockArray(id, _blocks, NumBlocks,
 				 // Memory alignment issues prevent using sizeof(BlockT) here.
 				 PointerDifference(_blocks, _blocks + 1)) { }

    // Raw memory for blocks.
 	BlockT _blocks[NumBlocks];
};


// TODO: move to impl namespace
class BlockArrayManager {
    friend class BlockArray;
    friend class BlockRef;

 public:

    static const int kMaxArrays = numeric_limits<uint8_t>::max() - 1;

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
    static Block* GetBlock(const BlockRef& ref) {
    	// Find the BlockArray that the block lives in.
        ArrayInfo& info = _info[ref._array];

        // Compute the byte-offset of the block within the array.
        size_t byte_index = ref._block * info.stride;

        // Compute and return a raw pointer to the block.
        uint8_t* block = &(info.block_mem[byte_index]);
        return reinterpret_cast<Block*>(block);
    }

    // Instantiate a BlockArray of the specified type and size.
    // TODO: throw exception if full, or otherwise unable to create array.        
    template <typename BlockArrayT>
    static BlockArrayT* create() {
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

 private:
 	
    // No constructor: everything is static/global.
    BlockArrayManager() {}

    // TODO: pad to 1/2 cacheline size
    struct ArrayInfo {
        BlockArray* array;
        uint8_t*    block_mem;
        size_t      stride;
    };

    static ArrayInfo _info[kMaxArrays];
    static uint8_t   _count;
};
// class BlockArrayManager



//
// Implementations of functions which were forward-declared above.
//

// Instantiate a new pool of the specified type.
template <typename BlockArrayT>
inline BlockArrayT* BlockArray::create() {
    return BlockArrayManager::create<BlockArrayT>();
}

// Return raw pointer to the referenced Block (or nullptr).
inline Block* BlockRef::GetBlock() const {
    if (_block == kNullBlock) {
        return nullptr;  // not a valid ref
    }
    return BlockArrayManager::GetBlock(*this);
}


}}}  // schwa::job01::mem =====================================================

#endif  // #ifndef __schwa__job01__mem__blocks__