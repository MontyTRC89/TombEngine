#pragma once

using std::string;

struct PoseData;

// TODO: Clean this out. Move every struct into an appropriate file. Let's stop dumping everything in here.

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

struct LevelCameraInfo
{
	Vector3i Position	= Vector3i::Zero;
	int		 RoomNumber	= 0;
	int		 Flags		= 0;
	int		 Speed		= 1;
	string	 LuaName	= "";

	LevelCameraInfo()
	{
	}

	LevelCameraInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	LevelCameraInfo(int xPos, int yPos, int zPos, short roomNumber)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->RoomNumber = roomNumber;
	}

	// Use isFlags to use flag instead of new data.
	LevelCameraInfo(int xPos, int yPos, int zPos, short flags, bool isFlags)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Flags = flags;
	}

	LevelCameraInfo(int xPos, int yPos, int zPos, short roomNumber, short newflags)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->RoomNumber = roomNumber;
		this->Flags = newflags;
	}
};

struct SinkInfo
{
	Vector3i Position = Vector3i::Zero;
	int		 Strength = 0;
	int		 BoxIndex = 0;
	string	 LuaName  = "";

	SinkInfo()
	{
	}

	SinkInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	SinkInfo(int xPos, int yPos, int zPos, short strength)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Strength = strength;
	}

	SinkInfo(int xPos, int yPos, int zPos, short strength, short boxIndex)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->Strength = strength;
		this->BoxIndex = boxIndex;
	}
};

struct SoundSourceInfo
{
	Vector3i Position = Vector3i::Zero;
	int		 SoundID  = 0;
	int		 Flags	  = 0;
	string	 LuaName  = "";

	SoundSourceInfo()
	{
	}

	SoundSourceInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	SoundSourceInfo(int xPos, int yPos, int zPos, short soundID)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->SoundID = soundID;
	}

	SoundSourceInfo(int xPos, int yPos, int zPos, short soundID, short newflags)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
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

	BOUNDING_BOX operator +(const PoseData& pose)
	{
		auto newBox = *this;
		newBox.X1 += pose.Position.x;
		newBox.X2 += pose.Position.x;
		newBox.Y1 += pose.Position.y;
		newBox.Y2 += pose.Position.y;
		newBox.Z1 += pose.Position.z;
		newBox.Z2 += pose.Position.z;
		return newBox;
	}

	BOUNDING_BOX operator *(float scale)
	{
		auto newBox = *this;
		newBox.X1 *= scale;
		newBox.X2 *= scale;
		newBox.Y1 *= scale;
		newBox.Y2 *= scale;
		newBox.Z1 *= scale;
		newBox.Z2 *= scale;
		return newBox;
	}
};
