#pragma once
#include <stdint.h>
#include <memory>
#include <iostream>
#include "../debug/debug.h"
#include <type_traits>
namespace T5M::Memory {
	//Block Size suggestions
	enum class BlockSize : size_t {
		Minimal = 4,
		Tiny = 8,
		Small = 16,
		Medium = 32,
		Big = 64,
		Huge = 128,
	};

	//A multi purpose Memory pool
	template<unsigned BlkSz>
	class Pool {
		static_assert(BlkSz > 0 && (BlkSz & (BlkSz - 1)) == 0,"Block Size needs to be PoT!");
		using Size = size_t;
		using Byte = uint8_t;
		using Magic = uint32_t;
		//Block representation

		struct Block {
			Byte data[static_cast<Size>(BlkSz)];
		};
		// Block Header Data. Contains either Data about Free Blocks in the case of Free
		// Or Number of Blocks that were allocated in the case of Used
		union BlockHeader;
		struct BlockHeaderData {
			Magic magic;
			//pointer to the next BlockHeader, nullptr if tail
			BlockHeader* nextFreeBlock;
			//pointer to the previous BlockHeader, nullptr if head
			BlockHeader* previousFreeBlock;
			//Number of Free Blocks / Used Blocks
			Size managedBlocks;
			//Whether this Header is Free or Used
			bool isFree;
			//pointer to the next physical block header
			BlockHeader* nextPhysicalBlock;
			//pointer to the previous physical block header
			BlockHeader* previousPhysicalBlock;
			//pointer to first physical block header
			BlockHeaderData() : magic(MAGIC), nextFreeBlock(nullptr), previousFreeBlock(nullptr), managedBlocks(0), nextPhysicalBlock(nullptr), previousPhysicalBlock(nullptr), isFree(false) {}
		};
		union BlockHeader {

			BlockHeaderData data;
			//Padding to ensure BlockHeader is always the next multiple of BlkSz blocks large
			Block padding[getBlockCount(sizeof(BlockHeaderData))];
			BlockHeader() : data(BlockHeaderData()) {};
		};

	private:
		static constexpr Magic MAGIC = 0xDEADBEEF;
		Size allocatedBlockCount;
		Size allocatedBytesCount;
		const std::unique_ptr<Block> allocatedBlocks;
		BlockHeader* head;
		BlockHeader* firstPhysicalBlock;
		void initialize() {
			std::memset(allocatedBlocks.get(), 0, allocatedBytesCount);
			// Place initial BlockHeader at the beginning of all blocks
			head = new(allocatedBlocks.get())BlockHeader();
			head->data.nextFreeBlock = nullptr;
			head->data.managedBlocks = allocatedBlockCount - getHeaderBlockCount();
			head->data.previousFreeBlock = nullptr;
			head->data.nextPhysicalBlock = nullptr;
			head->data.previousPhysicalBlock = nullptr;
			this->firstPhysicalBlock = head;
		}


	public:
		Pool(Size numBlocks) : allocatedBlockCount(numBlocks), allocatedBytesCount(sizeof(Block)* numBlocks) , allocatedBlocks(new Block[numBlocks]) {
			initialize();
			logD("Pool Init: Initialized Pool with ", numBlocks ," Blocks");
			logD("Pool Init: Free Blocks :" , head->data.managedBlocks);
		}
	private:
		//Returns the number of Blocks needed for the number of Bytes
		static constexpr Size getBlockCount(Size bytes) {
			return (bytes + (static_cast<Size>(BlkSz) - 1)) / static_cast<Size>(BlkSz);
		}

		constexpr Size getHeaderBlockCount() const {
			return getBlockCount(sizeof(BlockHeader));
		};
		BlockHeader*& getFreeListHead() {
			return head;
		}
		//try coalescing with the next block in the list
		void tryCoalesce(BlockHeader* origin) {
			BlockHeader* left = origin->data.previousPhysicalBlock;
			BlockHeader* right = origin->data.nextPhysicalBlock;
			if (right != nullptr && right->data.isFree)
				coalesce(origin, right);
			if (left != nullptr && left->data.isFree)
				coalesce(left, origin);
		}

		void coalesce(BlockHeader* left, BlockHeader* right) {
			logD("Coalescing BlockHeader at " , left , " & " , right);
			left->data.managedBlocks += right->data.managedBlocks;
			left->data.managedBlocks += getHeaderBlockCount();
			left->data.nextFreeBlock = right->data.nextFreeBlock;
			left->data.nextPhysicalBlock = right->data.nextPhysicalBlock;
			if (right->data.nextFreeBlock != nullptr) {
				right->data.nextFreeBlock->data.previousFreeBlock = left;
				right->data.nextPhysicalBlock->data.previousPhysicalBlock = left;
			}
		}
	public:

		template <typename T>
		void free(T* ptr) noexcept {
			assertion((static_cast<void*>(ptr) >= allocatedBlocks.get() && static_cast<void*>(ptr) < (allocatedBlocks.get() + allocatedBlockCount)), "memory must be in Pool range!");
			ptr->~T();
			Block* block = reinterpret_cast<Block*>(ptr);
			BlockHeader* blockHead = getBlockHeaderFromBlock(block);
			if (blockHead->data.magic == MAGIC) {
				logD("Free: BlockHeader at ", blockHead);
				BlockHeader* oldHead = head;
				BlockHeader* newHead = blockHead;
				head = newHead;
				head->data.isFree = true;
				head->data.nextFreeBlock = oldHead;
				oldHead->data.previousFreeBlock = head;
				//Now try to coalesce with the next and previous physical block
				tryCoalesce(blockHead);
				tryCoalesce(oldHead);
			}
		}

		template <typename T,typename ... Args>
		[[nodiscard]]T* malloc(Size count = 1,Args&&...args) noexcept {
			if (count < 1) return nullptr;

			Size requestedBlocks = getBlockCount(sizeof(T) * count);
			logD("Malloc: Requested Blocks for " , typeid(T).name() , " x " , count , ": " , requestedBlocks);
			BlockHeader* currentFreeNode = getFreeListHead();

			while (currentFreeNode != nullptr && currentFreeNode->data.isFree && currentFreeNode->data.managedBlocks < requestedBlocks+getHeaderBlockCount()) {
				assertion(currentFreeNode->data.magic == MAGIC, "Pool is corrupt");
				currentFreeNode = currentFreeNode->data.nextFreeBlock;
			}
			assertion(currentFreeNode != nullptr, "Pool is full");
			Block* blockToReturn = getFirstManagedBlock(currentFreeNode);
			logD("Malloc: Return block at " , blockToReturn);
			splitBlocks(currentFreeNode, requestedBlocks, blockToReturn + requestedBlocks + 1);
			return new(blockToReturn)T(std::forward<Args>(args)...);
		}
		private:
		void splitBlocks(BlockHeader*& currentFreeNode, size_t& requestedBlocks, Block* whereToSplit) {
			Size numFreeBlocksAfterSplit = currentFreeNode->data.managedBlocks - getHeaderBlockCount();
			currentFreeNode->data.managedBlocks = requestedBlocks;
			currentFreeNode->data.isFree = false;

			Block* splitBlock = whereToSplit;
			placeNewFreeHead(splitBlock, currentFreeNode, numFreeBlocksAfterSplit);
			logD("Malloc: New Free Blocks : " , numFreeBlocksAfterSplit);
		}

		void placeNewFreeHead(Block*& position,BlockHeader*& oldFreeBlock, size_t& numManagedBlocks) {
			BlockHeader* newHead = new(position)BlockHeader();
			logD("Malloc: New Head at " , newHead);
				newHead->data.previousPhysicalBlock = oldFreeBlock;
			BlockHeader* physicalRightBlock = oldFreeBlock->data.nextPhysicalBlock;
			newHead->data.nextPhysicalBlock = physicalRightBlock;
			if(physicalRightBlock != nullptr)
				physicalRightBlock->data.previousPhysicalBlock = newHead;
			oldFreeBlock->data.nextPhysicalBlock = newHead;
			newHead->data.isFree = true;
			head = newHead;
			head->data.nextFreeBlock = oldFreeBlock->data.nextFreeBlock;
			head->data.managedBlocks = numManagedBlocks;
		}


		Block* getFirstManagedBlock(BlockHeader * header) const {
			return reinterpret_cast<Block*>(header + 1);
		}

		BlockHeader* getBlockHeaderFromBlock(Block* block) const {
			return (reinterpret_cast<BlockHeader*>(block)) - 1;
		}

		void reset() {
			initialize();
		}
	};
}