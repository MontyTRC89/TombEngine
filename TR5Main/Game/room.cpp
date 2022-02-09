#include "framework.h"
#include "Game/room.h"

#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

byte FlipStatus = 0;
int FlipStats[MAX_FLIPMAP];
int FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

void DoFlipMap(short group)
{
	ROOM_INFO temp;

	for (size_t i = 0; i < g_Level.Rooms.size(); i++)
	{
		ROOM_INFO* r = &g_Level.Rooms[i];

		if (r->flippedRoom >= 0 && r->flipNumber == group)
		{
			RemoveRoomFlipItems(r);

			ROOM_INFO* flipped = &g_Level.Rooms[r->flippedRoom];

			temp = *r;
			*r = *flipped;
			*flipped = temp;

			r->flippedRoom = flipped->flippedRoom;
			flipped->flippedRoom = -1;

			r->itemNumber = flipped->itemNumber;
			r->fxNumber = flipped->fxNumber;

			AddRoomFlipItems(r);

			g_Renderer.flipRooms(static_cast<short>(i), r->flippedRoom);

			for (auto& fd : r->floor)
				fd.Room = i;
			for (auto& fd : flipped->floor)
				fd.Room = r->flippedRoom;
		}
	}

	FlipStatus = FlipStats[group] = !FlipStats[group];

	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		ActiveCreatures[i]->LOT.targetBox = NO_BOX;
	}
}

void AddRoomFlipItems(ROOM_INFO* r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].NextItem)
	{
		ITEM_INFO* item = &g_Level.Items[linkNum];

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(linkNum);
	}
}

void RemoveRoomFlipItems(ROOM_INFO* r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].NextItem)
	{
		ITEM_INFO* item = &g_Level.Items[linkNum];

		if (item->Flags & ONESHOT 
			&& Objects[item->ObjectNumber].intelligent 
			&& item->HitPoints <= 0 
			&& item->HitPoints != NOT_TARGETABLE)
		{
			KillItem(linkNum);
		}

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(linkNum, true);
	}
}

int IsObjectInRoom(short roomNumber, short objectNumber)
{
	short itemNumber = g_Level.Rooms[roomNumber].itemNumber;

	if (itemNumber == NO_ITEM)
		return 0;

	while (true)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->ObjectNumber == objectNumber)
			break;

		itemNumber = item->NextItem;

		if (itemNumber == NO_ITEM)
			return 0;
	}

	return 1;
}

int IsRoomOutside(int x, int y, int z)
{
	if (x < 0 || z < 0)
		return NO_ROOM;

	int xTable = x / 1024;
	int zTable = z / 1024;

	if (OutsideRoomTable[xTable][zTable].size() == 0)
		return NO_ROOM;

	for (size_t i = 0; i < OutsideRoomTable[xTable][zTable].size(); i++)
	{
		short roomNumber = OutsideRoomTable[xTable][zTable][i];
		auto r = &g_Level.Rooms[roomNumber];

		if ((y > r->maxceiling) && (y < r->minfloor)
			&& ((z > (r->z + 1024)) && (z < (r->z + ((r->zSize - 1) * 1024))))
			&& ((x > (r->x + 1024)) && (x < (r->x + ((r->xSize - 1) * 1024)))))
		{
			auto floor = GetFloor(x, y, z, &roomNumber);
			int height = GetFloorHeight(floor, x, y, z);
			if (height == NO_HEIGHT || y > height)
				return NO_ROOM;
			height = GetCeiling(floor, x, y, z);
			if (y < height)
				return NO_ROOM;

			return ((r->flags & (ENV_FLAG_WIND | ENV_FLAG_WATER)) != 0 ? roomNumber : NO_ROOM);
		}
	}

	return NO_ROOM;
}

FLOOR_INFO* GetSector(ROOM_INFO* r, int x, int z) 
{
	int sectorX = std::clamp(x / SECTOR(1), 0, r->xSize - 1);
	int sectorZ = std::clamp(z / SECTOR(1), 0, r->zSize - 1);

	int index = sectorZ + sectorX * r->zSize;
	if (index > r->floor.size()) 
	{
		return nullptr;
	}
	return &r->floor[index];
}

bool IsPointInRoom(PHD_3DPOS const & pos, int roomNumber)
{
	int x = pos.xPos;
	int y = pos.yPos;
	int z = pos.zPos;
	ROOM_INFO* r = &g_Level.Rooms[roomNumber];
	int xSector = (x - r->x) / SECTOR(1);
	int zSector = (z - r->z) / SECTOR(1);

	return (xSector >= 0 && xSector <= r->xSize - 1) &&
		(zSector >= 0 && zSector <= r->zSize - 1) &&
		(y <= r->minfloor && y >= r->maxceiling); // up is negative y axis, hence y should be "less" than floor
}

PHD_3DPOS GetRoomCenter(int roomNumber)
{
	ROOM_INFO* r = &g_Level.Rooms[roomNumber];
	auto halfLength = SECTOR(r->xSize)/2;
	auto halfDepth = SECTOR(r->zSize)/2;
	auto halfHeight = (r->maxceiling - r->minfloor) / 2;
	PHD_3DPOS center;
	center.xPos = r->x + halfLength;
	center.yPos = r->minfloor + halfHeight;
	center.zPos = r->z + halfDepth;
	return center;
}

std::set<int> GetRoomList(int roomNumber)
{
	std::set<int> result;

	if (g_Level.Rooms.size() <= roomNumber)
		return result;

	result.insert(roomNumber);

	auto room = &g_Level.Rooms[roomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		result.insert(room->doors[i].room);

	for (auto i : result)
	{
		room = &g_Level.Rooms[i];
		for (int j = 0; j < room->doors.size(); j++)
			result.insert(room->doors[j].room);
	}

	return result;
}