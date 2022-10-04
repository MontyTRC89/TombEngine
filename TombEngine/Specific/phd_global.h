#pragma once

using std::string;

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

	Vector3Int(Vector3 vector)
	{
		this->x = int(vector.x);
		this->y = int(vector.y);
		this->z = int(vector.z);
	}

	static float Distance(const Vector3Int& origin, const Vector3Int& target)
	{
		return Vector3::Distance(origin.ToVector3(), target.ToVector3());
	}
	
	static float DistanceSquared(const Vector3Int& origin, const Vector3Int& target)
	{
		return Vector3::DistanceSquared(origin.ToVector3(), target.ToVector3());
	}

	Vector3 ToVector3() const
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

	Vector3Int operator /(float value) const
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

	Vector3 ToVector3() const
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

	GameVector(int xPos, int yPos, int zPos)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GameVector(int xPos, int yPos, int zPos, short roomNumber)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = roomNumber;
		this->boxNumber = 0;
	}

	GameVector(int xPos, int yPos, int zPos, short roomNumber, short boxNumber)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = roomNumber;
		this->boxNumber = boxNumber;
	}

	GameVector(const Vector3Int& pos)
	{
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GameVector(const Vector3Int& pos, short roomNumber)
	{
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
		this->roomNumber = roomNumber;
		this->boxNumber = 0;
	}
};

struct LevelCameraInfo
{
	Vector3Int Position = Vector3Int::Zero;
	int RoomNumber		= 0;
	int Flags			= 0;
	int Speed			= 1;
	string LuaName		= "";

	LevelCameraInfo()
	{
	}

	LevelCameraInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
	}

	LevelCameraInfo(int xPos, int yPos, int zPos, short roomNumber)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->RoomNumber = roomNumber;
	}

	// Use isFlags to use flag instead of new data.
	LevelCameraInfo(int xPos, int yPos, int zPos, short flags, bool isFlags)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Flags = flags;
	}

	LevelCameraInfo(int xPos, int yPos, int zPos, short roomNumber, short newflags)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->RoomNumber = roomNumber;
		this->Flags = newflags;
	}
};

struct SinkInfo
{
	Vector3Int Position = Vector3Int::Zero;
	int		   Strength = 0;
	int		   BoxIndex = 0;
	string	   LuaName	= "";

	SinkInfo()
	{
	}

	SinkInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
	}

	SinkInfo(int xPos, int yPos, int zPos, short strength)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Strength = strength;
	}

	SinkInfo(int xPos, int yPos, int zPos, short strength, short boxIndex)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Strength = strength;
		this->BoxIndex = boxIndex;
	}
};

struct SoundSourceInfo
{
	Vector3Int Position = Vector3Int::Zero;
	int		   SoundID	= 0;
	int		   Flags	= 0;
	string	   LuaName	= "";

	SoundSourceInfo()
	{
	}

	SoundSourceInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
	}

	SoundSourceInfo(int xPos, int yPos, int zPos, short soundID)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->SoundID = soundID;
	}

	SoundSourceInfo(int xPos, int yPos, int zPos, short soundID, short newflags)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->SoundID = soundID;
		this->Flags = newflags;
	}
};

struct VECTOR
{
	int vx;
	int vy;
	int vz;
	int pad;
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

	int Height() { return abs(Y2 - Y1); }
};

BOUNDING_BOX operator+(const BOUNDING_BOX& box, const PHD_3DPOS& pose);
BOUNDING_BOX operator*(const BOUNDING_BOX& box, const float scale);
