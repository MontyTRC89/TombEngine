#pragma once
#include "Specific/EulerAngle.h"

struct WraithInfo
{
	PHD_3DPOS Pose;
	EulerAngle Orientation; // temp

	unsigned char r;
	unsigned char g;
	unsigned char b;
};
