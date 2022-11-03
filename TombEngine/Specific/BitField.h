#pragma once

namespace TEN::Utils
{
	// TODO: Switch to std::span container type as parameter whenever we update to C++20.
	// TODO: Remove size cap when all conversions are complete.

	class BitField
	{
	private:
		// Components
		std::vector<bool> Bits = {};

	public:
		// Constructors
		BitField();
		BitField(unsigned int size);
		BitField(unsigned int size, unsigned int packedBits);
		BitField(const std::string& bitString);
		
		// Getters
		unsigned int GetSize() const;
		unsigned int GetCount() const;

		// Setters
		void Set(const std::vector<unsigned int>& indices);
		void Set(unsigned int index);
		void SetAll();
		void Clear(const std::vector<unsigned int>& indices);
		void Clear(unsigned int index);
		void ClearAll();
		void Flip(const std::vector<unsigned int>& indices);
		void Flip(unsigned int index);
		void FlipAll();

		// Inquirers
		bool Test(const std::vector<unsigned int>& indices, bool testAny = true) const;
		bool Test(unsigned int index) const;
		bool TestAny() const;
		bool TestAll() const;

		// Converters
		unsigned int ToPackedBits() const;
		std::string	 ToString() const;

		// Operators
		// NOTE: packedBits will not be assessed in full if the size of the given BitField object is less than BIT_FIELD_SIZE_MAX.
		bool		 operator ==(unsigned int packedBits) const;
		bool		 operator !=(unsigned int packedBits) const;
		BitField&	 operator =(unsigned int packedBits);
		BitField&	 operator &=(unsigned int packedBits);
		BitField&	 operator |=(unsigned int packedBits);
		unsigned int operator &(unsigned int packedBits) const;
		unsigned int operator |(unsigned int packedBits) const;

	private:
		// Helpers
		void Fill(bool value);
	};
}
