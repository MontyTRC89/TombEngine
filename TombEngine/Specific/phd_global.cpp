#include "framework.h"
#include "phd_global.h"

BOUNDING_BOX operator+(const BOUNDING_BOX& box, const PHD_3DPOS& vec)
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

BOUNDING_BOX operator*(const BOUNDING_BOX& box, const float scale)
{
	BOUNDING_BOX box2 = box;
	box2.X1 *= scale;
	box2.X2 *= scale;
	box2.Y1 *= scale;
	box2.Y2 *= scale;
	box2.Z1 *= scale;
	box2.Z2 *= scale;
	return box2;
}
