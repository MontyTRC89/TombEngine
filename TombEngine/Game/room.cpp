#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
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
		auto* room = &g_Level.Rooms[i];

		if (room->flippedRoom >= 0 && room->flipNumber == group)
		{
			RemoveRoomFlipItems(room);

			auto* flipped = &g_Level.Rooms[room->flippedRoom];

			temp = *room;
			*room = *flipped;
			*flipped = temp;

			room->flippedRoom = flipped->flippedRoom;
			flipped->flippedRoom = -1;

			room->Items = flipped->Items;
			room->Effects = flipped->Effects;

			AddRoomFlipItems(room);

			g_Renderer.FlipRooms(static_cast<short>(i), room->flippedRoom);

			for (auto& fd : room->floor)
				fd.Room = i;
			for (auto& fd : flipped->floor)
				fd.Room = room->flippedRoom;
		}
	}

	FlipStatus = FlipStats[group] = !FlipStats[group];

	for (int i = 0; i < ActiveCreatures.size(); i++)
		ActiveCreatures[i]->LOT.TargetBox = NO_BOX;
}

void AddRoomFlipItems(ROOM_INFO* room)
{
	for (short currentItemNumber : room->Items)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(currentItemNumber);
	}
}

void RemoveRoomFlipItems(ROOM_INFO* room)
{
	for (short currentItemNumber : room->Items)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (item->Flags & ONESHOT &&
			Objects[item->ObjectNumber].intelligent &&
			item->HitPoints <= 0 &&
			item->HitPoints != NOT_TARGETABLE)
		{
			KillItem(currentItemNumber);
		}

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(currentItemNumber, true);
	}
}

bool IsObjectInRoom(short roomNumber, short objectNumber)
{
	if (g_Level.Rooms[roomNumber].Items.size()==0)
		return false;

	for (short currentItemNumber : g_Level.Rooms[roomNumber].Items)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (item->ObjectNumber == objectNumber)
			break;
	}

	return true;
}

int IsRoomOutside(int x, int y, int z)
{
	if (x < 0 || z < 0)
		return NO_ROOM;

	int xTable = x / SECTOR(1);
	int zTable = z / SECTOR(1);

	if (OutsideRoomTable[xTable][zTable].size() == 0)
		return NO_ROOM;

	for (size_t i = 0; i < OutsideRoomTable[xTable][zTable].size(); i++)
	{
		short roomNumber = OutsideRoomTable[xTable][zTable][i];
		auto* room = &g_Level.Rooms[roomNumber];

		if (y > room->maxceiling &&
			y < room->minfloor &&
			(z > (room->z + SECTOR(1)) &&
				z < (room->z + (room->zSize - 1) * SECTOR(1))) &&
			(x > (room->x + SECTOR(1)) &&
				x < (room->x + (room->xSize - 1) * SECTOR(1))))
		{
			auto probe = GetCollision(x, y, z, roomNumber);

			if (probe.Position.Floor == NO_HEIGHT || y > probe.Position.Floor)
				return NO_ROOM;

			if (y < probe.Position.Ceiling)
				return NO_ROOM;

			return ((room->flags & (ENV_FLAG_WIND | ENV_FLAG_WATER)) != 0 ? probe.RoomNumber : NO_ROOM);
		}
	}

	return NO_ROOM;
}

FloorInfo* GetSector(ROOM_INFO* room, int x, int z) 
{
	int sectorX = std::clamp(x / SECTOR(1), 0, room->xSize - 1);
	int sectorZ = std::clamp(z / SECTOR(1), 0, room->zSize - 1);

	int index = sectorZ + sectorX * room->zSize;
	if (index > room->floor.size()) 
		return nullptr;
	
	return &room->floor[index];
}

bool IsPointInRoom(PHD_3DPOS const& pos, int roomNumber)
{
	int x = pos.Position.x;
	int y = pos.Position.y;
	int z = pos.Position.z;
	auto* room = &g_Level.Rooms[roomNumber];
	int xSector = (x - room->x) / SECTOR(1);
	int zSector = (z - room->z) / SECTOR(1);

	if ((xSector >= 0 && xSector <= room->xSize - 1) &&
		(zSector >= 0 && zSector <= room->zSize - 1) &&
		(y <= room->minfloor && y >= room->maxceiling))	 // up is negative y axis, hence y should be "less" than floor
	{
		return true;
	}

	return false;
}

PHD_3DPOS GetRoomCenter(int roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];
	auto halfLength = SECTOR(room->xSize)/2;
	auto halfDepth = SECTOR(room->zSize)/2;
	auto halfHeight = (room->maxceiling - room->minfloor) / 2;

	PHD_3DPOS center;
	center.Position.x = room->x + halfLength;
	center.Position.y = room->minfloor + halfHeight;
	center.Position.z = room->z + halfDepth;
	return center;
}

std::set<int> GetRoomList(int roomNumber)
{
	std::set<int> result;

	if (g_Level.Rooms.size() <= roomNumber)
		return result;

	result.insert(roomNumber);

	auto* room = &g_Level.Rooms[roomNumber];
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
