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
	return item->pos.yPos;
}

int BridgeCeiling(short itemNumber, int x, int y, int z)
{
	auto item = &g_Level.Items[itemNumber];
	return item->pos.yPos;
}

int GetOffset(PHD_3DPOS* pos)
{
	auto vector = FLOOR_INFO::GetSectorPoint(pos->xPos, pos->zPos);
	auto matrix = Matrix::CreateRotationZ(TO_RAD(pos->yRot));
	auto result = Vector2::Transform(Vector2(vector.x, vector.y), matrix);
	return -(pos->zRot * result.x + pos->xRot * result.y) / 8192;
}
