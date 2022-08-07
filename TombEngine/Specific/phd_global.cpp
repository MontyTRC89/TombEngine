#include "framework.h"
#include "Specific/phd_global.h"

BOUNDING_BOX operator+(BOUNDING_BOX const & box, PHD_3DPOS const & vec)
{
	BOUNDING_BOX box2 = box;
	box2.X1 += vec.Position.x;
	box2.X2 += vec.Position.x;
	box2.Y1 += vec.Position.y;
	box2.Y2 += vec.Position.y;
	box2.Z1 += vec.Position.z;
	box2.Z2 += vec.Position.z;
	return box2;
}