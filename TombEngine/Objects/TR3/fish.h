#pragma once

struct FishInfo
{
	Vector3Int Offset;
	short Angle;
	short DestY;
	float AngleAdd;
	unsigned int Velocity;
	unsigned int Acceleration;
	unsigned int Swim;
};

struct FishLeaderInfo
{
	short Angle;
	unsigned int Velocity;
	bool On;
	short AngleTime;
	short VelocityTime;
	Vector3Int Range;
};
