#pragma once
#include "Math/Math.h"

using std::string;

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
