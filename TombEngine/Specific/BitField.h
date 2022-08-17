#pragma once

using std::vector;

namespace TEN::Utils
{
	class BitField
	{
		typedef unsigned int  uint;
		typedef unsigned long ulong;

	private:
		vector<bool> Field;

	public:
		BitField();
		BitField(uint size);
		BitField(uint size, ulong packedBits);

		unsigned int  GetSize();
		unsigned long GetPackedBits();

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

	private:
		void Fill(bool value);
	};
}
