#pragma once
namespace T5M::Memory {
	void* calloc(size_t nmemb, size_t size);
	void* malloc(size_t size);
	void free(void* ptr);
	void* realloc(void* ptr, size_t size);
	void* alloca(size_t size);
	void alloca_reset();
}
