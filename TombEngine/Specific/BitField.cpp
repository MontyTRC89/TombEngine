#include "framework.h"
#include "Specific/BitField.h"

#include <limits>

using std::string;
using std::vector;

namespace TEN::Utils
{
	constexpr auto BIT_FIELD_SIZE_MAX = std::numeric_limits<uint>::digits;

	BitField::BitField()
	{
		this->Bits.resize(BIT_FIELD_SIZE_MAX);
	}

	BitField::BitField(uint size)
	{
		size = std::min<uint>(size, BIT_FIELD_SIZE_MAX);
		this->Bits.resize(size);
	}

	BitField::BitField(uint size, uint packedBits)
	{
		size = std::min<uint>(size, BIT_FIELD_SIZE_MAX);
		this->Bits.resize(size);

		for (uint i = 0; i < size; i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
		}
	}

	BitField::BitField(const string& bitString)
	{
		for (const char& bit : bitString)
			this->Bits.push_back((bit == '1') ? true : false);
	}

	uint BitField::GetSize() const
	{
		return Bits.size();
	}

	uint BitField::GetCount() const
	{
		uint count = 0;
		for (const bool& bit : this->Bits)
		{
			if (bit)
				count++;
		}

		return count;
	}

	void BitField::Set(const vector<uint>& indices)
	{
		for (const uint& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to set bit at invalid index."), LogLevel::Warning);
				continue;
			}
			
			this->Bits[index] = true;
		}
	}

	void BitField::Set(uint index)
	{
		if (index >= Bits.size())
		{
			TENLog(string("BitField attempted to set bit at invalid index."), LogLevel::Warning);
			return;
		}

		this->Bits[index] = true;
	}

	void BitField::SetAll()
	{
		this->Fill(true);
	}

	void BitField::Clear(const vector<uint>& indices)
	{
		for (const uint& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to clear bit at invalid index."), LogLevel::Warning);
				continue;
			}

			this->Bits[index] = false;
		}
	}
	
	void BitField::Clear(uint index)
	{
		if (index >= Bits.size())
		{
			TENLog(string("BitField attempted to clear bit at invalid index."), LogLevel::Warning);
			return;
		}

		this->Bits[index] = false;
	}

	void BitField::ClearAll()
	{
		this->Fill(false);
	}
	
	void BitField::Flip(const vector<uint>& indices)
	{
		for (const uint& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to flip bit at invalid index."), LogLevel::Warning);
				continue;
			}

			this->Bits[index].flip();
		}
	}
	
	void BitField::Flip(uint index)
	{
		if (index >= Bits.size())
		{
			TENLog(string("BitField attempted to flip bit at invalid index."), LogLevel::Warning);
			return;
		}

		this->Bits[index].flip();
	}

	void BitField::FlipAll()
	{
		this->Bits.flip();
	}

	bool BitField::Test(const vector<uint>& indices, bool testAny) const
	{
		// Test whether ANY bits at passed indices are true.
		if (testAny)
		{
			for (const uint& index : indices)
			{
				if (index >= Bits.size())
				{
					TENLog(string("BitField attempted to test bit at invalid index."), LogLevel::Warning);
					continue;
				}

				if (Bits[index])
					return true;
			}

			return false;
		}
		// Test whether ALL bits at passed indices are true.
		else
		{
			for (const uint& index : indices)
			{
				if (index >= Bits.size())
				{
					TENLog(string("BitField attempted to test bit at invalid index."), LogLevel::Warning);
					return false;
				}

				if (!Bits[index])
					return false;
			}

			return true;
		}
	}

	bool BitField::Test(uint index) const
	{
		if (index >= Bits.size())
		{
			TENLog(string("BitField attempted to test bit at invalid index."), LogLevel::Warning);
			return false;
		}

		return Bits[index];
	}

	bool BitField::TestAny() const
	{
		for (const bool& bit : this->Bits)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::TestAll() const
	{
		for (const bool& bit : this->Bits)
		{
			if (!bit)
				return false;
		}

		return true;
	}

	uint BitField::ToPackedBits() const
	{
		uint packedBits = 0;
		for (uint i = 0; i < Bits.size(); i++)
		{
			if (Bits[i])
			{
				uint bit = uint(1 << i);
				packedBits |= bit;
			}
		}

		return packedBits;
	}

	string BitField::ToString() const
	{
		auto bitString = string();
		for (const bool& bit : this->Bits)
			bitString += bit ? '1' : '0';

		return bitString;
	}

	bool BitField::operator ==(uint packedBits) const
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if (Bits[i] != ((packedBits & bit) == bit))
				return false;
		}

		return true;
	}
	
	bool BitField::operator !=(uint packedBits) const
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if (Bits[i] != ((packedBits & bit) == bit))
				return true;
		}

		return false;
	}

	BitField& BitField::operator =(uint packedBits)
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
			else
				this->Bits[i] = false;
		}

		return *this;
	}

	BitField& BitField::operator &=(uint packedBits)
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			uint bit = uint(1 << i);
			if (Bits[i] && (packedBits & bit) == bit)
				this->Bits[i] = true;
			else
				this->Bits[i] = false;
		}

		return *this;
	}

	BitField& BitField::operator |=(uint packedBits)
	{
		for (uint i = 0; i < Bits.size(); i++)
		{
			if (Bits[i])
				continue;

			uint bit = uint(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
		}

		return *this;
	}
	
	uint BitField::operator &(uint packedBits) const
	{
		return (this->ToPackedBits() & packedBits);
	}

	uint BitField::operator |(uint packedBits) const
	{
		return (this->ToPackedBits() | packedBits);
	}

	void BitField::Fill(bool value)
	{
		std::fill(this->Bits.begin(), this->Bits.end(), value);
	}
}
