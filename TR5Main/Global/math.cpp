#include "math.h"
#include "vars.h"

// fix "improperly terminated macro invocation"
// fix "expression must have integral or unscoped enum type"
short ANGLE(float angle)
{
	return angle * 65536.0f / 360.0f;
}

float TR_ANGLE_TO_DEGREES(short angle)
{
	return (unsigned short) angle * 360.0f / 65536.0f;
}

float TR_ANGLE_TO_RAD(short angle)
{
	return angle * 360.0f / 65536.0f * RADIAN;
}

const float frand() {
	int randValue = rand();
	float result = randValue / (float)RAND_MAX;
	return result;
}

const float frandMinMax(float min, float max)
{
	return frand()* (max - min) + min;
}

const float lerp(float v0, float v1, float t) {
	return (1 - t) * v0 + t * v1;
}
