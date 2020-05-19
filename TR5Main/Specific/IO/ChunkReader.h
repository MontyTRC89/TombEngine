#pragma once

#include <stdlib.h>
#include <memory>
#include <functional>
#include "ChunkId.h"
#include "LEB128.h"
#include "Streams.h"

using namespace std;

class ChunkReader
{
private:
	bool				m_isValid;
	ChunkId*			m_emptyChunk;
	BaseStream*			m_stream;

	int readInt32()
	{
		int value = 0;
		m_stream->Read(reinterpret_cast<char *>(&value), 4);
		return value;
	}

	short readInt16()
	{
		short value = 0;
		m_stream->Read(reinterpret_cast<char *>(&value), 2);
		return value;
	}

public:
	ChunkReader(int expectedMagicNumber, BaseStream* stream)
	{
		m_isValid = false;

		if (stream == NULL)
			return;

		m_stream = stream;

		// Check the magic number
		int magicNumber = readInt32();
		if (magicNumber != expectedMagicNumber)
			return;

		// TODO: future use for compression
		m_stream->Seek(4, SeekOrigin::CURRENT);
		m_emptyChunk = new ChunkId(NULL, 0);
		m_isValid = true;
	}

	~ChunkReader()
	{
		delete m_emptyChunk;
	}

	bool IsValid()
	{
		return m_isValid;
	}

	bool ReadChunks(bool(*func)(ChunkId* parentChunkId, int maxSize, int arg), int arg)
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
			int startPos = m_stream->GetCurrentPosition();

			chunkRecognized = func(chunkId, chunkSize, arg);
			int readDataCount = m_stream->GetCurrentPosition() - startPos;

			// Adjust _stream position if necessary
			if (readDataCount != chunkSize)
				m_stream->Seek(chunkSize - readDataCount, SeekOrigin::CURRENT);
		} while (true);

		return true;
	}

	bool ReadChunks(std::mem_fn<int()> func, int arg)
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
			int startPos = m_stream->GetCurrentPosition();

			chunkRecognized = func(chunkId, chunkSize, arg);
			int readDataCount = m_stream->GetCurrentPosition() - startPos;

			// Adjust _stream position if necessary
			if (readDataCount != chunkSize)
				m_stream->Seek(chunkSize - readDataCount, SeekOrigin::CURRENT);
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

	int ReadChunkInt32(__int64 length)
	{
		return LEB128::ReadInt32(m_stream);
	}

	unsigned int ReadChunkUInt32(__int64 length)
	{
		return LEB128::ReadUInt32(m_stream);
	}

	short ReadChunkInt16(__int64 length)
	{
		return LEB128::ReadInt16(m_stream);
	}

	unsigned short ReadChunkUInt16(__int64 length)
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
