#ifndef POOL_H
#define POOL_H
#include <stdint.h>
#include <memory>
#include <iostream>
namespace T5M::Memory {
	enum class BlockSize : size_t {
		Minimal = 4,
		Tiny = 8,
		Small = 16,
		Medium = 32,
		Big = 64,
		Huge = 128,
	};

	//A multi purpose Memory pool
	template<BlockSize BlkSz>
	class Pool {

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
			BlockHeaderData() : magic(MAGIC), nextFreeBlock(nullptr), previousFreeBlock(nullptr), managedBlocks(0), nextPhysicalBlock(nullptr), previousPhysicalBlock(nullptr) {}
		};
		union BlockHeader {

			BlockHeaderData data;
			//Padding to ensure BlockHeader is always the next multiple of BlkSz blocks large
			Block padding[(((static_cast<Size>(BlkSz) + sizeof(BlockHeaderData) - 1) / sizeof(BlockHeaderData)) * sizeof(BlockHeaderData)) / sizeof(Block)];
			BlockHeader() : data(BlockHeaderData()) {};
		};

	private:
		static constexpr Magic MAGIC = 0xDEADBEEF;
		Size allocatedBlockCount;
		const std::unique_ptr<Block> allocatedBlocks;
		BlockHeader* head;
		BlockHeader* firstPhysicalBlock;
		void initialize() {
			// Place initial BlockHeader at the beginning of all blocks
			head = new(allocatedBlocks.get())BlockHeader();
			head->data.nextFreeBlock = nullptr;
			head->data.managedBlocks = allocatedBlockCount - blockHeaderBlockSize();
			head->data.previousFreeBlock = nullptr;
			head->data.nextPhysicalBlock = nullptr;
			head->data.previousPhysicalBlock = nullptr;
			this->firstPhysicalBlock = head;
		}


	public:
		Pool(Size numBlocks) : allocatedBlockCount(numBlocks), allocatedBlocks(new Block[numBlocks]{ 0 }) {
			initialize();
		}
	private:
		Size blockHeaderBlockSize() const {
			return sizeof(BlockHeader) / static_cast<Size>(BlkSz);
		};


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
			//accumulate free blocks + now obsolete header data
			left->data.managedBlocks += right->data.managedBlocks;
			left->data.managedBlocks += blockHeaderBlockSize();
			//relink so left points to right->next
			left->data.nextFreeBlock = right->data.nextFreeBlock;
			left->data.nextPhysicalBlock = right->data.nextPhysicalBlock;

			//relink so right->next->previous points to left
			if (right->data.nextFreeBlock != nullptr) {
				right->data.nextFreeBlock->data.previousFreeBlock = left;
			}
		}
	public:
		template <typename T>
		T* malloc(Size count = 1) {

			//Forbid allocation of size 0
			if (count < 1) return nullptr;
			// calculate the amount of blocks we want
			Size requestedBlocks = 1 + (((sizeof(T) * count) - 1) / static_cast<Size>(BlkSz));
			BlockHeader* currentNode = head;
			//Make sure we have enough space for one Block Header and one free Block!
			while (currentNode != nullptr && currentNode->data.managedBlocks < requestedBlocks) {
				if (currentNode->data.magic != MAGIC)
					throw std::runtime_error("Pool is corrupt!");
				currentNode = currentNode->data.nextFreeBlock;
			}
			//We could not find a block of suitable size
			if (currentNode == nullptr)
				return nullptr;
			//Free Blocks are directly behind BlockHeader
			Block* blockToReturn = reinterpret_cast<Block*>(currentNode + 1);
			//The current node will be removed from Free List, so keep the number of Blocks that will be deallocated for later!
			Size numFreeBlocksAfterSplit = currentNode->data.managedBlocks - ((blockHeaderBlockSize()));
			currentNode->data.managedBlocks = requestedBlocks;
			currentNode->data.isFree = false;
			Block* splitBlock = blockToReturn + requestedBlocks + 1;
			//Place a new BlockHeader right after the Requested Block
			BlockHeader* newHead = new(splitBlock)BlockHeader();
			newHead->data.previousPhysicalBlock = currentNode;
			newHead->data.nextPhysicalBlock = currentNode->data.nextPhysicalBlock;
			currentNode->data.nextPhysicalBlock = newHead;
			newHead->data.isFree = true;
			head = newHead;
			head->data.nextFreeBlock = currentNode->data.nextFreeBlock;
			head->data.managedBlocks = numFreeBlocksAfterSplit;
			return reinterpret_cast<T*>(blockToReturn);

		}

		void free(void* ptr) {
			//Go to the point infront of the pointer to free, where an old header was before
			BlockHeader* blockHead = (reinterpret_cast<BlockHeader*>(ptr)) - 1;
			if (blockHead->data.magic == MAGIC) {
				BlockHeader* oldHead = head;
				BlockHeader* newHead = blockHead;
				head = newHead;
				head->data.isFree = true;
				head->data.nextFreeBlock = oldHead;
				oldHead->data.previousFreeBlock = head;
				//Now try to coalesce with the next and previous block

				tryCoalesce(blockHead);
				tryCoalesce(oldHead);
			}
		}

		void reset() {
			initialize();
		}
	};
}

#endif