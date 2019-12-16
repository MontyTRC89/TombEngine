#include "misc.h"

short GF(short animIndex, short frameToStart)
{
	return short(Anims[animIndex].frameBase + frameToStart);
}

short GF2(short objectID, short animIndex, short frameToStart)
{
	return short(Anims[Objects[objectID].animIndex + animIndex].frameBase + frameToStart);
}

int INVERT(int value1, int value2, bool isInvert)
{
	if (isInvert)
		return value2 - value1;
	else
		return value1 - value2;
}
