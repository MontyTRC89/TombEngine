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
		this->Field.resize(size);
	}

	// TODO: packedBits as ulong has max size of 64.
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

	unsigned int BitField::GetSize()
	{
		return Field.size();
	}

	unsigned long BitField::GetPackedBits()
	{
		ulong packedBits = 0;

		// TODO: ulong has max size of 64.
		for (size_t i = 0; i <= 64; i++)
		{
			uint bit = uint(1 << i);
			packedBits |= bit;
		}

		return packedBits;
	}

	void BitField::Set(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > Field.size())
				continue;

			this->Field[index] = true;
		}
	}

	void BitField::Set(uint index)
	{
		this->Set({ index });
	}

	void BitField::SetAll()
	{
		this->Fill(true);
	}

	void BitField::Clear(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > Field.size())
				continue;

			this->Field[index] = false;
		}
	}
	
	void BitField::Clear(uint index)
	{
		this->Clear({ index });
	}

	void BitField::ClearAll()
	{
		this->Fill(false);
	}
	
	void BitField::Flip(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (index > Field.size())
				continue;

			this->Field[index].flip();
		}
	}
	
	void BitField::Flip(uint index)
	{
		this->Flip({ index });
	}

	void BitField::FlipAll()
	{
		this->Field.flip();
	}

	bool BitField::Test(vector<uint>& indices)
	{
		for (uint& index : indices)
		{
			if (!Field[index])
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
		for (auto& bit : this->Field)
		{
			if (!bit)
				return false;
		}

		return true;
	}
	
	bool BitField::TestNone()
	{
		for (auto& bit : this->Field)
		{
			if (bit)
				return false;
		}

		return true;
	}
	
	void BitField::Fill(bool value)
	{
		std::fill(this->Field.begin(), this->Field.end(), value);
	}
}
