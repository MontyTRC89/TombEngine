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
