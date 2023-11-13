#include "framework.h"
#include "Objects/Generic/Object/generic_bridge.h"

#include "Game/collision/floordata.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

void InitializeBridge(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	UpdateBridgeItem(item);
}

int GetOffset(short angle, int x, int z)
{
	// Get rotated sector point.
	auto sectorPoint = GetSectorPoint(x, z).ToVector2();
	auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(angle));
	Vector2::Transform(sectorPoint, rotMatrix, sectorPoint);

	// Return offset.
	return -sectorPoint.x;
}
