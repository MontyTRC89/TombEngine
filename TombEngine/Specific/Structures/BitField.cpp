#include "framework.h"
#include "Specific/Structures/BitField.h"

namespace TEN::Utils
{
	const BitField BitField::Empty	 = BitField(0);
	const BitField BitField::Default = BitField(SIZE_DEFAULT);

	BitField::BitField()
	{
		_bits.resize(SIZE_DEFAULT);
	}

	BitField::BitField(unsigned int size)
	{
		// NOTE: Bits initialize as unset.
		size = std::min<unsigned int>(size, SIZE_DEFAULT);
		_bits.resize(size);
	}

	BitField::BitField(unsigned int size, unsigned int packedBits)
	{
		size = std::min<unsigned int>(size, SIZE_DEFAULT);
		_bits.reserve(size);

		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int bit = unsigned int(1 << i);
			_bits.push_back((packedBits & bit) == bit);
		}
	}

	BitField::BitField(const std::string& bitString)
	{
		_bits.reserve(bitString.size());

		for (const char& bit : bitString)
			_bits.push_back(bit == '1');
	}

	bool BitField::IndexIsCorrect(unsigned int index) const
	{
		if (index >= _bits.size())
		{
			TENLog(std::string("BitField attempted to access bit at invalid index."), LogLevel::Warning);
			return false;
		}

		return true;
	}

	unsigned int BitField::GetSize() const
	{
		return (unsigned int)_bits.size();
	}

	unsigned int BitField::GetCount() const
	{
		unsigned int count = 0;
		for (const bool& bit : _bits)
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
			if (!IndexIsCorrect(index))
				continue;
			
			_bits[index] = true;
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
			if (!IndexIsCorrect(index))
				continue;

			_bits[index] = false;
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
			if (!IndexIsCorrect(index))
				continue;

			_bits[index].flip();
		}
	}
	
	void BitField::Flip(unsigned int index)
	{
		Flip(std::vector<unsigned int>{ index });
	}

	void BitField::FlipAll()
	{
		_bits.flip();
	}

	bool BitField::Test(const std::vector<unsigned int>& indices, bool testAny) const
	{
		for (const unsigned int& index : indices)
		{
			if (!IndexIsCorrect(index))
				continue;

			// Test if any bits at input indices are set.
			if (testAny)
			{
				if (_bits[index])
					return true;
			}
			// Test if any bits at input indices are set.
			else
			{
				if (!_bits[index])
					return false;
			}
		}

		return (testAny ? false : true);
	}

	bool BitField::Test(unsigned int index) const
	{
		if (!IndexIsCorrect(index))
			return false;

		return _bits[index];
	}

	bool BitField::TestAny() const
	{
		for (const bool& bit : _bits)
		{
			if (bit)
				return true;
		}

		return false;
	}

	bool BitField::TestAll() const
	{
		for (const bool& bit : _bits)
		{
			if (!bit)
				return false;
		}

		return true;
	}

	unsigned int BitField::ToPackedBits() const
	{
		unsigned int packedBits = 0;
		for (unsigned int i = 0; i < _bits.size(); i++)
		{
			if (_bits[i])
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
		for (const bool& bit : _bits)
			bitString += bit ? '1' : '0';

		return bitString;
	}

	bool BitField::operator ==(unsigned int packedBits) const
	{
		for (unsigned int i = 0; i < _bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			if (_bits[i] != ((packedBits & bit) == bit))
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
		for (unsigned int i = 0; i < _bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			_bits[i] = ((packedBits & bit) == bit);
		}

		return *this;
	}

	BitField& BitField::operator &=(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < _bits.size(); i++)
		{
			unsigned int bit = unsigned int(1 << i);
			_bits[i] = (_bits[i] && (packedBits & bit) == bit);
		}

		return *this;
	}

	BitField& BitField::operator |=(unsigned int packedBits)
	{
		for (unsigned int i = 0; i < _bits.size(); i++)
		{
			if (_bits[i])
				continue;

			unsigned int bit = unsigned int(1 << i);
			if ((packedBits & bit) == bit)
				_bits[i] = true;
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
		std::fill(_bits.begin(), _bits.end(), value);
	}
}
