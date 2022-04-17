#include "framework.h"
#include "phd_global.h"

BOUNDING_BOX operator+(BOUNDING_BOX const & box, PHD_3DPOS const & vec)
{
	BOUNDING_BOX box2 = box;
	box2.X1 += vec.xPos;
	box2.X2 += vec.xPos;
	box2.Y1 += vec.yPos;
	box2.Y2 += vec.yPos;
	box2.Z1 += vec.zPos;
	box2.Z2 += vec.zPos;
	return box2;
}