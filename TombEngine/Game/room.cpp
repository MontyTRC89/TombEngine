#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Renderer;

bool FlipStatus = false;
bool FlipStats[MAX_FLIPMAP];
int  FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

bool ROOM_INFO::Active()
{
	if (flipNumber == NO_ROOM)
		return true;

	// Because engine swaps whole room memory block but substitutes flippedRoom,
	// we have to check both original index and flippedRoom equality, as well as NO_ROOM
	// in case we are checking non-flipped rooms.

	return (!FlipStats[flipNumber] && flippedRoom != index && flippedRoom != NO_ROOM) ||
		   ( FlipStats[flipNumber] && flippedRoom == index);
		   
}

void DoFlipMap(short group)
{
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto* room = &g_Level.Rooms[i];

		if (room->flippedRoom >= 0 && room->flipNumber == group)
		{
			RemoveRoomFlipItems(room);

			auto* flipped = &g_Level.Rooms[room->flippedRoom];

			auto temp = *room;
			*room = *flipped;
			*flipped = temp;

			room->flippedRoom = flipped->flippedRoom;
			flipped->flippedRoom = NO_ROOM;

			room->itemNumber = flipped->itemNumber;
			room->fxNumber = flipped->fxNumber;

			AddRoomFlipItems(room);

			g_Renderer.FlipRooms(i, room->flippedRoom);

			for (auto& fd : room->floor)
				fd.Room = i;

			for (auto& fd : flipped->floor)
				fd.Room = room->flippedRoom;
		}
	}

	FlipStatus = FlipStats[group] = !FlipStats[group];

	for (auto& currentCreature : ActiveCreatures)
		currentCreature->LOT.TargetBox = NO_BOX;
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

	int xTable = x / BLOCK(1);
	int zTable = z / BLOCK(1);

	if (OutsideRoomTable[xTable][zTable].empty())
		return NO_ROOM;

	for (int i = 0; i < OutsideRoomTable[xTable][zTable].size(); i++)
	{
		short roomNumber = OutsideRoomTable[xTable][zTable][i];
		auto* room = &g_Level.Rooms[roomNumber];

		if ((y > room->maxceiling && y < room->minfloor) &&
			(z > (room->z + BLOCK(1)) && z < (room->z + (room->zSize - 1) * BLOCK(1))) &&
			(x > (room->x + BLOCK(1)) && x < (room->x + (room->xSize - 1) * BLOCK(1))))
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
	int sectorX = std::clamp(x / BLOCK(1), 0, room->xSize - 1);
	int sectorZ = std::clamp(z / BLOCK(1), 0, room->zSize - 1);

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

	if (pos.z >= (room->z + BLOCK(1)) && pos.z <= (room->z + ((room->zSize - 1) * BLOCK(1))) &&
		pos.x >= (room->x + BLOCK(1)) && pos.x <= (room->x + ((room->xSize - 1) * BLOCK(1))) &&
		pos.y <= room->minfloor && pos.y > room->maxceiling) // Up is -Y, hence Y should be "less" than floor.
	{
		return true;
	}

	return false;
}

int FindRoomNumber(Vector3i position, int startRoom)
{
	if (startRoom != NO_ROOM && startRoom < g_Level.Rooms.size())
	{
		auto& room = g_Level.Rooms[startRoom];
		for (auto n : room.neighbors)
		{
			if (n != startRoom && IsPointInRoom(position, n) && g_Level.Rooms[n].Active())
				return n;
		}
	}

	for (int i = 0; i < g_Level.Rooms.size(); i++)
		if (IsPointInRoom(position, i) && g_Level.Rooms[i].Active())
			return i;

	return (startRoom != NO_ROOM) ? startRoom : 0;
}

Vector3i GetRoomCenter(int roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];

	auto halfLength = BLOCK(room->xSize) / 2;
	auto halfDepth = BLOCK(room->zSize) / 2;
	auto halfHeight = (room->maxceiling - room->minfloor) / 2;

	auto center = Vector3i(
		room->x + halfLength,
		room->minfloor + halfHeight,
		room->z + halfDepth);
	return center;
}

std::set<int> GetRoomList(int roomNumber)
{
	auto roomNumberList = std::set<int>{};

	if (g_Level.Rooms.size() <= roomNumber)
		return roomNumberList;

	roomNumberList.insert(roomNumber);

	auto* roomPtr = &g_Level.Rooms[roomNumber];
	for (size_t i = 0; i < roomPtr->doors.size(); i++)
		roomNumberList.insert(roomPtr->doors[i].room);

	for (int roomNumber : roomNumberList)
	{
		roomPtr = &g_Level.Rooms[roomNumber];
		for (size_t j = 0; j < roomPtr->doors.size(); j++)
			roomNumberList.insert(roomPtr->doors[j].room);
	}

	return roomNumberList;
}

void InitializeNeighborRoomList()
{
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto& room = g_Level.Rooms[i];

		room.neighbors.clear();

		auto roomNumberList = GetRoomList(i);
		for (int roomNumber : roomNumberList)
			room.neighbors.push_back(roomNumber);
	}

	// Add flipped variations of itself.
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto& room = g_Level.Rooms[i];
		if (room.flippedRoom == NO_ROOM)
			continue;

		auto it = std::find(room.neighbors.begin(), room.neighbors.end(), room.flippedRoom);
		if (it == room.neighbors.end())
			room.neighbors.push_back(room.flippedRoom);

		auto& flippedRoom = g_Level.Rooms[room.flippedRoom];
		auto it2 = std::find(flippedRoom.neighbors.begin(), flippedRoom.neighbors.end(), i);

		if (it2 == flippedRoom.neighbors.end())
			flippedRoom.neighbors.push_back(i);
	}
}
