#include "framework.h"
#include "qmalloc.h"
namespace T5M::Memory {
	constexpr size_t MEMORY_UNIT = 1024 * 1024; //MB
	constexpr size_t MEMORY_AMOUNT = 256; // 256MB
	constexpr size_t MEMORY_SIZE = MEMORY_UNIT * MEMORY_AMOUNT;
	unsigned char arena[MEMORY_SIZE];
	int* _arena_metadata = 0;
	unsigned char* _arena_data = 0;
	int _arena_pages = 0;
	int _arena_freestart = 1;
	constexpr unsigned int MALLOC_MAGIC = 0xDEADBEEF;
	constexpr size_t PAGESIZE = 128;

	/*
	Every PAGESIZE-bytes chunk of heap is attached to one integer metadata.
	Metadata holds a key, the value of the starting address of the memory block.
	The result is _arena_pages number of chunks, with a corresponding _arena_pages
	number of metadata integers.
	*/


	void malloc_init() {
		if (((int*)arena)[0] == MALLOC_MAGIC)
			return;  /* Already initialized. */

		{
			int divider;
			divider = PAGESIZE + sizeof(*_arena_metadata);
			_arena_pages = MEMORY_SIZE / divider;
			_arena_metadata = (int*)arena;
			_arena_data = (unsigned char*)(_arena_metadata + _arena_pages);
			((int*)arena)[0] = MALLOC_MAGIC;
		}
#ifdef _DEBUG_MEM
		printf("malloc_init: %d pages of %d bytes.  Metadata @ %08X, heap @ %08X\n", _arena_pages, PAGESIZE, _arena_metadata, _arena_data);
#endif // _DEBUG
	}


	int findspot(int pages) {
		int i, j;

		malloc_init();
#ifdef _DEBUG_MEM
		printf("findspot: for %d pages\n", pages);
#endif // _DEBUG
		//  for (i = 1; i < (_arena_pages - pages); i++)
		if (_arena_freestart >= (_arena_pages - pages))
			_arena_freestart = 1;
#ifdef _DEBUG_MEM
		printf("findspot: for %d pages starting @ %d\n", pages, _arena_freestart);
#endif // _DEBUG
		for (i = _arena_freestart; i < (_arena_pages - pages); i++) {
			if (_arena_metadata[i] == 0) {
				for (j = 0; j < pages; j++) {
					if (_arena_metadata[i + j] != 0)
						break;
				}
				if (j < pages)  /* loop ended because of insufficient free pages. */
				{
					i += j;    /* Skip the blocks we know we can't use. */
					continue;  /* with  for i = ... */
				}
				for (j = 0; j < pages; j++) {
					_arena_metadata[i + j] = (int)(_arena_data + (i * PAGESIZE));
				}
#ifdef _DEBUG_MEM
				printf("findspot: returning page %d (x %d) @ %08X\n", i, pages, (_arena_data + (i * PAGESIZE)));
#endif // _DEBUG
				_arena_freestart = i;
				return i;
			}
		}
#if _DEBUG_MEM
		printf("FATAL: findspot: Heap exhausted\n");
#endif
		return 0;
	}


	int usedblocks(void* ptr) {
		int i;
		int retval;

		malloc_init();
		retval = 0;
		for (i = 0; i < _arena_pages; i++) {
			if (_arena_metadata[i] == (int)ptr)
				retval++;
		}
		return retval;
	}


	void* malloc(size_t size) {
		int pages;
		int n;
		void* retval;

		pages = 1 + ((size - 1) / PAGESIZE);
		n = findspot(pages);
		retval = (void*)(_arena_data + (n * PAGESIZE));
#if _DEBUG_MEM
		printf("malloc: for %d bytes -> %08X\n", size, retval);
#endif
		return retval;
	}


	void* calloc(size_t nmemb, size_t size) {
		char* retval;
		int i;

		retval = (char*)(malloc(nmemb * size));
		for (i = 0; i < (nmemb * size); i++) {
			retval[i] = 0;
		}
		return (void*)retval;
	}


	void free(void* ptr) {
		int n;

		malloc_init();
		//  n = 0;
		n = ((unsigned char*)ptr - _arena_data) / PAGESIZE;
		if ((n < 0) || (n > _arena_pages)) {
			/* Outside heap.  Bad. */
			return;
		}
		_arena_freestart = n;  /* Next allocation tries here, to see if it fits. */
		while (_arena_metadata[n] == (int)ptr) {
			_arena_metadata[n] = 0;
			n++;
		}
		return;
	}


	void* realloc(void* ptr, size_t size) {
		void* m;
		int n, movesize;
		int pages;

		if (size == 0) {
			free(ptr);
			return 0;
		}
		m = malloc(size);
		movesize = usedblocks(ptr) * PAGESIZE;
#ifdef _DEBUG_MEM
		printf("realloc: from %08X to %08X, old size %d, new size %d\n", ptr, m, movesize, size);
#endif // _DEBUG
		if (size < movesize)
			movesize = size;
#ifdef _DEBUG_MEM
		printf("realloc: moving %d bytes\n", movesize);
#endif // _DEBUG
		//  memmove(m, ptr, movesize);
		memcpy(m, ptr, movesize);
		free(ptr);
		return m;
	}


#define _ALLOCA_RINGSIZE 256

	void* alloca(size_t size) {
		static void* alloca_strips[_ALLOCA_RINGSIZE] = { 0, };
		static int snum = 0;
		void* p;

		if (size < 0) {
			/* Hack: reset alloca allocations */
	  //printf("alloca: reset hack\n");
			for (snum = 0; snum < _ALLOCA_RINGSIZE; snum++) {
				free(alloca_strips[snum]);
				alloca_strips[snum] = 0;
			}
			snum = 0;
			return 0;
		}

		if (alloca_strips[snum]) {
			free(alloca_strips[snum]);
			alloca_strips[snum] = 0;
		}
		p = malloc(size);
		if (p == 0) {
			printf("FATAL: malloc out of memoy\n");
		}
		alloca_strips[snum] = p;
		snum = (snum + 1) % _ALLOCA_RINGSIZE;
		return p;
	}


	void alloca_reset() {
		alloca(-1);
	}

}
