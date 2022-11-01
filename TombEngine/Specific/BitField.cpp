#include "framework.h"
#include "Specific/BitField.h"

#include <limits>

using std::string;
using std::vector;

namespace TEN::Utils
{
	constexpr auto BIT_FIELD_SIZE_MAX = std::numeric_limits<unsigned int>::digits;

	BitField::BitField()
	{
		this->Bits.resize(BIT_FIELD_SIZE_MAX);
	}

	BitField::BitField(unsigned int size)
	{
		size = std::min<unsigned int>(size, BIT_FIELD_SIZE_MAX);
		this->Bits.resize(size);
	}

	BitField::BitField(unsigned int size, unsigned int packedBits)
	{
		size = std::min<unsigned int>(size, BIT_FIELD_SIZE_MAX);
		this->Bits.resize(size);

		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
		}
	}

	BitField::BitField(const string& bitString)
	{
		for (const char& bit : bitString)
			this->Bits.push_back((bit == '1') ? true : false);
	}

	unsigned int BitField::GetSize() const
	{
		return Bits.size();
	}

	unsigned int BitField::GetCount() const
	{
		unsigned int count = 0;
		for (const bool& bit : this->Bits)
		{
			if (bit)
				count++;
		}

		return count;
	}

	void BitField::Set(const vector<unsigned int>& indices)
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to set bit at invalid index."), LogLevel::Warning);
				continue;
			}
			
			this->Bits[index] = true;
		}
	}

	void BitField::Set(unsigned int index)
	{
		this->Set(std::vector<unsigned int> { index });
	}

	void BitField::SetAll()
	{
		this->Fill(true);
	}

	void BitField::Clear(const vector<unsigned int>& indices)
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to clear bit at invalid index."), LogLevel::Warning);
				continue;
			}

			this->Bits[index] = false;
		}
	}
	
	void BitField::Clear(unsigned int index)
	{
		this->Clear(std::vector<unsigned int> { index });
	}

	void BitField::ClearAll()
	{
		this->Fill(false);
	}
	
	void BitField::Flip(const vector<unsigned int>& indices)
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to flip bit at invalid index."), LogLevel::Warning);
				continue;
			}

			this->Bits[index].flip();
		}
	}
	
	void BitField::Flip(unsigned int index)
	{
		this->Flip(std::vector<unsigned int> { index });
	}

	void BitField::FlipAll()
	{
		this->Bits.flip();
	}

	bool BitField::Test(const vector<unsigned int>& indices, bool testAny) const
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(string("BitField attempted to test bit at invalid index."), LogLevel::Warning);
				continue;
			}

			// Test whether ANY bits at passed indices are true.
			if (testAny)
			{
				if (Bits[index])
					return true;
			}
			// Test whether ALL bits at passed indices are true.
			else
			{
				if (!Bits[index])
					return false;
			}
		}

		return (testAny ? false : true);
	}

	bool BitField::Test(unsigned int index) const
	{
		return this->Test(std::vector<unsigned int> { index });
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

	unsigned int BitField::ToPackedBits() const
	{
		unsigned int packedBits = 0;
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			if (Bits[i])
			{
				unsigned int bit = unsigned int(1 << i);
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

	bool BitField::operator ==(unsigned int packedBits) const
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if (Bits[i] != ((packedBits & bit) == bit))
				return false;
		}

		return true;
	}
	
	bool BitField::operator !=(unsigned int packedBits) const
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if (Bits[i] != ((packedBits & bit) == bit))
				return true;
		}

		return false;
	}

	BitField& BitField::operator =(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
			else
				this->Bits[i] = false;
		}

		return *this;
	}

	BitField& BitField::operator &=(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if (Bits[i] && (packedBits & bit) == bit)
				this->Bits[i] = true;
			else
				this->Bits[i] = false;
		}

		return *this;
	}

	BitField& BitField::operator |=(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			if (Bits[i])
				continue;

			unsigned int bit = unsigned int(1 << i);
			if ((packedBits & bit) == bit)
				this->Bits[i] = true;
		}

		return *this;
	}
	
	unsigned int BitField::operator &(unsigned int packedBits) const
	{
		return (this->ToPackedBits() & packedBits);
	}

	unsigned int BitField::operator |(unsigned int packedBits) const
	{
		return (this->ToPackedBits() | packedBits);
	}

	void BitField::Fill(bool value)
	{
		std::fill(this->Bits.begin(), this->Bits.end(), value);
	}
}
