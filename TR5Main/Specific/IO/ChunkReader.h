#pragma once

#include <stdlib.h>
#include <memory>

#include "ChunkId.h"
#include "LEB128.h"
#include "Streams.h"

class ChunkReader
{
private:
	bool				m_isValid;
	ChunkId*			m_emptyChunk;
	BaseStream*			m_stream;

	__int32 readInt32()
	{
		__int32 value = 0;
		m_stream->Read(reinterpret_cast<char *>(&value), 4);
		return value;
	}

	__int16 readInt16()
	{
		__int16 value = 0;
		m_stream->Read(reinterpret_cast<char *>(&value), 2);
		return value;
	}

public:
	ChunkReader(__int32 expectedMagicNumber, BaseStream* stream)
	{
		m_isValid = false;

		if (stream == NULL)
			return;

		m_stream = stream;

		// Check the magic number
		__int32 magicNumber = readInt32();
		if (magicNumber != expectedMagicNumber)
			return;

		// TODO: future use for compression
		m_stream->Seek(4, SEEK_ORIGIN::CURRENT);

		m_emptyChunk = new ChunkId(NULL, 0);

		m_isValid = true;
	}

	~ChunkReader()
	{
		delete m_emptyChunk;
	}

	bool ChunkReader::IsValid()
	{
		return m_isValid;
	}

	bool ChunkReader::ReadChunks(bool(*func)(ChunkId* parentChunkId, __int32 maxSize))
	{
		do
		{
			ChunkId* chunkId = ChunkId::FromStream(m_stream);
			if (chunkId->EqualsTo(m_emptyChunk)) // End reached
				break;

			// Read up to a 64 bit number for the chunk size
			__int64 chunkSize = LEB128::ReadLong(m_stream);

			// Try loading chunk content
			bool chunkRecognized = false;
			__int32 startPos = m_stream->GetCurrentPosition();

			chunkRecognized = func(chunkId, chunkSize);
			__int32 readDataCount = m_stream->GetCurrentPosition() - startPos;

			// Adjust _stream position if necessary
			if (readDataCount != chunkSize)
				m_stream->Seek(chunkSize, SEEK_ORIGIN::CURRENT);
		} while (true);

		return true;
	}

	char* ReadChunkArrayOfBytes(__int64 length)
	{
		char* value = (char*)malloc(length);
		m_stream->Read(value, length);
		return value;
	}

	bool ReadChunkBool(__int64 length)
	{
		return (LEB128::ReadByte(m_stream) != 0);
	}

	__int64 ReadChunkLong(__int64 length)
	{
		return LEB128::ReadLong(m_stream);
	}

	__int32 ReadChunkInt32(__int64 length)
	{
		return LEB128::ReadInt32(m_stream);
	}

	unsigned __int32 ReadChunkUInt32(__int64 length)
	{
		return LEB128::ReadUInt32(m_stream);
	}

	__int16 ReadChunkInt16(__int64 length)
	{
		return LEB128::ReadInt16(m_stream);
	}

	unsigned __int16 ReadChunkUInt16(__int64 length)
	{
		return LEB128::ReadUInt16(m_stream);
	}

	byte ReadChunkByte(__int64 length)
	{
		return LEB128::ReadByte(m_stream);
	}

	char* ReadChunkString(long length)
	{
		char* value = (char*)malloc(length);
		memcpy(value, LevelDataPtr, length);
		return value;
	}

	BaseStream* GetRawStream()
	{
		return m_stream;
	}
};

