#pragma once

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

	LEVEL_CAMERA_INFO(int xPos, int yPos, int zPos)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = 0;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xPos, int yPos, int zPos, short roomNumber)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = roomNumber;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xPos, int yPos, int zPos, short flags, bool isFlags) // Use isFlags to use flag instead of new data.
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = 0;
		this->flags = flags;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xPos, int yPos, int zPos, short roomNumber, short newflags)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->roomNumber = roomNumber;
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

	SINK_INFO(int xPos, int yPos, int zPos)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->strength = 0;
		this->boxIndex = 0;
	}

	SINK_INFO(int xPos, int yPos, int zPos, short strength)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->strength = strength;
		this->boxIndex = 0;
	}

	SINK_INFO(int xPos, int yPos, int zPos, short strength, short boxIndex)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
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

	SOUND_SOURCE_INFO(int xPos, int yPos, int zPos)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->soundId = 0;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xPos, int yPos, int zPos, short soundID)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->soundId = soundID;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xPos, int yPos, int zPos, short soundID, short newFlags)
	{
		this->x = xPos;
		this->y = yPos;
		this->z = zPos;
		this->soundId = soundID;
		this->flags = newFlags;
	}
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
