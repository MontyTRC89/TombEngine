#include "framework.h"
#include "phd_global.h"

const Vector2Int Vector2Int::Zero = Vector2Int(0, 0);
const Vector3Int Vector3Int::Zero = Vector3Int(0, 0, 0);
const Vector3Shrt Vector3Shrt::Zero = Vector3Shrt(0, 0, 0);

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