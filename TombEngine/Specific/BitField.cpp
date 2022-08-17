#include "framework.h"
#include "Specific/BitField.h"

using std::vector;

namespace TEN::Utils
{
	BitField::BitField()
	{
	}

	BitField::BitField(uint size)
	{
		this->BitSet.resize(size);
	}

	// TODO: packedBits as uint has max size of 64.
	BitField::BitField(uint size, ulong packedBits)
	{
		vector<uint> indices = {};
		
		for (size_t i = 0; i <= size; i++)
		{
			uint bit = uint(1 << i);

			if ((packedBits & bit) == bit)
				indices.push_back(i);
		}

		this->Set(indices);
	}

	void BitField::Set(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > BitSet.size())
				continue;

			this->BitSet[index] = true;
		}
	}

	void BitField::Set(uint index)
	{
		this->Set({ index });
	}

	void BitField::SetAll()
	{
		std::fill(this->BitSet.begin(), this->BitSet.end(), true);
	}

	void BitField::Clear(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > BitSet.size())
				continue;

			this->BitSet[index] = false;
		}
	}
	
	void BitField::Clear(uint index)
	{
		this->Clear({ index });
	}

	void BitField::ClearAll()
	{
		std::fill(this->BitSet.begin(), this->BitSet.end(), false);
	}
	
	void BitField::Flip(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > BitSet.size())
				continue;

			this->BitSet[index].flip();
		}
	}
	
	void BitField::Flip(uint index)
	{
		this->Flip({ index });
	}

	void BitField::FlipAll()
	{
		this->BitSet.flip();
	}

	bool BitField::Test(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (!BitSet[index])
				return false;
		}

		return true;
	}

	bool BitField::Test(uint index)
	{
		return this->Test({ index });
	}

	bool BitField::TestAll()
	{
		for (auto& bit : this->BitSet)
		{
			if (!bit)
				return false;
		}

		return true;
	}
	
	bool BitField::TestNone()
	{
		for (auto& bit : this->BitSet)
		{
			if (bit)
				return false;
		}

		return true;
	}
	
	unsigned int BitField::GetSize()
	{
		return BitSet.size();
	}

	unsigned long BitField::GetPackedBits()
	{
		ulong packedBits = 0;

		// TODO: uint has max size of 64.
		for (size_t i = 0; i <= 64; i++)
		{
			uint bit = uint(1 << i);
			packedBits |= bit;
		}

		return packedBits;
	}
}
