#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Renderer/Renderer.h"
#include "Math/Math.h"
#include "Objects/game_object_ids.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Collision::Floordata;
using namespace TEN::Renderer;
using namespace TEN::Utils;

bool FlipStatus = false;
bool FlipStats[MAX_FLIPMAP];
int  FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

bool ROOM_INFO::Active() const
{
	if (flipNumber == NO_ROOM)
		return true;

	// Since engine swaps whole room memory block but substitutes flippedRoom,
	// must check both original room number and flippedRoom equality,
	// as well as NO_ROOM if checking non-flipped rooms.
	return (!FlipStats[flipNumber] && flippedRoom != index && flippedRoom != NO_ROOM) ||
		   ( FlipStats[flipNumber] && flippedRoom == index);
}

void DoFlipMap(int group)
{
	// Run through rooms.
	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = g_Level.Rooms[roomNumber];

		// Handle flipmap.
		if (room.flippedRoom >= 0 && room.flipNumber == group)
		{
			RemoveRoomFlipItems(&room);

			auto& flippedRoom = g_Level.Rooms[room.flippedRoom];

			std::swap(room, flippedRoom);

			room.flippedRoom = flippedRoom.flippedRoom;
			flippedRoom.flippedRoom = NO_ROOM;

			room.itemNumber = flippedRoom.itemNumber;
			room.fxNumber = flippedRoom.fxNumber;

			AddRoomFlipItems(&room);

			g_Renderer.FlipRooms(roomNumber, room.flippedRoom);

			for (auto& sector : room.floor)
				sector.Room = roomNumber;

			for (auto& sector : flippedRoom.floor)
				sector.Room = room.flippedRoom;
		}
	}

	FlipStatus =
	FlipStats[group] = !FlipStats[group];

	for (auto& creature : ActiveCreatures)
		creature->LOT.TargetBox = NO_BOX;
}

void AddRoomFlipItems(ROOM_INFO* room)
{
	// Run through linked items.
	for (int itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].NextItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		// Item is bridge; update relevant sectors.
		if (object.GetFloorHeight != nullptr)
			UpdateBridgeItem(item);
	}
}

void RemoveRoomFlipItems(ROOM_INFO* room)
{
	// Run through linked items.
	for (int itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].NextItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		if (item.Flags & ONESHOT &&
			item.HitPoints != NOT_TARGETABLE &&
			item.HitPoints <= 0 &&
			object.intelligent)
		{
			KillItem(itemNumber);
		}

		// Item is bridge; update relevant sectors.
		if (Objects[item.ObjectNumber].GetFloorHeight != nullptr)
			UpdateBridgeItem(item, true);
	}
}

bool IsObjectInRoom(int roomNumber, GAME_OBJECT_ID objectID)
{
	int itemNumber = g_Level.Rooms[roomNumber].itemNumber;
	if (itemNumber == NO_ITEM)
		return false;

	while (true)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (item.ObjectNumber == objectID)
			break;

		itemNumber = item.NextItem;
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
		int roomNumber = OutsideRoomTable[xTable][zTable][i];
		const auto& room = g_Level.Rooms[roomNumber];

		if ((x > (room.x + BLOCK(1)) && x < (room.x + (room.xSize - 1) * BLOCK(1))) &&
			(y > room.maxceiling && y < room.minfloor) &&
			(z > (room.z + BLOCK(1)) && z < (room.z + (room.zSize - 1) * BLOCK(1))))
		{
			auto pointColl = GetCollision(x, y, z, roomNumber);

			if (pointColl.Position.Floor == NO_HEIGHT || y > pointColl.Position.Floor)
				return NO_ROOM;

			if (y < pointColl.Position.Ceiling)
				return NO_ROOM;

			if (TestEnvironmentFlags(ENV_FLAG_WATER, room.flags) ||
				TestEnvironmentFlags(ENV_FLAG_WIND, room.flags))
			{
				return pointColl.RoomNumber;
			}

			return NO_ROOM;
		}
	}

	return NO_ROOM;
}

// TODO: Can use floordata's GetRoomGridCoord()?
FloorInfo* GetSector(ROOM_INFO* room, int x, int z) 
{
	int sectorX = std::clamp(x / BLOCK(1), 0, room->xSize - 1);
	int sectorZ = std::clamp(z / BLOCK(1), 0, room->zSize - 1);

	int sectorID = sectorZ + (sectorX * room->zSize);
	if (sectorID > room->floor.size()) 
		return nullptr;
	
	return &room->floor[sectorID];
}

GameBoundingBox& GetBoundsAccurate(const MESH_INFO& mesh, bool getVisibilityBox)
{
	static auto bounds = GameBoundingBox();

	if (getVisibilityBox)
	{
		bounds = StaticObjects[mesh.staticNumber].visibilityBox * mesh.scale;
	}
	else
	{
		bounds = StaticObjects[mesh.staticNumber].collisionBox * mesh.scale;
	}

	return bounds;
}

bool IsPointInRoom(const Vector3i& pos, int roomNumber)
{
	const auto& room = g_Level.Rooms[roomNumber];

	if (pos.z >= (room.z + BLOCK(1)) && pos.z <= (room.z + BLOCK(room.zSize - 1)) &&
		pos.y <= room.minfloor && pos.y > room.maxceiling &&
		pos.x >= (room.x + BLOCK(1)) && pos.x <= (room.x + BLOCK(room.xSize - 1)))
	{
		return true;
	}

	return false;
}

int FindRoomNumber(const Vector3i& pos, int startRoomNumber)
{
	if (startRoomNumber != NO_ROOM && startRoomNumber < g_Level.Rooms.size())
	{
		const auto& room = g_Level.Rooms[startRoomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			const auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
			if (neighborRoomNumber != startRoomNumber && neighborRoom.Active() &&
				IsPointInRoom(pos, neighborRoomNumber))
			{
				return neighborRoomNumber;
			}
		}
	}

	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		if (IsPointInRoom(pos, roomNumber) && g_Level.Rooms[roomNumber].Active())
			return roomNumber;
	}

	return (startRoomNumber != NO_ROOM) ? startRoomNumber : 0;
}

Vector3i GetRoomCenter(int roomNumber)
{
	const auto& room = g_Level.Rooms[roomNumber];

	int halfLength = BLOCK(room.xSize) / 2;
	int halfDepth = BLOCK(room.zSize) / 2;
	int halfHeight = (room.maxceiling - room.minfloor) / 2;

	// Calculate and return center.
	return Vector3i(
		room.x + halfLength,
		room.minfloor + halfHeight,
		room.z + halfDepth);
}

static std::vector<int> GetNeighborRoomNumbers(int roomNumber, unsigned int searchDepth, std::vector<int>& visitedRoomNumbers = std::vector<int>{})
{
	// Invalid room; return empty vector.
	if (g_Level.Rooms.size() <= roomNumber)
		return {};

	// Search depth limit reached; return empty vector.
	if (searchDepth == 0)
		return {};

	// Collect current room number as neighbor of itself.
	visitedRoomNumbers.push_back(roomNumber);

	auto neighborRoomNumbers = std::vector<int>{};

	// Recursively collect neighbors of current neighbor.
	const auto& room = g_Level.Rooms[roomNumber];
	if (room.doors.empty())
	{
		neighborRoomNumbers.push_back(roomNumber);
	}
	else
	{
		for (int doorID = 0; doorID < room.doors.size(); doorID++)
		{
			int neighborRoomNumber = room.doors[doorID].room;
			neighborRoomNumbers.push_back(neighborRoomNumber);

			auto recNeighborRoomNumbers = GetNeighborRoomNumbers(neighborRoomNumber, searchDepth - 1, visitedRoomNumbers);
			neighborRoomNumbers.insert(neighborRoomNumbers.end(), recNeighborRoomNumbers.begin(), recNeighborRoomNumbers.end());
		}
	}

	// Sort and clean collection.
	std::sort(neighborRoomNumbers.begin(), neighborRoomNumbers.end());
	neighborRoomNumbers.erase(std::unique(neighborRoomNumbers.begin(), neighborRoomNumbers.end()), neighborRoomNumbers.end());

	return neighborRoomNumbers;
}

void InitializeNeighborRoomList()
{
	constexpr auto NEIGHBOR_ROOM_SEARCH_DEPTH = 2;

	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = g_Level.Rooms[roomNumber];
		room.neighbors = GetNeighborRoomNumbers(roomNumber, NEIGHBOR_ROOM_SEARCH_DEPTH);
	}

	// Add flipped variations of itself.
	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = g_Level.Rooms[roomNumber];
		if (room.flippedRoom == NO_ROOM)
			continue;

		if (!Contains(room.neighbors, room.flippedRoom))
			room.neighbors.push_back(room.flippedRoom);

		auto& flippedRoom = g_Level.Rooms[room.flippedRoom];
		if (!Contains(flippedRoom.neighbors, roomNumber))
			flippedRoom.neighbors.push_back(roomNumber);
	}
}
