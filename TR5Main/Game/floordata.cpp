#include "framework.h"
#include "trmath.h"
#include "floordata.h"
#include "room.h"

SECTOR_POSITION FLOOR_INFO::GetSectorPosition(ROOM_INFO* room, int x, int z)
{
	SECTOR_POSITION pos;

	pos.x = (z - room->z) / WALL_SIZE;
	pos.y = (x - room->x) / WALL_SIZE;

	if (pos.x < 0)
	{
		pos.x = 0;
	}
	else if (pos.x > room->xSize - 1)
	{
		pos.x = room->xSize - 1;
	}

	if (pos.y < 0)
	{
		pos.y = 0;
	}
	else if (pos.y > room->ySize - 1)
	{
		pos.y = room->ySize - 1;
	}

	return pos;
}

FLOOR_INFO& FLOOR_INFO::GetFloor(ROOM_INFO* room, int x, int z)
{
	return GetFloor(room, GetSectorPosition(room, x, z));
}

FLOOR_INFO& FLOOR_INFO::GetFloor(ROOM_INFO* room, SECTOR_POSITION pos)
{
	return room->floor[pos.x + pos.y * room->xSize];
}
