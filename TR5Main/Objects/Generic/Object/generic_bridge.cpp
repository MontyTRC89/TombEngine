#include "framework.h"
#include "generic_bridge.h"
#include "floordata.h"
#include "level.h"

void InitialiseBridge(short itemNumber)
{
	FLOOR_INFO::AddFloor(itemNumber);
	FLOOR_INFO::AddCeiling(itemNumber);
}

int BridgeFloor(short itemNumber, int x, int y, int z)
{
	auto item = &g_Level.Items[itemNumber];
	return item->pos.yPos + GetOffset(&item->pos, x, z);
}

int BridgeCeiling(short itemNumber, int x, int y, int z)
{
	auto item = &g_Level.Items[itemNumber];
	return item->pos.yPos + GetOffset(&item->pos, x, z);
}

int GetOffset(PHD_3DPOS* pos, int x, int z)
{
	auto vector = FLOOR_INFO::GetSectorPoint(x, z);
	auto matrix = Matrix::CreateRotationZ(TO_RAD(pos->yRot));
	auto result = Vector2::Transform(Vector2(vector.x, vector.y), matrix);
	return tan(TO_RAD(pos->zRot)) * result.x - tan(TO_RAD(pos->xRot)) * result.y;
}
