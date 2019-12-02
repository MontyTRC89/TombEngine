#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

#include "Streams.h"

extern char* LevelDataPtr;

typedef struct LEB128 {
	static const __int64 MaximumSize1Byte = 64L - 1;
	static const __int64 MaximumSize2Byte = 64L * 128 - 1;
	static const __int64 MaximumSize3Byte = 64L * 128 * 128 - 1;
	static const __int64 MaximumSize4Byte = 64L * 128 * 128 * 128 - 1;
	static const __int64 MaximumSize5Byte = 64L * 128 * 128 * 128 * 128 - 1;
	static const __int64 MaximumSize6Byte = 64L * 128 * 128 * 128 * 128 * 128 - 1;
	static const __int64 MaximumSize7Byte = 64L * 128 * 128 * 128 * 128 * 128 * 128 - 1;
	static const __int64 MaximumSize8Byte = 64L * 128 * 128 * 128 * 128 * 128 * 128 * 128 - 1;
	static const __int64 MaximumSize9Byte = 64L * 128 * 128 * 128 * 128 * 128 * 128 * 128 * 128 - 1;
	static const __int64 MaximumSize10Byte = UINT64_MAX;

	static __int64 ReadLong(BaseStream* stream)
	{
		__int64 result = 0;
		int currentShift = 0;

		byte currentByte;
		do
		{
			stream->Read(reinterpret_cast<char *>(&currentByte), 1);
			
			result |= (__int64)(currentByte & 0x7F) << currentShift;
			currentShift += 7;
		} while ((currentByte & 0x80) != 0);

		// Sign extend
		int shift = 64 - currentShift;
		if (shift > 0)
			result = (result << shift) >> shift;

		return result;
	}

	static int ReadInt32(BaseStream* stream)
	{
		__int64 value = ReadLong(stream);
		return (int)min(max(value, INT32_MIN), INT32_MAX);
	}

	static short ReadInt16(BaseStream* stream)
	{
		__int64 value = ReadLong(stream);
		return (short)min(max(value, INT16_MIN), INT16_MAX);
	}

	static byte ReadByte(BaseStream* stream)
	{
		__int64 value = ReadLong(stream);
		return (byte)min(max(value, 0), UINT8_MAX);
	}	

	static unsigned int ReadUInt32(BaseStream* stream)
	{
		__int64 value = ReadLong(stream);
		return (unsigned int)min(max(value, 0), UINT32_MAX);
	}

	static unsigned short ReadUInt16(BaseStream* stream)
	{
		__int64 value = ReadLong(stream);
		return (unsigned short)min(max(value, 0), UINT16_MAX);
	}

	static void Write(BaseStream* stream, __int64 value, __int64 maximumSize)
	{
		do
		{
			// Write byte
			byte currentByte = ((byte)(value & 0x7F));
			if (maximumSize >> 6 == 0 || maximumSize >> 6 == -1)
			{
				stream->WriteByte(currentByte);

				if (value >> 6 != 0 && value >> 6 != -1)
					throw "Unable to write integer because the available space overflowed.";

				return;
			}

			byte b = currentByte | 0x80;
			stream->WriteByte(b);

			// Move data to next byte
			value >>= 7;
			maximumSize >>= 7;
		} while (true);
	}

	static void Write(BaseStream* stream, __int64 value)
	{
		Write(stream, value, value);
	}

	static int GetLength(BaseStream* stream, __int64 value)
	{
		int length = 1;
		value >>= 6;
		while (value > 0)
		{
			value >>= 7;
			length += 1;
		}
		return length;
	}
};

