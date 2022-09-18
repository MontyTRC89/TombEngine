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
		uint GetSize();
		uint GetCount();

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
		bool Test(const vector<uint>& indices, bool checkAny = true);
		bool Test(uint index);
		bool TestAny();
		bool TestAll();
		bool TestNone();

		// Converters
		uint   ToPackedBits() const;
		string ToString() const;

		// Operators
		// NOTE: packedBits will not be assessed in full if the length of the given BitField object is less than BIT_FIELD_SIZE_MAX.
		bool	  operator ==(uint packedBits);
		bool	  operator !=(uint packedBits);
		BitField& operator =(uint packedBits);
		BitField& operator &=(uint packedBits);
		BitField& operator |=(uint packedBits);
		uint	  operator &(uint packedBits);
		uint	  operator |(uint packedBits);

	private:
		void Fill(bool value);
	};
}
