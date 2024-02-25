#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct FishInfo
{
	Pose Pose = Pose::Zero;
	int	angle;
	short destX;
	short destY;
	short destZ;
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
