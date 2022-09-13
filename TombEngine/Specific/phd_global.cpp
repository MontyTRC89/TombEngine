#include "framework.h"
#include "phd_global.h"

const Vector2Int Vector2Int::Zero = Vector2Int(0, 0);
const Vector3Int Vector3Int::Zero = Vector3Int(0, 0, 0);
const Vector3Shrt Vector3Shrt::Zero = Vector3Shrt(0, 0, 0);

BOUNDING_BOX operator+(const BOUNDING_BOX& box, const PHD_3DPOS& pose)
{
	auto box2 = box;
	box2.X1 += pose.Position.x;
	box2.X2 += pose.Position.x;
	box2.Y1 += pose.Position.y;
	box2.Y2 += pose.Position.y;
	box2.Z1 += pose.Position.z;
	box2.Z2 += pose.Position.z;
	return box2;
}

BOUNDING_BOX operator*(const BOUNDING_BOX& box, const float scale)
{
	auto box2 = box;
	box2.X1 *= scale;
	box2.X2 *= scale;
	box2.Y1 *= scale;
	box2.Y2 *= scale;
	box2.Z1 *= scale;
	box2.Z2 *= scale;
	return box2;
}