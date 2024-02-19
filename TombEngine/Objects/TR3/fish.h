#pragma once

struct FishInfo
{
	Pose Pose;
	int	angle;
	short destY;
	short angAdd;
	int speed;
	int acc;
	int swim;
};

struct FishLeaderInfo
{
	short angle;
	unsigned char speed;
	unsigned char on;
	short angleTime;
	short speedTime;
	short xRange, yRange, zRange;
};
