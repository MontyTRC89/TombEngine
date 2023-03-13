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
	constexpr auto SHORTS_TO_ONE_DEGREE = 65536.0f / 360.0f;

	return short(degrees * SHORTS_TO_ONE_DEGREE);
}

constexpr short FROM_RAD(float radians)
{
	constexpr auto SHORTS_TO_ONE_DEGREE = 65536.0f / 360.0f;

	return short((radians / RADIAN) * SHORTS_TO_ONE_DEGREE);
}

constexpr float TO_DEGREES(short shortAngle)
{
	constexpr auto DEGREES_TO_ONE_SHORT = 360.0f / 65536.0f;

	return (shortAngle * DEGREES_TO_ONE_SHORT);
}

constexpr float TO_RAD(short shortAngle)
{
	constexpr auto DEGREES_TO_ONE_SHORT = 360.0f / 65536.0f;

	return ((shortAngle * DEGREES_TO_ONE_SHORT) * RADIAN);
}

float phd_sin(short x);
float phd_cos(short x);
int	  phd_atan(int y, int x);
