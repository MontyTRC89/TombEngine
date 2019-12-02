#include "anim.h"

short GF(short animIndex, short frameToStart)
{
	return short(Anims[animIndex].frameBase + frameToStart);
}

short GF2(short objectID, short animIndex, short frameToStart)
{
	return short(Anims[Objects[objectID].animIndex + animIndex].frameBase + frameToStart);
}
