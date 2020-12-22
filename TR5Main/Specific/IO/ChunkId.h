#pragma once
#include "LEB128.h"
#include "Streams.h"
#include <memory>

struct ChunkId
{
private:
	byte*		m_chunkBytes;
	int		m_length;

public:
	ChunkId(char* bytes, int length);

	~ChunkId();

	static std::unique_ptr<ChunkId> FromString(const char* str);

	static std::unique_ptr<ChunkId> FromString(std::string* str);

	static std::unique_ptr<ChunkId> FromStream(BaseStream* stream);

	void ToStream(BaseStream* stream);

	byte* GetBytes();

	int GetLength();

	bool EqualsTo(ChunkId* other);
};