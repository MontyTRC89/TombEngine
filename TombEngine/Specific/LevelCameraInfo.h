#pragma once
#include <string>

#include "Math/Math.h"

using namespace TEN::Math;

struct LevelCameraInfo
{
	std::string Name  = {};
	short		Index = -1;

	Vector3i Position	= Vector3i::Zero;
	int		 RoomNumber = 0;
	int		 Flags		= 0;
	int		 Speed		= 1;

	LevelCameraInfo() {}

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
