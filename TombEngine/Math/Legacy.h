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

constexpr short ANGLE(float degrees)
{
	return (degrees * (65536.0f / 360.0f));
}

constexpr short FROM_RAD(float radians)
{
	return ((radians / RADIAN) * (65536.0f / 360.0f));
}

constexpr float TO_DEGREES(short shortAngle)
{
	return (shortAngle * (360.0f / 65536.0f));
}

constexpr float TO_RAD(short shortAngle)
{
	return ((shortAngle * (360.0f / 65536.0f)) * RADIAN);
}

float phd_sin(short x);
float phd_cos(short x);
int	  phd_atan(int y, int x);

void GetMatrixFromTrAngle(Matrix& matrix, short* framePtr, int index);
