#pragma once

struct FishInfo
{
	Vector3Int Offset;
	float Angle;
	short DestY;
	float AngleAdd;
	unsigned int Velocity;
	unsigned int Acceleration;
	unsigned int Swim;
};

struct FishLeaderInfo
{
	float Angle;
	unsigned int Velocity;
	bool On;
	short AngleTime;
	short VelocityTime;
	Vector3Int Range;
};
