#pragma once

using std::string;
using std::vector;

namespace TEN::Utils
{
	// TODO: Switch to std::span container type as parameter whenever we update to C++20.
	// TODO: Remove size cap when all conversions are complete.

	class BitField
	{
	private:
		vector<bool> Bits;

	public:
		// Constructors
		BitField();
		BitField(uint size);
		BitField(uint size, uint packedBits);
		BitField(const string& bitString);

		// Getters
		uint GetSize() const;
		uint GetCount() const;

		// Setters
		void Set(const vector<uint>& indices);
		void Set(uint index);
		void SetAll();
		void Clear(const vector<uint>& indices);
		void Clear(uint index);
		void ClearAll();
		void Flip(const vector<uint>& indices);
		void Flip(uint index);
		void FlipAll();

		// Inquirers
		bool Test(const vector<uint>& indices, bool testAny = true) const;
		bool Test(uint index) const;
		bool TestAny() const;
		bool TestAll() const;
		bool TestNone() const;

		// Converters
		uint   ToPackedBits() const;
		string ToString() const;

		// Operators
		// NOTE: packedBits will not be assessed in full if the size of the given BitField object is less than BIT_FIELD_SIZE_MAX.
		bool	  operator ==(uint packedBits) const;
		bool	  operator !=(uint packedBits) const;
		BitField& operator =(uint packedBits);
		BitField& operator &=(uint packedBits);
		BitField& operator |=(uint packedBits);
		uint	  operator &(uint packedBits) const;
		uint	  operator |(uint packedBits) const;

	private:
		void Fill(bool value);
	};
}
