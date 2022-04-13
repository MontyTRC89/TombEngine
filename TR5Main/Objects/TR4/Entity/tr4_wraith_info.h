#pragma once
#include "Specific/EulerAngle.h"

struct WraithInfo
{
	PHD_3DPOS Pose;
	EulerAngle Orientation;

	unsigned char r;
	unsigned char g;
	unsigned char b;
};
