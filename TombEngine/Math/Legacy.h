#pragma once
#include "Math/Constants.h"

constexpr auto FP_SHIFT				   = 16;
constexpr auto W2V_SHIFT			   = 14;
constexpr auto PREDICTIVE_SCALE_FACTOR = 14;

struct ColorData
{
	byte r, g, b;
	byte cd;
};

constexpr short ANGLE(float angle)
{
	return (angle * (65536.0f / 360.0f));
}

constexpr short FROM_DEGREES(float angle)
{
	return (angle * (65536.0f / 360.0f));
}

constexpr short FROM_RAD(float angle)
{
	return ((angle / RADIAN) * (65536.0f / 360.0f));
}

constexpr float TO_DEGREES(short angle)
{
	return (angle * (360.0f / 65536.0f));
}

constexpr float TO_RAD(short angle)
{
	return ((angle * (360.0f / 65536.0f)) * RADIAN);
}

float phd_sin(short a);
float phd_cos(short a);
int	  phd_atan(int dz, int dx);

void InterpolateAngle(short angle, short& rotation, short& outAngle, int shift);
void GetMatrixFromTrAngle(Matrix& matrix, short* framePtr, int index);
