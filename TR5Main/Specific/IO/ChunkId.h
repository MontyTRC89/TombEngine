#pragma once
#include "LEB128.h"
#include "Streams.h"


typedef struct ChunkId
{
private:
	byte*		m_chunkBytes;
	int		m_length;

public:
	ChunkId(char* bytes, int length);

	~ChunkId();

	static ChunkId* FromString(const char* str);

	static ChunkId* FromString(std::string* str);

	static ChunkId* FromStream(BaseStream* stream);

	void ToStream(BaseStream* stream);

	byte* GetBytes();

	int GetLength();

	bool EqualsTo(ChunkId* other);
};