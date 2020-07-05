#include "framework.h"
#include "qmalloc.h"
namespace T5M::Memory {

	void MemoryPool::free(void* ptr) noexcept {
		int n;

		//malloc_init();
		//  n = 0;
		n = ((unsigned char*)ptr - _arena_data) / pageSize;
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

	int MemoryPool::usedblocks(void* ptr) noexcept {
		int i;
		int retval;

		//malloc_init();
		retval = 0;
		for (i = 0; i < _arena_pages; i++) {
			if (_arena_metadata[i] == (int)ptr)
				retval++;
		}
		return retval;
	}

	int MemoryPool::findspot(int pages) noexcept {
		int i, j;

		//malloc_init();
#ifdef _DEBUG
		printf("findspot: for %d pages\n", pages);
#endif // _DEBUG
		//  for (i = 1; i < (_arena_pages - pages); i++)
		if (_arena_freestart >= (_arena_pages - pages))
			_arena_freestart = 1;
#ifdef _DEBUG
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
					_arena_metadata[i + j] = (int)(_arena_data + (i * pageSize));
				}
#ifdef _DEBUG
				printf("findspot: returning page %d (x %d) @ %08X\n", i, pages, (_arena_data + (i * pageSize)));
#endif // _DEBUG
				_arena_freestart = i;
				return i;
			}
		}
#if _DEBUG
		printf("FATAL: findspot: Heap exhausted\n");
#endif
		return -1;
	}

	void MemoryPool::init() noexcept {
		int* magic = (int*)arena;
		if (magic[0] == MALLOC_MAGIC)
			return;  /* Already initialized. */
		{
			int divider;
			divider = pageSize + sizeof(*_arena_metadata);
			_arena_pages = size / divider;
			_arena_metadata = (int*)arena;
			_arena_data = (unsigned char*)(_arena_metadata + _arena_pages);
			int* magic = (int*)arena;
			magic[0] = MALLOC_MAGIC;
		}
#ifdef _DEBUG
		printf("malloc_init: %d pages of %d bytes.  Metadata @ %08X, heap @ %08X\n", _arena_pages, pageSize, _arena_metadata, _arena_data);
#endif // _DEBUG
	}

}
