#pragma once
#include <memory>
#include "Game/debug/assert.h"
#include "Game/debug/log.h"

namespace T5M::Memory {
	enum class MemoryUnit : size_t {
		Byte = 1,
		KByte = Byte*1024,
		MByte = KByte*1024,
		GByte = MByte*1024,
	};
	class LinearPool {
	private:
		const size_t allocatedBytes;
		const std::unique_ptr<uint8_t> bytes;
		size_t offset;
	public:
		LinearPool(size_t amount, MemoryUnit unit) : allocatedBytes(amount* static_cast<size_t>(unit)), bytes(new uint8_t[allocatedBytes]) {
			offset = 0;
		}
		template<typename T>
		[[nodiscard]]T* malloc(size_t count = 1) noexcept {
			assertm(sizeof(T) * count >= 1, "Requested memory needs to be greater than 0!");
			size_t requestedBytes = sizeof(T) * count;
			Log("LinearPool - Malloc :" << requestedBytes << " Bytes")
			assertm(offset + requestedBytes > allocatedBytes, "Memory must not overflow linear pool!");
			T* returnValue = reinterpret_cast<T*>(bytes.get()[offset]);
			offset += requestedBytes;
			Log("LinearPool - Malloc : New Offset at " << offset)
			return returnValue;
		}

		void flush() noexcept {
			std::memset(bytes.get(), 0, allocatedBytes);
			offset = 0;
		}
	public:
		size_t size() const {
			return allocatedBytes;
		}
	};
}
