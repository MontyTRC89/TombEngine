#include "framework.h"
#include "ChunkId.h"
using std::string;
inline ChunkId::ChunkId(char* bytes, int length) {
	if (length == 0) {
		m_chunkBytes = NULL;
		m_length = 0;
	} else {
		m_chunkBytes = (byte*)malloc(length);
		memcpy(m_chunkBytes, bytes, length);
		m_length = length;
	}
}

inline ChunkId::~ChunkId() {
	if (m_chunkBytes != NULL)
		delete m_chunkBytes;
}

inline ChunkId* ChunkId::FromString(const char* str) {
	return new ChunkId((char*)str, strlen(str));
}

inline ChunkId* ChunkId::FromString(string* str) {
	return new ChunkId((char*)str->c_str(), str->length());
}

inline ChunkId* ChunkId::FromStream(BaseStream* stream) {
	int idLength = LEB128::ReadInt32(stream);
	char* buffer = (char*)malloc(idLength);
	stream->Read(buffer, idLength);
	ChunkId* chunk = new ChunkId(buffer, idLength);
	free(buffer);
	return chunk;
}

inline void ChunkId::ToStream(BaseStream* stream) {
	LEB128::Write(stream, m_length);
	stream->WriteBytes(m_chunkBytes, m_length);
}

inline byte* ChunkId::GetBytes() {
	return m_chunkBytes;
}

inline int ChunkId::GetLength() {
	return m_length;
}

inline bool ChunkId::EqualsTo(ChunkId* other) {
	if (m_length != other->GetLength())
		return false;
	return (strncmp((const char*)m_chunkBytes, (const char*)other->GetBytes(), m_length) == 0);
}
