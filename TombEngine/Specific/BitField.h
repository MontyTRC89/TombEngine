#pragma once

using std::bitset;
using std::vector;

namespace TEN::Utils
{
	template <unsigned int MAX>
	class BitField
	{
		typedef unsigned int  uint;
		typedef unsigned long ulong;

	private:
		bitset<MAX> BitSet;

	public:
		BitField();
		BitField(ulong packedBits);

		void Set(vector<uint>& indices);
		void Set(uint index);
		void SetAll();
		void Clear(vector<uint>& indices);
		void Clear(uint index);
		void ClearAll();
		void Flip(vector<uint>& indices);
		void Flip(uint index);
		void FlipAll();

		bool Test(vector<uint>& indices);
		bool Test(uint index);
		bool TestAll();
		bool TestNone();

		unsigned int  GetSize();
		unsigned long GetPackedBits();
	};
}
