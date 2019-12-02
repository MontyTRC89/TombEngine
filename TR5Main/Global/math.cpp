#include "math.h"

// fix "improperly terminated macro invocation"
// fix "expression must have integral or unscoped enum type"
short ANGLE(short ang)
{
	return short((ang * 65536.0) / 360.0);
}