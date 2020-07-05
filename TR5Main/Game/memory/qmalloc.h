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
		size_t pageSize;
		size_t size;
		uint8_t* arena;
		int* _arena_metadata = 0;
		unsigned char* _arena_data = 0;
		int _arena_pages = 0;
		int _arena_freestart = 1;
		static constexpr unsigned int MALLOC_MAGIC = 0xDEADBEEF;

	public:
		MemoryPool(MemoryUnit unit, size_t amount, size_t pageSize) : pageSize(pageSize), size(static_cast<size_t>(unit)* amount), arena(new uint8_t[size]{ 0 }) {
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
		T* malloc(size_t count = 1) noexcept {
			int pages;
			int n;
			T* retval;

			pages = 1 + ((sizeof(T)*count - 1) / pageSize);
			n = findspot(pages);
			if (n < 0) std::exception("Memory Pool is exhausted!");
			retval = (T*)(_arena_data + (n * pageSize));
#if _DEBUG
			printf("malloc: for %d bytes -> %08X\n", size, retval);
#endif
			return retval;
		}
		/*
		 * Frees the occupied memory of ptr
		 **/
		void free(void* ptr) noexcept;
	};
}
