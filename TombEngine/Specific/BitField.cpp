#include "framework.h"
#include "Specific/BitField.h"

namespace TEN::Utils
{
	const BitField BitField::Empty	 = BitField(0);
	const BitField BitField::Default = BitField(SIZE_DEFAULT);

	BitField::BitField()
	{
		Bits.resize(SIZE_DEFAULT);
	}

	BitField::BitField(unsigned int size)
	{
		// NOTE: Bits initialize as unset.
		size = std::min<unsigned int>(size, SIZE_DEFAULT);
		Bits.resize(size);
	}

	BitField::BitField(unsigned int size, unsigned int packedBits)
	{
		size = std::min<unsigned int>(size, SIZE_DEFAULT);
		Bits.reserve(size);

		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int bit = unsigned int(1 << i);
			Bits.push_back((packedBits & bit) == bit);
		}
	}

	BitField::BitField(const std::string& bitString)
	{
		Bits.reserve(bitString.size());

		for (const char& bit : bitString)
			Bits.push_back(bit == '1');
	}

	unsigned int BitField::GetSize() const
	{
		return Bits.size();
	}

	unsigned int BitField::GetCount() const
	{
		unsigned int count = 0;
		for (const bool& bit : Bits)
		{
			if (bit)
				count++;
		}

		return count;
	}

	void BitField::Set(const std::vector<unsigned int>& indices)
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(std::string("BitField attempted to set bit at invalid index."), LogLevel::Warning);
				continue;
			}
			
			Bits[index] = true;
		}
	}

	void BitField::Set(unsigned int index)
	{
		Set(std::vector<unsigned int>{ index });
	}

	void BitField::SetAll()
	{
		Fill(true);
	}

	void BitField::Clear(const std::vector<unsigned int>& indices)
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(std::string("BitField attempted to clear bit at invalid index."), LogLevel::Warning);
				continue;
			}

			Bits[index] = false;
		}
	}
	
	void BitField::Clear(unsigned int index)
	{
		Clear(std::vector<unsigned int>{ index });
	}

	void BitField::ClearAll()
	{
		Fill(false);
	}
	
	void BitField::Flip(const std::vector<unsigned int>& indices)
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(std::string("BitField attempted to flip bit at invalid index."), LogLevel::Warning);
				continue;
			}

			Bits[index].flip();
		}
	}
	
	void BitField::Flip(unsigned int index)
	{
		Flip(std::vector<unsigned int>{ index });
	}

	void BitField::FlipAll()
	{
		Bits.flip();
	}

	bool BitField::Test(const std::vector<unsigned int>& indices, bool testAny) const
	{
		for (const unsigned int& index : indices)
		{
			if (index >= Bits.size())
			{
				TENLog(std::string("BitField attempted to test bit at invalid index."), LogLevel::Warning);
				continue;
			}

			// Test if any bits at input indices are set.
			if (testAny)
			{
				if (Bits[index])
					return true;
			}
			// Test if any bits at input indices are set.
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
		return Test(std::vector<unsigned int>{ index });
	}

	bool BitField::TestAny() const
	{
		for (const bool& bit : Bits)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::TestAll() const
	{
		for (const bool& bit : Bits)
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

	std::string BitField::ToString() const
	{
		auto bitString = std::string();
		for (const bool& bit : Bits)
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
		return !(*this == packedBits);
	}

	BitField& BitField::operator =(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if ((packedBits & bit) == bit)
				Bits[i] = true;
			else
				Bits[i] = false;
		}

		return *this;
	}

	BitField& BitField::operator &=(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < Bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if (Bits[i] && (packedBits & bit) == bit)
				Bits[i] = true;
			else
				Bits[i] = false;
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
				Bits[i] = true;
		}

		return *this;
	}
	
	unsigned int BitField::operator &(unsigned int packedBits) const
	{
		return (ToPackedBits() & packedBits);
	}

	unsigned int BitField::operator |(unsigned int packedBits) const
	{
		return (ToPackedBits() | packedBits);
	}

	void BitField::Fill(bool value)
	{
		std::fill(Bits.begin(), Bits.end(), value);
	}
}
