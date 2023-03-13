#include "framework.h"
#include "Game/collision/floordata.h"
#include "Objects/Generic/Object/generic_bridge.h"
#include "Specific/level.h"

using namespace TEN::Floordata;

void InitialiseBridge(short itemNumber)
{
	UpdateBridgeItem(itemNumber);
}

int GetOffset(short angle, int x, int z)
{
	const auto point = GetSectorPoint(x, z);
	auto vector = Vector2(point.x, point.y);
	const auto matrix = Matrix::CreateRotationZ(TO_RAD(angle));
	Vector2::Transform(vector, matrix, vector);
	return -vector.x;
}
