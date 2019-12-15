#include "math.h"

// fix "improperly terminated macro invocation"
// fix "expression must have integral or unscoped enum type"
short ANGLE(short angle)
{
	return short(TR_ANGLE_TO_DEGREES(angle));
}

float ANGLEF(short angle)
{
	return TR_ANGLE_TO_DEGREES(angle);
}
