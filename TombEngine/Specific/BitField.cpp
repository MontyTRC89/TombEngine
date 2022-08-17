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
		this->Container.resize(size);
	}

	// TODO: packedBits as ulong has max size of 64.
	BitField::BitField(uint size, ulong packedBits)
	{
		this->Container.resize(size);

		vector<uint> indices = {};
		for (size_t i = 0; i < size; i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				indices.push_back(i);
		}

		this->Set(indices);
	}

	uint BitField::GetSize()
	{
		return Container.size();
	}

	ulong BitField::GetPackedBits()
	{
		ulong packedBits = 0;

		// TODO: ulong has max size of 64.
		for (size_t i = 0; i < 64; i++)
		{
			uint bit = uint(1 << i);
			packedBits |= bit;
		}

		return packedBits;
	}

	void BitField::Set(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index >= Container.size())
				continue;

			this->Container[index] = true;
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

	void BitField::Clear(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index >= Container.size())
				continue;

			this->Container[index] = false;
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
	
	void BitField::Flip(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index >= Container.size())
				continue;

			this->Container[index].flip();
		}
	}
	
	void BitField::Flip(uint index)
	{
		this->Flip({ index });
	}

	void BitField::FlipAll()
	{
		this->Container.flip();
	}

	bool BitField::Test(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (!Container[index])
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
		for (auto& bit : this->Container)
		{
			if (!bit)
				return false;
		}

		return true;
	}
	
	bool BitField::TestNone()
	{
		for (auto& bit : this->Container)
		{
			if (bit)
				return false;
		}

		return true;
	}
	
	void BitField::Fill(bool value)
	{
		std::fill(this->Container.begin(), this->Container.end(), value);
	}
}
