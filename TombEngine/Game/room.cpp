#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Math/Math.h"
#include "Objects/game_object_ids.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Renderer/Renderer.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Entities::Doors;
using namespace TEN::Renderer;
using namespace TEN::Utils;

bool FlipStatus = false;
bool FlipStats[MAX_FLIPMAP];
int  FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

bool ROOM_INFO::Active() const
{
	if (flipNumber == NO_VALUE)
		return true;

	// Since engine swaps whole room memory block but substitutes flippedRoom,
	// must check both original room number and flippedRoom equality,
	// as well as NO_VALUE if checking non-flipped rooms.
	return (!FlipStats[flipNumber] && flippedRoom != RoomNumber && flippedRoom != NO_VALUE) ||
		   ( FlipStats[flipNumber] && flippedRoom == RoomNumber);
}

static void AddRoomFlipItems(const ROOM_INFO& room)
{
	// Run through linked items.
	for (int itemNumber = room.itemNumber; itemNumber != NO_VALUE; itemNumber = g_Level.Items[itemNumber].NextItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		// Add bridges.
		if (item.IsBridge())
			UpdateBridgeItem(item);
	}
}

static void RemoveRoomFlipItems(const ROOM_INFO& room)
{
	// Run through linked items.
	for (int itemNumber = room.itemNumber; itemNumber != NO_VALUE; itemNumber = g_Level.Items[itemNumber].NextItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		// Kill item.
		if (item.Flags & ONESHOT &&
			item.HitPoints != NOT_TARGETABLE &&
			item.HitPoints <= 0 &&
			object.intelligent)
		{
			KillItem(itemNumber);
		}

		// Clear bridge.
		if (item.IsBridge())
			UpdateBridgeItem(item, BridgeUpdateType::Remove);
	}
}

static void FlipRooms(int roomNumber, ROOM_INFO& activeRoom, ROOM_INFO& flippedRoom)
{
	RemoveRoomFlipItems(activeRoom);

	// Swap rooms.
	std::swap(activeRoom, flippedRoom);
	activeRoom.flippedRoom = flippedRoom.flippedRoom;
	flippedRoom.flippedRoom = NO_VALUE;
	activeRoom.itemNumber = flippedRoom.itemNumber;
	activeRoom.fxNumber = flippedRoom.fxNumber;

	AddRoomFlipItems(activeRoom);

	// Update active room sectors.
	for (auto& sector : activeRoom.Sectors)
		sector.RoomNumber = roomNumber;

	// Update flipped room sectors.
	for (auto& sector : flippedRoom.Sectors)
		sector.RoomNumber = activeRoom.flippedRoom;

	// Update renderer data.
	g_Renderer.FlipRooms(roomNumber, activeRoom.flippedRoom);
}

void ResetRoomData()
{
	// Remove all door collisions.
	for (const auto& item : g_Level.Items)
	{
		if (item.ObjectNumber == NO_VALUE || !item.Data.is<DOOR_DATA>())
			continue;

		auto& doorItem = g_Level.Items[item.Index];
		auto& door = *(DOOR_DATA*)doorItem.Data;

		if (door.opened)
			continue;

		OpenThatDoor(&door.d1, &door);
		OpenThatDoor(&door.d2, &door);
		OpenThatDoor(&door.d1flip, &door);
		OpenThatDoor(&door.d2flip, &door);
		door.opened = true;
	}

	// Unflip all rooms and remove all bridges and stopper flags.
	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = g_Level.Rooms[roomNumber];
		if (room.flippedRoom != NO_VALUE && room.flipNumber != NO_VALUE && FlipStats[room.flipNumber])
		{
			auto& flippedRoom = g_Level.Rooms[room.flippedRoom];
			FlipRooms(roomNumber, room, flippedRoom);
		}

		for (auto& sector : room.Sectors)
		{
			sector.Stopper = false;
			sector.BridgeItemNumbers.clear();
		}
	}

	// Make sure no pathfinding boxes are blocked (either by doors or by other door-like objects).
	for (int pathfindingBoxID = 0; pathfindingBoxID < g_Level.PathfindingBoxes.size(); pathfindingBoxID++)
		g_Level.PathfindingBoxes[pathfindingBoxID].flags &= ~BLOCKED;
}

void DoFlipMap(int group)
{
	if (group >= MAX_FLIPMAP)
	{
		TENLog("Maximum flipmap group number is " + std::to_string(MAX_FLIPMAP) + ".", LogLevel::Warning);
		return;
	}

	// Run through rooms.
	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = g_Level.Rooms[roomNumber];

		// Handle flipmap.
		if (room.flippedRoom != NO_VALUE && room.flipNumber == group)
		{
			auto& flippedRoom = g_Level.Rooms[room.flippedRoom];
			FlipRooms(roomNumber, room, flippedRoom);
		}
	}

	FlipStatus =
	FlipStats[group] = !FlipStats[group];

	for (auto& creature : ActiveCreatures)
		creature->LOT.TargetBox = NO_VALUE;
}

bool IsObjectInRoom(int roomNumber, GAME_OBJECT_ID objectID)
{
	int itemNumber = g_Level.Rooms[roomNumber].itemNumber;
	if (itemNumber == NO_VALUE)
		return false;

	while (true)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (item.ObjectNumber == objectID)
			break;

		itemNumber = item.NextItem;
		if (itemNumber == NO_VALUE)
			return false;
	}

	return true;
}

int IsRoomOutside(int x, int y, int z)
{
	if (x < 0 || z < 0)
		return NO_VALUE;

	int xTable = x / BLOCK(1);
	int zTable = z / BLOCK(1);

	if (OutsideRoomTable[xTable][zTable].empty())
		return NO_VALUE;

	for (int i = 0; i < OutsideRoomTable[xTable][zTable].size(); i++)
	{
		int roomNumber = OutsideRoomTable[xTable][zTable][i];
		const auto& room = g_Level.Rooms[roomNumber];

		if ((x > (room.Position.x + BLOCK(1)) && x < (room.Position.x + (room.XSize - 1) * BLOCK(1))) &&
			(y > room.TopHeight && y < room.BottomHeight) &&
			(z > (room.Position.z + BLOCK(1)) && z < (room.Position.z + (room.ZSize - 1) * BLOCK(1))))
		{
			auto pointColl = GetPointCollision(Vector3i(x, y, z), roomNumber);

			if (pointColl.GetFloorHeight() == NO_HEIGHT || y > pointColl.GetFloorHeight())
				return NO_VALUE;

			if (y < pointColl.GetCeilingHeight())
				return NO_VALUE;

			if (TestEnvironmentFlags(ENV_FLAG_WATER, room.flags) ||
				TestEnvironmentFlags(ENV_FLAG_WIND, room.flags))
			{
				return pointColl.GetRoomNumber();
			}

			return NO_VALUE;
		}
	}

	return NO_VALUE;
}

namespace TEN::Collision::Room
{
	// TODO: Can use floordata's GetRoomGridCoord()?
	FloorInfo* GetSector(ROOM_INFO* room, int x, int z)
	{
		int sectorX = std::clamp(x / BLOCK(1), 0, room->XSize - 1);
		int sectorZ = std::clamp(z / BLOCK(1), 0, room->ZSize - 1);

		int sectorID = sectorZ + (sectorX * room->ZSize);
		if (sectorID > room->Sectors.size())
			return nullptr;

		return &room->Sectors[sectorID];
	}
}

GameBoundingBox& GetBoundsAccurate(const MESH_INFO& mesh, bool getVisibilityBox)
{
	static auto bounds = GameBoundingBox();

	if (getVisibilityBox)
	{
		bounds = Statics[mesh.staticNumber].visibilityBox * mesh.scale;
	}
	else
	{
		bounds = Statics[mesh.staticNumber].collisionBox * mesh.scale;
	}

	return bounds;
}

bool IsPointInRoom(const Vector3i& pos, int roomNumber)
{
	const auto& room = g_Level.Rooms[roomNumber];

	if (!room.Active())
		return false;

	if (pos.z >= (room.Position.z + BLOCK(1)) && pos.z <= (room.Position.z + BLOCK(room.ZSize - 1)) &&
		pos.y <= room.BottomHeight && pos.y > room.TopHeight &&
		pos.x >= (room.Position.x + BLOCK(1)) && pos.x <= (room.Position.x + BLOCK(room.XSize - 1)))
	{
		return true;
	}

	return false;
}

int FindRoomNumber(const Vector3i& pos, int startRoomNumber, bool onlyNeighbors)
{
	if (startRoomNumber != NO_VALUE && startRoomNumber < g_Level.Rooms.size())
	{
		const auto& room = g_Level.Rooms[startRoomNumber];
		for (int neighborRoomNumber : room.NeighborRoomNumbers)
		{
			const auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
			if (neighborRoomNumber != startRoomNumber && IsPointInRoom(pos, neighborRoomNumber))
			{
				return neighborRoomNumber;
			}
		}
	}

	if (!onlyNeighbors)
	{
		for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
		{
			if (IsPointInRoom(pos, roomNumber))
				return roomNumber;
		}
	}

	return (startRoomNumber != NO_VALUE) ? startRoomNumber : 0;
}

Vector3i GetRoomCenter(int roomNumber)
{
	const auto& room = g_Level.Rooms[roomNumber];

	int halfLength = BLOCK(room.XSize) / 2;
	int halfDepth = BLOCK(room.ZSize) / 2;
	int halfHeight = (room.TopHeight - room.BottomHeight) / 2;

	// Calculate and return center.
	return Vector3i(
		room.Position.x + halfLength,
		room.BottomHeight + halfHeight,
		room.Position.z + halfDepth);
}

std::vector<int> GetNeighborRoomNumbers(int roomNumber, unsigned int searchDepth, std::vector<int>& visitedRoomNumbers)
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
		room.NeighborRoomNumbers = GetNeighborRoomNumbers(roomNumber, NEIGHBOR_ROOM_SEARCH_DEPTH);
	}

	// Add flipped variations of itself.
	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = g_Level.Rooms[roomNumber];
		if (room.flippedRoom == NO_VALUE)
			continue;

		if (!Contains(room.NeighborRoomNumbers, room.flippedRoom))
			room.NeighborRoomNumbers.push_back(room.flippedRoom);

		auto& flippedRoom = g_Level.Rooms[room.flippedRoom];
		if (!Contains(flippedRoom.NeighborRoomNumbers, roomNumber))
			flippedRoom.NeighborRoomNumbers.push_back(roomNumber);
	}
}
