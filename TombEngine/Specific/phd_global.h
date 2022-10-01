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
