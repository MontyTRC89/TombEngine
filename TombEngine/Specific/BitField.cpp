#include "framework.h"
#include "Specific/BitField.h"

#include <limits>

using std::vector;

namespace TEN::Utils
{
	// TODO: Remove size cap when all conversions are complete.
	constexpr auto BIT_FIELD_SIZE_MAX = std::numeric_limits<uint>::digits;

	BitField::BitField()
	{
	}

	BitField::BitField(uint size)
	{
		size = std::min<uint>(size, BIT_FIELD_SIZE_MAX);
		this->Container.resize(size);
	}

	BitField::BitField(uint size, uint packedBits)
	{
		size = std::min<uint>(size, BIT_FIELD_SIZE_MAX);
		this->Container.resize(size);

		for (size_t i = 0; i < size; i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Container[i] = true;
		}
	}

	uint BitField::GetSize()
	{
		return Container.size();
	}

	uint BitField::GetPackedBits()
	{
		uint packedBits = 0;
		for (size_t i = 0; i < this->GetSize(); i++)
		{
			if (Container[i])
			{
				uint bit = uint(1 << i);
				packedBits |= bit;
			}
		}

		return packedBits;
	}

	void BitField::Set(const vector<uint>& indices)
	{
		for (auto& index : indices)
		{
			if (index < Container.size())
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
			if (index < Container.size())
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
			if (index < Container.size())
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
			if (Container[index])
				return true;
		}

		return false;
	}

	bool BitField::Test(uint index)
	{
		return this->Test({ index });
	}

	bool BitField::TestAny()
	{
		for (auto& bit : this->Container)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::TestAll()
	{
		for (auto& bit : this->Container)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::IsFull()
	{
		for (auto& bit : this->Container)
		{
			if (!bit)
				return false;
		}

		return true;
	}

	bool BitField::IsEmpty()
	{
		for (auto& bit : this->Container)
		{
			if (bit)
				return false;
		}

		return true;
	}

	BitField::operator uint()
	{
		return this->GetPackedBits();
	}

	BitField BitField::operator =(uint packedBits)
	{
		for (size_t i = 0; i < this->GetSize(); i++)
		{
			uint bit = uint(1 << i);

			if ((packedBits & bit) == bit)
				this->Container[i] = true;
			else
				this->Container[i] = false;
		}
	}

	void BitField::Fill(bool value)
	{
		std::fill(this->Container.begin(), this->Container.end(), value);
	}
}
