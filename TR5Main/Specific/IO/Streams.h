#pragma once

enum SeekOrigin
{
	BEGIN,
	CURRENT
};

class BaseStream
{
public:
	virtual bool Read(char* buffer, int length) = 0;
	virtual bool Write(char* buffer, int length) = 0;
	virtual int GetCurrentPosition() = 0;
	virtual bool Seek(int seek, SeekOrigin origin) = 0;
	virtual bool IsEOF() = 0;
	virtual bool Close() = 0;
	
	bool ReadBytes(byte* value, int length)
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

	bool ReadInt16(short* value)
	{
		return Read(reinterpret_cast<char*>(value), 2);
	}

	bool ReadInt32(int* value)
	{
		return Read(reinterpret_cast<char*>(value), 4);
	}

	bool ReadFloat(float* value)
	{
		return Read(reinterpret_cast<char*>(value), 4);
	}

	bool ReadString(char** value)
	{
		int length;
		ReadInt32(&length);
		*value = (char*)malloc(length + 1);
		Read(*value, length);
		(*value)[length] = NULL;

		return true;
	}

	bool ReadString(string* value)
	{
		int length;
		ReadInt32(&length);
		char* buffer = (char*)malloc(length + 1);
		Read(buffer, length);
		buffer[length] = NULL;
		*value = string(buffer);
		free(buffer);

		return true;
	}

	bool ReadVector2(Vector2* value)
	{
		ReadFloat(&value->x);
		ReadFloat(&value->y);

		return true;
	}

	bool ReadVector3(Vector3* value)
	{
		ReadFloat(&value->x);
		ReadFloat(&value->y);
		ReadFloat(&value->z);

		return true;
	}

	bool ReadVector4(Vector4* value)
	{
		ReadFloat(&value->x);
		ReadFloat(&value->y);
		ReadFloat(&value->z);
		ReadFloat(&value->w);

		return true;
	}

	bool ReadQuaternion(Quaternion* value)
	{
		ReadFloat(&value->x);
		ReadFloat(&value->y);
		ReadFloat(&value->z);
		ReadFloat(&value->w);

		return true;
	}

	bool ReadBoundingBox(BoundingBox* value)
	{
		Vector3 minPos;
		Vector3 maxPos;

		ReadVector3(&minPos);
		ReadVector3(&maxPos);

		BoundingBox::CreateFromPoints(*value, minPos, maxPos);

		return true;
	}

	bool ReadBoundingSphere(BoundingSphere* sphere)
	{
		Vector3 center;
		float radius;

		ReadVector3(&center);
		ReadFloat(&radius);

		sphere->Center = center;
		sphere->Radius = radius;

		return true;
	}

	bool WriteBytes(byte* value, int length)
	{
		return Write(reinterpret_cast<char*>(value), length);
	}

	bool WriteByte(byte value)
	{
		return Write(reinterpret_cast<char*>(&value), 1);
	}

	bool WriteInt16(short value)
	{
		return Write(reinterpret_cast<char*>(&value), 2);
	}

	bool WriteInt32(int value)
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
		int length = (int)strlen(str);
		WriteInt32(length);
		Write(str, length);
		return true;
	}
};

class MemoryStream : public BaseStream {
private:
	char* m_startBuffer;
	char* m_buffer;
	int m_size;

public:
	MemoryStream(char* buffer, int size)
	{
		m_buffer = (char*)malloc(size);
		m_startBuffer = m_buffer;
		memcpy(m_buffer, buffer, size);
		m_size = size;
	}

	MemoryStream(int size)
	{
		m_buffer = (char*)malloc(size);
		m_startBuffer = m_buffer;
		m_size = size;
	}

	~MemoryStream()
	{
		free(m_startBuffer);
	}

	bool Read(char* buffer, int length)
	{
		memcpy(buffer, m_buffer, length);
		m_buffer += length;
		return true;
	}

	bool Write(char* buffer, int length)
	{
		memcpy(m_buffer, buffer, length);
		m_buffer += length;
		return true;
	}

	int GetCurrentPosition()
	{
		return (m_buffer - m_startBuffer);
	}

	bool Seek(int seek, SeekOrigin origin)
	{
		if (origin == SeekOrigin::BEGIN)
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
		int mode = 0;
		if (read)
			mode |= ifstream::binary | fstream::in;
		if (write)
			mode |= ofstream::binary | fstream::out | fstream::trunc;

		m_stream.open(fileName, mode);
		bool opened = m_stream.is_open();
	}

	~FileStream()
	{
		m_stream.close();
	}

	bool Read(char* buffer, int length)
	{
		m_stream.read(buffer, length);
		return true;
	}

	bool Write(char* buffer, int length)
	{
		m_stream.write(buffer, length);
		return true;
	}

	int GetCurrentPosition()
	{
		return (int)(m_stream.tellg());
	}

	bool Seek(int seek, SeekOrigin origin)
	{
		m_stream.seekg(seek, (origin == SeekOrigin::BEGIN ? m_stream.beg : m_stream.cur));
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