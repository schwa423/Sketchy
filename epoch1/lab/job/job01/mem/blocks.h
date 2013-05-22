//
//    blocks.h
//    schwa::job01::mem
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Support for allocating BlockArrays (arrays of fixed-size Blocks).
//    See test_blocks.cc for a usage example.
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
//    - documentation
//
//    - verify imposibility of user creating a invalid block-ref
//
//    - pretty ostream printing of public types
//
//    - benchmark
//
//    - get rid of Block?
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__mem__blocks__
#define __schwa__job01__mem__blocks__

#include <limits>
#include <cstdint>

#include "job01/core/link.h"
#include "job01/core/meta.h"
#include "job01/core/schwassert.h"
#include "job01/mem/util.h"

#include "job01/mem/impl/blocks_impl.h"


// schwa::job01::mem ==========================================================
namespace schwa { namespace job01 { namespace mem {


// Instantiate a new instance of the specified BlockArray subclass.
template <SCHWA_USE_IF_SUBCLASS(BlockArrayT, impl::BlockArrayImpl)>				        
inline BlockArrayT* Create() {
	return impl::BlockArrayManager::Create<BlockArrayT>();
}


// A strongly-typed reference to an object in a BlockArray.
// Can be compared, assigned, and dereferenced.
template <typename BlockT>
class BlockRef : public impl::BlockRefImpl {
 public:
 	BlockRef() : impl::BlockRefImpl() { }
	BlockRef(nullptr_t ptr) : impl::BlockRefImpl(nullptr) { }

	// Allow dereferencing through ref as though it is a pointer type.
	BlockT* operator->() {
		return static_cast<BlockT*>(GetBlock());
	}

	// Allows using static_cast to transform to raw pointer.
	explicit operator BlockT*() const {
		return static_cast<BlockT*>(GetBlock());
	}

	// BlockRef also inherits the following operators:
	//     bool operator== (nullptr_t p) const
	//     bool operator!= (nullptr_t p) const
	//     bool operator== (const BlockRefImpl& ref) const
	//     bool operator!= (const BlockRefImpl& ref) const
	//     BlockRefImpl& operator=(const BlockRefImpl& ref)

 protected:
 	// Only BlockArrays can create valid/non-null refs to a block.
    friend class impl::BlockArrayImpl; 
 	BlockRef(impl::BlockArrayRef array_ref, uint16_t block_index)
    : impl::BlockRefImpl(array_ref, block_index) { }
};


// BlockArray where the block-type is specified by the user (but
// must be a subclass of Block).  TypedBlockArray is intended to
// satisfy most use-cases, but users are free to implement their
// own subclasses of BlockArray.
// TODO: document differently now that this is no longer called TypedBlockArray.
template <typename BlockType, int NumBlocks = impl::BlockArrayImpl::kMaxBlocks>
class BlockArray : public impl::BlockArrayImpl {
	friend class BlockTests;
	friend class impl::BlockArrayManager;

 public:
	typedef BlockType BlockT;
	typedef BlockRef<BlockT> BlockRefT;

 	// Return a ref to the block at the specified index
	// (or a null-ref if the index is invalid).
	BlockRefT operator[](int32_t block_index) const {
		if (block_index >= 0 && block_index < count) {
			return CreateRef<BlockRefT>(id, block_index);
		} else {
			return BlockRefT(nullptr);
		}
	}

 protected:
 	// "id" is a newly-minted ref to this array.
 	// This is the only way that new BlockArrays are instantiated.
 	BlockArray(const impl::BlockArrayRef& id)
 	: impl::BlockArrayImpl(id, _blocks, NumBlocks,
                           // Cannot use sizeof(BlockT) here due to mem-alignment.
						   PointerDifference(_blocks, _blocks + 1)) {
 	}

    // Raw memory for blocks.
 	BlockT _blocks[NumBlocks];
};


}}}  // schwa::job01::mem =====================================================

#endif  // #ifndef __schwa__job01__mem__blocks__