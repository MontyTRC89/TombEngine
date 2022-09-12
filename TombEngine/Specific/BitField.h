#pragma once

using std::vector;

namespace TEN::Utils
{
	// TODO: Switch to std::span container type as parameter whenever we update to C++20.
	// TODO: Remove size cap when all conversions are complete.

	class BitField
	{
	private:
		vector<bool> Bits;

	public:
		// Constructors
		BitField();
		BitField(uint size);
		BitField(uint size, uint packedBits);

		// Getters
		uint GetSize();
		uint GetPackedBits() const;

		// Setters
		void Set(const vector<uint>& indices);
		void Set(uint index);
		void SetAll();
		void Clear(const vector<uint>& indices);
		void Clear(uint index);
		void ClearAll();
		void Flip(const vector<uint>& indices);
		void Flip(uint index);
		void FlipAll();

		// Inquirers
		bool Test(const vector<uint>& indices);
		bool Test(uint index);
		bool TestAny();
		bool IsFull();
		bool IsEmpty();

		// Operators
		operator uint() const;
		BitField& operator =(uint packedBits);
		BitField& operator |=(uint packedBits);
		BitField  operator &(uint packedBits);

	private:
		void Fill(bool value);
	};
}
