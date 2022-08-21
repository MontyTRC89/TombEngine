#pragma once

using std::vector;

namespace TEN::Utils
{
	class BitField
	{
	private:
		vector<bool> Container;

	public:
		// Constructors
		BitField();
		BitField(uint size);
		BitField(uint size, uint packedBits);

		// Getters
		uint GetSize();
		uint GetPackedBits();

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
		bool TestAll();
		bool IsFull();
		bool IsEmpty();

		// Operators
		operator uint();
		BitField operator =(uint packedBits);

	private:
		void Fill(bool value);
	};
}
