#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct FishInfo
{
	Pose Pose = Pose::Zero;
	int	angle;
	short angAdd;
	int speed;
	int acc;
	int swim;
};

