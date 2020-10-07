#pragma once
namespace T5M::Memory {
	template <typename T, size_t Sz>
	class LinearArrayBuffer {
		size_t numElements;
		std::array<T, Sz> buffer;
	public:
		LinearArrayBuffer() : numElements(0) {}

		void push_back(const T& obj) {
			buffer[numElements] = obj;
			increaseCount();
		}

		void push_back(T&& obj) {
			buffer[numElements] = obj;
			increaseCount();
		}

		void increaseCount()
		{
			numElements++;
			numElements = std::min(numElements, maxSize()-1);
		}

		T& operator[](size_t index) {
			return buffer[index];
		}
		constexpr size_t maxSize() const {
			return Sz;
		}
		size_t size() const {
			return numElements;
		}

		auto begin() const {
			return buffer.begin();
		}

		auto end() const {
			return buffer.begin() + numElements;
		}

		void clear() {
			numElements = 0;
		}

		auto data() const {
			return buffer.data();
		}
	};
}