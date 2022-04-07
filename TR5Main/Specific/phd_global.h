#pragma once

struct Vector2Int
{
	int x;
	int y;

	Vector2Int()
	{
		this->x = 0;
		this->y = 0;
	}

	Vector2Int(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};

struct Vector3Int
{
	int x;
	int y;
	int z;

	Vector3Int operator +(Vector3Int pos)
	{
		return Vector3Int(x + pos.x, y + pos.y, z + pos.z);
	}

	Vector3Int operator -(Vector3Int pos)
	{
		return Vector3Int(x - pos.x, y - pos.y, z - pos.z);
	}

	Vector3Int operator *(Vector3Int pos)
	{
		return Vector3Int(x * pos.x, y * pos.y, z * pos.z);
	}
	
	Vector3Int operator *(float value)
	{
		return Vector3Int(x * value, y * value, z * value);
	}

	Vector3Int operator /(Vector3Int pos)
	{
		return Vector3Int((int)round(x / pos.x), (int)round(y / pos.y), (int)round(z / pos.z));
	}

	Vector3Int& operator +=(Vector3Int pos)
	{
		this->x += pos.x;
		this->y += pos.y;
		this->z += pos.z;
		return *this;
	}

	Vector3Int& operator -=(Vector3Int pos)
	{
		this->x -= pos.x;
		this->y -= pos.y;
		this->z -= pos.z;
		return *this;
	}

	Vector3Int& operator *=(Vector3Int pos)
	{
		this->x *= pos.x;
		this->y *= pos.y;
		this->z *= pos.z;
		return *this;
	}
	
	Vector3Int& operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	Vector3Int& operator /=(float value)
	{
		this->x = (int)round(x / value);
		this->y = (int)round(y / value);
		this->z = (int)round(z / value);
		return *this;
	}
	
	Vector3Int()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector3Int(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3 ToVector3()
	{
		return Vector3(x, y, z);
	}
};

struct Vector3Shrt
{
	short x;
	short y;
	short z;

	Vector3Shrt operator +(Vector3Shrt pos)
	{
		return Vector3Shrt(x + pos.x, y + pos.y, z + pos.z);
	}

	Vector3Shrt operator -(Vector3Shrt pos)
	{
		return Vector3Shrt(x - pos.x, y - pos.y, z - pos.z);
	}

	Vector3Shrt operator *(Vector3Shrt pos)
	{
		return Vector3Shrt(x * pos.x, y * pos.y, z * pos.z);
	}

	Vector3Shrt operator *(float value)
	{
		return Vector3Shrt(x * value, y * value, z * value);
	}

	Vector3Shrt operator /(Vector3Shrt pos)
	{
		return Vector3Shrt((short)round(x / pos.x), (short)round(y / pos.y), (short)round(z / pos.z));
	}

	Vector3Shrt& operator +=(Vector3Shrt pos)
	{
		this->x += pos.x;
		this->y += pos.y;
		this->z += pos.z;
		return *this;
	}

	Vector3Shrt& operator -=(Vector3Shrt pos)
	{
		this->x -= pos.x;
		this->y -= pos.y;
		this->z -= pos.z;
		return *this;
	}

	Vector3Shrt& operator *=(Vector3Shrt pos)
	{
		this->x *= pos.x;
		this->y *= pos.y;
		this->z *= pos.z;
		return *this;
	}

	Vector3Shrt& operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	Vector3Shrt& operator /=(float value)
	{
		this->x = (short)round(x / value);
		this->y = (short)round(y / value);
		this->z = (short)round(z / value);
		return *this;
	}

	Vector3Shrt()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector3Shrt(short x, short y, short z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3 ToVector3()
	{
		return Vector3(x, y, z);
	}
};

struct PHD_3DPOS
{
	Vector3Int Position;
	Vector3Shrt Orientation;

	PHD_3DPOS()
	{
		this->Position = Vector3Int();
		this->Orientation = Vector3Shrt();
	}

	PHD_3DPOS(int xPos, int yPos, int zPos)
	{
		this->Position.x = xPos;
		this->Position.y = yPos;
		this->Position.z = zPos;
		this->Orientation = Vector3Shrt();
	}

	PHD_3DPOS(short xOrient, short yOrient, short zOrient)
	{
		this->Position = Vector3Int();
		this->Orientation.x = xOrient;
		this->Orientation.y = yOrient;
		this->Orientation.z = zOrient;
	}

	PHD_3DPOS(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient)
	{
		this->Position.x = xPos;
		this->Position.y = yPos;
		this->Position.z = zPos;
		this->Orientation.x = xOrient;
		this->Orientation.y = yOrient;
		this->Orientation.z = zOrient;
	}
};

struct GameVector
{
	int x;
	int y;
	int z;
	int boxNumber;
	short roomNumber;

	GameVector()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GameVector(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GameVector(int xpos, int ypos, int zpos, short roomNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = roomNumber;
		this->boxNumber = 0;
	}

	GameVector(int xpos, int ypos, int zpos, short roomNumber, short boxNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = roomNumber;
		this->boxNumber = boxNumber;
	}
};

struct LEVEL_CAMERA_INFO
{
	int x;
	int y;
	int z;
	int roomNumber;
	int flags;
	int speed;
	std::string luaName;

	LEVEL_CAMERA_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->roomNumber = 0;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos, short room)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = room;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos, short flags, bool isFlags) // use isFlags to use flag instead of newdata !
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->flags = flags;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos, short room, short newflags)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = room;
		this->flags = newflags;
		this->speed = 1;
	}
};

struct SINK_INFO
{
	int x;
	int y;
	int z;
	int strength;
	int boxIndex;
	std::string luaName;

	SINK_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->strength = 0;
		this->boxIndex = 0;
	}

	SINK_INFO(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->strength = 0;
		this->boxIndex = 0;
	}

	SINK_INFO(int xpos, int ypos, int zpos, short strength)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->strength = strength;
		this->boxIndex = 0;
	}

	SINK_INFO(int xpos, int ypos, int zpos, short strength, short boxIndex)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->strength = strength;
		this->boxIndex = boxIndex;
	}
};

struct SOUND_SOURCE_INFO
{
	int x;
	int y;
	int z;
	int soundId;
	int flags;
	std::string luaName;

	SOUND_SOURCE_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->soundId = 0;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->soundId = 0;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xpos, int ypos, int zpos, short soundId)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->soundId = soundId;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xpos, int ypos, int zpos, short soundId, short newflags)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->soundId = soundId;
		this->flags = newflags;
	}
};

struct VECTOR
{
	int vx;
	int vy;
	int vz;
	int pad;
};

struct SVECTOR
{
	short vx;
	short vy;
	short vz;
	short pad;
};

struct CVECTOR
{
	byte r;
	byte g;
	byte b;
	byte cd;
};

struct TR_VERTEX
{
	int x;
	int y;
	int z;
};

enum MATRIX_ARRAY_VALUE
{
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23
};

struct MATRIX3D
{
	short m00;
	short m01;
	short m02;
	short m10;
	short m11;
	short m12;
	short m20;
	short m21;
	short m22;
	short pad;
	int tx;
	int ty;
	int tz;
};

struct BOUNDING_BOX
{
	short X1;
	short X2;
	short Y1;
	short Y2;
	short Z1;
	short Z2;
};
