#pragma once

#include <istream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <stdlib.h>

using namespace std;

enum SEEK_ORIGIN {
	BEGIN,
	CURRENT
};

class BaseStream {
public:
	virtual bool Read(char* buffer, __int32 length) = 0;
	virtual bool Write(char* buffer, __int32 length) = 0;
	virtual __int32 GetCurrentPosition() = 0;
	virtual bool Seek(__int32 seek, SEEK_ORIGIN origin) = 0;
	virtual bool IsEOF() = 0;
	virtual bool Close() = 0;
	
	bool ReadBytes(byte* value, __int32 length)
	{
		return Read(reinterpret_cast<char*>(value), length);
	}

	bool ReadByte(byte* value)
	{
		return Read(reinterpret_cast<char*>(value), 1);
	}

	bool ReadBool(bool* value)
	{
		return Read(reinterpret_cast<char*>(value), 1);
	}

	bool ReadInt16(__int16* value)
	{
		return Read(reinterpret_cast<char*>(value), 2);
	}

	bool ReadInt32(__int32* value)
	{
		return Read(reinterpret_cast<char*>(value), 4);
	}

	bool ReadFloat(float* value)
	{
		return Read(reinterpret_cast<char*>(value), 4);
	}

	bool ReadString(char** value)
	{
		__int32 length;
		ReadInt32(&length);
		*value = (char*)malloc(length + 1);
		Read(*value, length);
		(*value)[length] = NULL;

		return true;
	}

	bool WriteBytes(byte* value, __int32 length)
	{
		return Write(reinterpret_cast<char*>(value), length);
	}

	bool WriteByte(byte value)
	{
		return Write(reinterpret_cast<char*>(&value), 1);
	}

	bool WriteInt16(__int16 value)
	{
		return Write(reinterpret_cast<char*>(&value), 2);
	}

	bool WriteInt32(__int32 value)
	{
		return Write(reinterpret_cast<char*>(&value), 4);
	}

	bool WriteBool(bool value)
	{
		return Write(reinterpret_cast<char*>(&value), 1);
	}

	bool WriteFloat(float value)
	{
		return Write(reinterpret_cast<char*>(&value), 4);
	}

	bool WriteString(char* str)
	{
		__int32 length = (__int32)strlen(str);
		WriteInt32(length);
		Write(str, length);
		return true;
	}
};

class MemoryStream : public BaseStream {
private:
	char* m_startBuffer;
	char* m_buffer;
	__int32 m_size;

public:
	MemoryStream(char* buffer, __int32 size)
	{
		m_buffer = (char*)malloc(size);
		m_startBuffer = m_buffer;
		memcpy(m_buffer, buffer, size);
		m_size = size;
	}

	MemoryStream(__int32 size)
	{
		m_buffer = (char*)malloc(size);
		m_startBuffer = m_buffer;
		m_size = size;
	}

	~MemoryStream()
	{
		free(m_startBuffer);
	}

	bool Read(char* buffer, __int32 length)
	{
		memcpy(buffer, m_buffer, length);
		m_buffer += length;
		return true;
	}

	bool Write(char* buffer, __int32 length)
	{
		memcpy(m_buffer, buffer, length);
		m_buffer += length;
		return true;
	}

	__int32 GetCurrentPosition()
	{
		return (m_buffer - m_startBuffer);
	}

	bool Seek(__int32 seek, SEEK_ORIGIN origin)
	{
		if (origin == SEEK_ORIGIN::BEGIN)
			m_buffer = m_startBuffer + seek;
		else
			m_buffer += seek;
		return true;
	}

	bool IsEOF()
	{
		return (GetCurrentPosition() > m_size);
	}

	bool Close()
	{
		return true;
	}
};

class FileStream : public BaseStream {
private:
	fstream m_stream;

public:
	FileStream(char* fileName, bool read, bool write)
	{
		__int32 mode = 0;
		if (read)
			mode |= ifstream::binary | fstream::in;
		if (write)
			mode |= ofstream::binary | fstream::out | fstream::trunc;

		m_stream.open(fileName, mode);
	}

	~FileStream()
	{
		m_stream.close();
	}

	bool Read(char* buffer, __int32 length)
	{
		printf("Pos: %d\n", (__int32)(m_stream.tellg()));
		m_stream.read(buffer, length);
		return true;
	}

	bool Write(char* buffer, __int32 length)
	{
		m_stream.write(buffer, length);
		return true;
	}

	__int32 GetCurrentPosition()
	{
		return (__int32)(m_stream.tellg());
	}

	bool Seek(__int32 seek, SEEK_ORIGIN origin)
	{
		m_stream.seekg(seek, (origin == SEEK_ORIGIN::BEGIN ? m_stream.beg : m_stream.cur));
		return true;
	}

	bool IsEOF()
	{
		return (m_stream.eof());
	}

	bool Close()
	{
		m_stream.flush();
		m_stream.close();
		return true;
	}
};