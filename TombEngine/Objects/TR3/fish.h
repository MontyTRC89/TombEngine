#pragma once

struct FishInfo
{
	Pose Pose;
	int	angle;
	short destY;
	short angAdd;
	unsigned char speed;
	unsigned char acc;
	unsigned char swim;
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
