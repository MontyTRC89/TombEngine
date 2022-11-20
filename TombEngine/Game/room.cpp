#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Floordata;
using namespace TEN::Renderer;

byte FlipStatus = 0;
int FlipStats[MAX_FLIPMAP];
int FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

bool ROOM_INFO::Active()
{
	if (flipNumber == -1)
		return true;

	return !(FlipStats[flipNumber] && flippedRoom == -1);
}

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

			room->itemNumber = flipped->itemNumber;
			room->fxNumber = flipped->fxNumber;

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
	for (short linkNumber = room->itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
	{
		auto* item = &g_Level.Items[linkNumber];

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(linkNumber);
	}
}

void RemoveRoomFlipItems(ROOM_INFO* room)
{
	for (short linkNumber = room->itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
	{
		auto* item = &g_Level.Items[linkNumber];

		if (item->Flags & ONESHOT &&
			Objects[item->ObjectNumber].intelligent &&
			item->HitPoints <= 0 &&
			item->HitPoints != NOT_TARGETABLE)
		{
			KillItem(linkNumber);
		}

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(linkNumber, true);
	}
}

bool IsObjectInRoom(short roomNumber, short objectNumber)
{
	short itemNumber = g_Level.Rooms[roomNumber].itemNumber;

	if (itemNumber == NO_ITEM)
		return false;

	while (true)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->ObjectNumber == objectNumber)
			break;

		itemNumber = item->NextItem;

		if (itemNumber == NO_ITEM)
			return false;
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

			if (TestEnvironmentFlags(ENV_FLAG_WATER, room->flags) ||
				TestEnvironmentFlags(ENV_FLAG_WIND, room->flags))
			{
				return probe.RoomNumber;
			}

			return NO_ROOM;
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

GameBoundingBox& GetBoundsAccurate(const MESH_INFO& mesh, bool visibility)
{
	static GameBoundingBox result;

	if (visibility)
		result = StaticObjects[mesh.staticNumber].visibilityBox * mesh.scale;
	else
		result = StaticObjects[mesh.staticNumber].collisionBox * mesh.scale;

	return result;
}

bool IsPointInRoom(Vector3i pos, int roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];

	int xSector = (pos.x - room->x) / SECTOR(1);
	int zSector = (pos.z - room->z) / SECTOR(1);

	if ((xSector >= 0 && xSector <= (room->xSize - 1)) &&
		(zSector >= 0 && zSector <= (room->zSize - 1)) &&
		(pos.y <= room->minfloor && pos.y >= room->maxceiling)) // Up is -Y, hence Y should be "less" than floor.
	{
		return true;
	}

	return false;
}

int FindRoomNumber(Vector3i position)
{
	for (int i = 0; i < g_Level.Rooms.size(); i++)
		if (IsPointInRoom(position, i))
			return i;

	return 0;
}

Vector3i GetRoomCenter(int roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];

	auto halfLength = SECTOR(room->xSize) / 2;
	auto halfDepth = SECTOR(room->zSize) / 2;
	auto halfHeight = (room->maxceiling - room->minfloor) / 2;

	auto center = Vector3i(
		room->x + halfLength,
		room->minfloor + halfHeight,
		room->z + halfDepth
	);
	return center;
}

std::set<int> GetRoomList(int roomNumber)
{
	std::set<int> roomNumberList;

	if (g_Level.Rooms.size() <= roomNumber)
		return roomNumberList;

	roomNumberList.insert(roomNumber);

	auto* room = &g_Level.Rooms[roomNumber];
	for (size_t i = 0; i < room->doors.size(); i++)
		roomNumberList.insert(room->doors[i].room);

	for (int roomNumber : roomNumberList)
	{
		room = &g_Level.Rooms[roomNumber];
		for (size_t j = 0; j < room->doors.size(); j++)
			roomNumberList.insert(room->doors[j].room);
	}

	return roomNumberList;
}

void InitializeNeighborRoomList()
{
	for (size_t i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto* room = &g_Level.Rooms[i];

		room->neighbors.clear();

		auto roomNumberList = GetRoomList(i);
		for (int roomNumber : roomNumberList)
			room->neighbors.push_back(roomNumber);
	}
}
