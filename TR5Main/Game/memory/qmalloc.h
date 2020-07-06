#pragma once
namespace T5M::Memory {
	enum class MemoryUnit : size_t{
		Byte = 1,
		KibiByte = Byte*1024,
		MebiByte = KibiByte*1024,
		GibiByte = MebiByte*1024
	};
	class MemoryPool {
	private:
		const size_t pageSize;
		const size_t size;
		uint8_t* arena;
		int* _arena_metadata = 0;
		unsigned char* _arena_data = 0;
		int _arena_pages = 0;
		int _arena_freestart = 1;
		static constexpr unsigned int MALLOC_MAGIC = 0xDEADBEEF;

	public:
		//Creates a Memory Pool with the specified memory and page size. The memory also contains the metadata and house keeping
		MemoryPool(MemoryUnit unit, size_t amount, size_t pageSize) : pageSize(pageSize), size(static_cast<size_t>(unit)* amount), arena(new uint8_t[size]{0}) {
			init();
		}
		~MemoryPool() {
			delete[] arena;
		}

	private:
		void init() noexcept;
		int findspot(int pages) noexcept;
		int usedblocks(void* ptr) noexcept;
	public:
		/*
		 * Reserves Memory of type T with an additional count
		 **/
		template<typename T>
		[[nodiscard]] T* malloc(size_t count = 1) noexcept {
			if (count < 1) return nullptr;
			int pages;
			int n;

			pages = 1 + ((sizeof(T)*count - 1) / pageSize);
			n = findspot(pages);
			if (n < 0) [[unlikely]] {
				throw std::exception("Memory Pool is exhausted!");
			}
#if _DEBUG
			printf("malloc: for %d bytes -> %08X\n", size, (_arena_data + (n * pageSize)));
#endif
			return (T*)(_arena_data + (n * pageSize));
		}
		/*
		 * Frees the occupied memory of ptr
		 **/
		void free(void* ptr) noexcept;
	};
}
