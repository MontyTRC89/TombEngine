#pragma once
#include <string>

#include "Math/Math.h"

using namespace TEN::Math;
using std::string;

struct SinkInfo
{
	Vector3i Position = Vector3i::Zero;
	int		 Strength = 0;
	int		 BoxIndex = 0;
	string	 LuaName = "";

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
