#pragma once

using std::vector;

namespace TEN::Utils
{
	class BitField
	{
	private:
		vector<bool> Field;

	public:
		BitField();
		BitField(uint size);
		BitField(uint size, ulong packedBits);

		uint  GetSize();
		ulong GetPackedBits();

		void Set(const vector<uint>& indices);
		void Set(uint index);
		void SetAll();
		void Clear(const vector<uint>& indices);
		void Clear(uint index);
		void ClearAll();
		void Flip(const vector<uint>& indices);
		void Flip(uint index);
		void FlipAll();

		bool Test(const vector<uint>& indices);
		bool Test(uint index);
		bool TestAll();
		bool TestNone();

	private:
		void Fill(bool value);
	};
}
