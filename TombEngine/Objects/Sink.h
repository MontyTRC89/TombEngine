#pragma once
#include <string>

struct SinkInfo
{
	Vector3		Position = Vector3::Zero;
	int			Strength = 0;
	int			BoxIndex = 0;
	std::string Name	 = {};

	SinkInfo()
	{
	}

	SinkInfo(const Vector3& pos, int strength = 0, int boxIndex = 0)
	{
		this->Position = pos;
		this->Strength = strength;
		this->BoxIndex = boxIndex;
	}
};
