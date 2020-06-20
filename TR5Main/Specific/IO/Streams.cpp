#include "framework.h"
#include "Streams.h"
using std::ios;
using std::string;
using std::ifstream;
using std::fstream;
using std::ofstream;
inline bool BaseStream::ReadBytes(byte* value, int length) {
	return Read(reinterpret_cast<char*>(value), length);
}

inline bool BaseStream::ReadByte(byte* value) {
	return Read(reinterpret_cast<char*>(value), 1);
}

inline bool BaseStream::ReadBool(bool* value) {
	return Read(reinterpret_cast<char*>(value), 1);
}

inline bool BaseStream::ReadInt16(short* value) {
	return Read(reinterpret_cast<char*>(value), 2);
}

inline bool BaseStream::ReadInt32(int* value) {
	return Read(reinterpret_cast<char*>(value), 4);
}

inline bool BaseStream::ReadFloat(float* value) {
	return Read(reinterpret_cast<char*>(value), 4);
}

inline bool BaseStream::ReadString(char** value) {
	int length;
	ReadInt32(&length);
	*value = (char*)malloc(length + 1);
	Read(*value, length);
	(*value)[length] = NULL;

	return true;
}

inline bool BaseStream::ReadString(string* value) {
	int length;
	ReadInt32(&length);
	char* buffer = (char*)malloc(length + 1);
	Read(buffer, length);
	buffer[length] = NULL;
	*value = string(buffer);
	free(buffer);

	return true;
}

inline bool BaseStream::ReadVector2(Vector2* value) {
	ReadFloat(&value->x);
	ReadFloat(&value->y);

	return true;
}

inline bool BaseStream::ReadVector3(Vector3* value) {
	ReadFloat(&value->x);
	ReadFloat(&value->y);
	ReadFloat(&value->z);

	return true;
}

inline bool BaseStream::ReadVector4(Vector4* value) {
	ReadFloat(&value->x);
	ReadFloat(&value->y);
	ReadFloat(&value->z);
	ReadFloat(&value->w);

	return true;
}

inline bool BaseStream::ReadQuaternion(Quaternion* value) {
	ReadFloat(&value->x);
	ReadFloat(&value->y);
	ReadFloat(&value->z);
	ReadFloat(&value->w);

	return true;
}

inline bool BaseStream::ReadBoundingBox(BoundingBox* value) {
	Vector3 minPos;
	Vector3 maxPos;

	ReadVector3(&minPos);
	ReadVector3(&maxPos);

	BoundingBox::CreateFromPoints(*value, minPos, maxPos);

	return true;
}

inline bool BaseStream::ReadBoundingSphere(BoundingSphere* sphere) {
	Vector3 center;
	float radius;

	ReadVector3(&center);
	ReadFloat(&radius);

	sphere->Center = center;
	sphere->Radius = radius;

	return true;
}

inline bool BaseStream::WriteBytes(byte* value, int length) {
	return Write(reinterpret_cast<char*>(value), length);
}

inline bool BaseStream::WriteByte(byte value) {
	return Write(reinterpret_cast<char*>(&value), 1);
}

inline bool BaseStream::WriteInt16(short value) {
	return Write(reinterpret_cast<char*>(&value), 2);
}

inline bool BaseStream::WriteInt32(int value) {
	return Write(reinterpret_cast<char*>(&value), 4);
}

inline bool BaseStream::WriteBool(bool value) {
	return Write(reinterpret_cast<char*>(&value), 1);
}

inline bool BaseStream::WriteFloat(float value) {
	return Write(reinterpret_cast<char*>(&value), 4);
}

inline bool BaseStream::WriteString(char* str) {
	int length = (int)strlen(str);
	WriteInt32(length);
	Write(str, length);
	return true;
}

inline MemoryStream::MemoryStream(char* buffer, int size) {
	m_buffer = (char*)malloc(size);
	m_startBuffer = m_buffer;
	memcpy(m_buffer, buffer, size);
	m_size = size;
}

inline MemoryStream::MemoryStream(int size) {
	m_buffer = (char*)malloc(size);
	m_startBuffer = m_buffer;
	m_size = size;
}

inline MemoryStream::~MemoryStream() {
	free(m_startBuffer);
}

inline bool MemoryStream::Read(char* buffer, int length) {
	memcpy(buffer, m_buffer, length);
	m_buffer += length;
	return true;
}

inline bool MemoryStream::Write(char* buffer, int length) {
	memcpy(m_buffer, buffer, length);
	m_buffer += length;
	return true;
}

inline int MemoryStream::GetCurrentPosition() {
	return (m_buffer - m_startBuffer);
}

inline bool MemoryStream::Seek(int seek, SeekOrigin origin) {
	if (origin == SeekOrigin::BEGIN)
		m_buffer = m_startBuffer + seek;
	else
		m_buffer += seek;
	return true;
}

inline bool MemoryStream::IsEOF() {
	return (GetCurrentPosition() > m_size);
}

inline bool MemoryStream::Close() {
	return true;
}

inline FileStream::FileStream(char* fileName, bool read, bool write) {
	int mode = 0;
	if (read)
		mode |= ifstream::binary | fstream::in;
	if (write)
		mode |= ofstream::binary | fstream::out | fstream::trunc;

	m_stream.open(fileName, mode);
	bool opened = m_stream.is_open();
}

inline FileStream::~FileStream() {
	m_stream.close();
}

inline bool FileStream::Read(char* buffer, int length) {
	m_stream.read(buffer, length);
	return true;
}

inline bool FileStream::Write(char* buffer, int length) {
	m_stream.write(buffer, length);
	return true;
}

inline int FileStream::GetCurrentPosition() {
	return (int)(m_stream.tellg());
}

inline bool FileStream::Seek(int seek, SeekOrigin origin) {
	m_stream.seekg(seek, (origin == SeekOrigin::BEGIN ? m_stream.beg : m_stream.cur));
	return true;
}

inline bool FileStream::IsEOF() {
	return (m_stream.eof());
}

inline bool FileStream::Close() {
	m_stream.flush();
	m_stream.close();
	return true;
}
