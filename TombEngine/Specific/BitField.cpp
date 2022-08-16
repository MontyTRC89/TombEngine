#include "framework.h"
#include "Specific/BitField.h"

using std::vector;

namespace TEN::Utils
{
	template <unsigned int MAX>
	BitField<MAX>::BitField()
	{
	}

	template <unsigned int MAX>
	BitField<MAX>::BitField(ulong packedBits)
	{
		vector<uint> indices = {};
		
		for (size_t i = 0; i <= MAX; i++)
		{
			uint bit = uint(1 << i);

			if ((packedBits & bit) == bit)
				indices.push_back(i);
		}

		this->Set(indices);
	}

	template <unsigned int MAX>
	void BitField<MAX>::Set(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > MAX)
				continue;

			this->BitSet.set(index);
		}
	}

	template <unsigned int MAX>
	void BitField<MAX>::Set(uint index)
	{
		this->Set({ index });
	}

	template <unsigned int MAX>
	void BitField<MAX>::SetAll()
	{
		this->BitSet.set();
	}

	template <unsigned int MAX>
	void BitField<MAX>::Clear(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > MAX)
				continue;

			this->BitSet.reset(index);
		}
	}

	template <unsigned int MAX>
	void BitField<MAX>::Clear(uint index)
	{
		this->Clear({ index });
	}

	template <unsigned int MAX>
	void BitField<MAX>::ClearAll()
	{
		this->BitSet.reset();
	}

	template <unsigned int MAX>
	void BitField<MAX>::Flip(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > MAX)
				continue;

			this->BitSet.flip(index);
		}
	}

	template <unsigned int MAX>
	void BitField<MAX>::Flip(uint index)
	{
		this->Flip({ index });
	}

	template <unsigned int MAX>
	void BitField<MAX>::FlipAll()
	{
		this->BitSet.flip();
	}

	template <unsigned int MAX>
	bool BitField<MAX>::Test(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (!BitSet.test(index))
				return false;
		}

		return true;
	}

	template <unsigned int MAX>
	bool BitField<MAX>::Test(uint index)
	{
		this->Test({ index });
	}

	template <unsigned int MAX>
	bool BitField<MAX>::TestAll()
	{
		return BitSet.all();
	}
	
	template <unsigned int MAX>
	bool BitField<MAX>::TestNone()
	{
		return BitSet.none();
	}

	template <unsigned int MAX>
	unsigned int BitField<MAX>::GetSize()
	{
		return MAX;
	}

	template <unsigned int MAX>
	unsigned long BitField<MAX>::GetPackedBits()
	{
		return BitSet.to_ulong();
	}
}
