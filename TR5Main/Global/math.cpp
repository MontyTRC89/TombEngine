#include "math.h"

// fix "improperly terminated macro invocation"
// fix "expression must have integral or unscoped enum type"
short ANGLE(short angle)
{
	return short((angle) * 65536.0f / 360.0f);
}

float ANGLEF(short angle)
{
	return TR_ANGLE_TO_DEGREES(angle);
}
