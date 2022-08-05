#pragma once

// TODO: Possibly rename these vector structs to more common standards later:
// Vector2i, Vector3i, Vector3s. -- Sezz 2022.07.23
struct Vector2Int
{
	int x;
	int y;

	static const Vector2Int Zero;

	Vector2Int()
	{
		*this = Vector2Int::Zero;
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

	static const Vector3Int Zero;

	Vector3Int()
	{
		*this = Vector3Int::Zero;
	}

	Vector3Int(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3Int(Vector3 v)
	{
		this->x = int(v.x);
		this->y = int(v.y);
		this->z = int(v.z);
	}

	Vector3 ToVector3()
	{
		return Vector3(x, y, z);
	}

	bool operator ==(Vector3Int vector)
	{
		return (x == vector.x && y == vector.y && z == vector.z);
	}

	bool operator !=(Vector3Int vector)
	{
		return (x != vector.x || y != vector.y || z != vector.z);
	}

	Vector3Int operator =(Vector3Int vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		this->z = vector.z;
		return *this;
	}

	Vector3Int operator +(Vector3Int vector)
	{
		return Vector3Int(x + vector.x, y + vector.y, z + vector.z);
	}

	Vector3Int operator -(Vector3Int vector)
	{
		return Vector3Int(x - vector.x, y - vector.y, z - vector.z);
	}

	Vector3Int operator *(Vector3Int vector)
	{
		return Vector3Int(x * vector.x, y * vector.y, z * vector.z);
	}

	Vector3Int operator *(float value)
	{
		return Vector3Int((int)round(x * value), (int)round(y * value), (int)round(z * value));
	}

	Vector3Int operator /(float value)
	{
		return Vector3Int((int)round(x / value), (int)round(y / value), (int)round(z / value));
	}

	Vector3Int& operator +=(const Vector3Int vector)
	{
		*this = *this + vector;
		return *this;
	}

	Vector3Int& operator -=(Vector3Int vector)
	{
		*this = *this - vector;
		return *this;
	}

	Vector3Int& operator *=(Vector3Int vector)
	{
		*this = *this * vector;
		return *this;
	}

	Vector3Int& operator *=(float value)
	{
		*this = *this * value;
		return *this;
	}

	Vector3Int& operator /=(float value)
	{
		*this = *this / value;
		return *this;
	}
};

struct Vector3Shrt
{
	short x;
	short y;
	short z;

	static const Vector3Shrt Zero;

	Vector3Shrt()
	{
		*this = Vector3Shrt::Zero;
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

	bool operator ==(Vector3Shrt vector)
	{
		return (x == vector.x && y == vector.y && z == vector.z);
	}

	bool operator !=(Vector3Shrt vector)
	{
		return (x != vector.x || y != vector.y || z != vector.z);
	}

	Vector3Shrt operator =(Vector3Shrt vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		this->z = vector.z;
		return *this;
	}

	Vector3Shrt operator +(Vector3Shrt vector)
	{
		return Vector3Shrt(x + vector.x, y + vector.y, z + vector.z);
	}

	Vector3Shrt operator -(Vector3Shrt vector)
	{
		return Vector3Shrt(x - vector.x, y - vector.y, z - vector.z);
	}

	Vector3Shrt operator *(Vector3Shrt vector)
	{
		return Vector3Shrt(x * vector.x, y * vector.y, z * vector.z);
	}

	Vector3Shrt operator *(float value)
	{
		return Vector3Shrt((short)round(x * value), (short)round(y * value), (short)round(z * value));
	}

	Vector3Shrt operator /(float value)
	{
		return Vector3Shrt((short)round(x / value), (short)round(y / value), (short)round(z / value));
	}

	Vector3Shrt& operator +=(const Vector3Shrt vector)
	{
		*this = *this + vector;
		return *this;
	}

	Vector3Shrt& operator -=(Vector3Shrt vector)
	{
		*this = *this - vector;
		return *this;
	}

	Vector3Shrt& operator *=(Vector3Shrt vector)
	{
		*this = *this * vector;
		return *this;
	}

	Vector3Shrt& operator *=(float value)
	{
		*this = *this * value;
		return *this;
	}

	Vector3Shrt& operator /=(float value)
	{
		*this = *this / value;
		return *this;
	}
};

struct RendererRectangle
{
	int left;
	int top;
	int right;
	int bottom;

	RendererRectangle()
	{
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}

	RendererRectangle(int left, int top, int right, int bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}
};

// TODO: Rename to PoseData (or something else that specifically describes a position + orientation representation, if we prefer).
// This struct has changed vastly and the old name is no longer appropriate. -- Sezz 2022.07.23
struct PHD_3DPOS
{
	Vector3Int	Position;
	Vector3Shrt Orientation;

	PHD_3DPOS()
	{
		this->Position = Vector3Int::Zero;
		this->Orientation = Vector3Shrt::Zero;
	}

	PHD_3DPOS(Vector3Int pos)
	{
		this->Position = pos;
		this->Orientation = Vector3Shrt::Zero;
	}

	PHD_3DPOS(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = Vector3Shrt::Zero;
	}

	PHD_3DPOS(Vector3Shrt orient)
	{
		this->Position = Vector3Int::Zero;
		this->Orientation = orient;
	}

	PHD_3DPOS(short xOrient, short yOrient, short zOrient)
	{
		this->Position = Vector3Int::Zero;
		this->Orientation = Vector3Shrt(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS(Vector3Int pos, Vector3Shrt orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	PHD_3DPOS(Vector3Int pos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = pos;
		this->Orientation = Vector3Shrt(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS(int xPos, int yPos, int zPos, Vector3Shrt orient)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = orient;
	}

	PHD_3DPOS(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = Vector3Shrt(xOrient, yOrient, zOrient);
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

BOUNDING_BOX operator+(BOUNDING_BOX const& box, PHD_3DPOS const& vec);
