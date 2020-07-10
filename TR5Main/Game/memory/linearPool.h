#pragma once
#include <memory>
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
			if (count < 1) return nullptr;
			
			size_t requestedBytes = sizeof(T) * count;
			if (offset + requestedBytes > allocatedBytes) throw std::runtime_error("Out of memory!");
			T* returnValue = reinterpret_cast<T*>(bytes.get()[offset]);
			offset += requestedBytes;
			return returnValue;
		}

		void flush() noexcept {
			offset = 0;
		}
	};
}
