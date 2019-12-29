#include "math.h"
#include "vars.h"

// fix "improperly terminated macro invocation"
// fix "expression must have integral or unscoped enum type"
short ANGLE(double angle)
{
	return angle * 65536.0 / 360.0;
}

float ANGLEF(short angle)
{
	return TR_ANGLE_TO_DEGREES(angle);
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
