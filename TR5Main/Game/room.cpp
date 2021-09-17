#include "framework.h"
#include "room.h"

FLOOR_INFO* GetSector(ROOM_INFO* r, int x, int z) 
{
	int sectorX = (x) / SECTOR(1);
	int sectorZ = (z) / SECTOR(1);
	int index = sectorZ + sectorX * r->xSize;
	if (index > r->floor.size()) 
	{
		return nullptr;
	}
	return &r->floor[index];
}
