#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/Attractor.h"
#include "Game/collision/Point.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Renderer/Renderer.h"
#include "Math/Math.h"
#include "Objects/game_object_ids.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Attractor;
using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Utils;

bool FlipStatus = false;
bool FlipStats[MAX_FLIPMAP];
int  FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

bool AttractorHandler::BvhNode::IsLeaf() const
{
	return (LeftChildID == NO_VALUE && RightChildID == NO_VALUE);
}

AttractorHandler::Bvh::Bvh(const std::vector<AttractorObject>& attracs)
{
	auto attracIds = std::vector<int>{};
	attracIds.reserve(attracs.size());
	for (int i = 0; i < attracs.size(); ++i)
		attracIds.push_back(i);

	Generate(attracs, attracIds, 0, (int)attracs.size());
}

std::vector<AttractorObject*> AttractorHandler::Bvh::GetBoundedAttractors(const BoundingSphere& sphere, std::vector<AttractorObject>& attracs)
{
	if (Nodes.empty())
		return {};

	auto boundedAttracs = std::vector<AttractorObject*>{};

	std::function<void(int)> traverseBvh = [&](int nodeID)
	{
		// Invalid node; return early.
		if (nodeID == NO_VALUE)
			return;

		const auto& node = Nodes[nodeID];

		// Test node intersection.
		if (!node.Box.Intersects(sphere))
			return;

		// Traverse nodes.
		if (node.IsLeaf())
		{
			for (int attracID : node.AttractorIds)
			{
				if (attracs[attracID].GetBox().Intersects(sphere))
					boundedAttracs.push_back(&attracs[attracID]);
			}
		}
		else
		{
			traverseBvh(node.LeftChildID);
			traverseBvh(node.RightChildID);
		}
	};

	// Traverse BVH from root node.
	traverseBvh((int)Nodes.size() - 1);
	return boundedAttracs;
}

int AttractorHandler::Bvh::Generate(const std::vector<AttractorObject>& attracs, const std::vector<int>& attracIds, int start, int end)
{
	constexpr auto ATTRAC_COUNT_PER_LEAF_MAX = 4;

	// FAILSAFE.
	if (start >= end)
		return NO_VALUE;

	auto node = BvhNode{};

	// Combine boxes.
	node.Box = attracs[attracIds[start]].GetBox();
	for (int i = (start + 1); i < end; i++)
		node.Box = Geometry::CombineBoundingBoxes(node.Box, attracs[attracIds[i]].GetBox());

	// Leaf node.
	if ((end - start) <= ATTRAC_COUNT_PER_LEAF_MAX)
	{
		node.AttractorIds.insert(node.AttractorIds.end(), attracIds.begin() + start, attracIds.begin() + end);
		Nodes.push_back(node);
		return int(Nodes.size() - 1);
	}
	// Split node.
	else
	{
		int mid = (start + end) / 2;
		node.LeftChildID = Generate(attracs, attracIds, start, mid);
		node.RightChildID = Generate(attracs, attracIds, mid, end);
		Nodes.push_back(node);
		return int(Nodes.size() - 1);
	}
}

AttractorHandler::AttractorHandler(std::vector<AttractorObject>& attracs)
{
	_attractors = attracs;
}

std::vector<AttractorObject>& AttractorHandler::GetAttractors()
{
	return _attractors;
}

std::vector<AttractorObject*> AttractorHandler::GetBoundedAttractors(const BoundingSphere& sphere)
{
	return _bvh.GetBoundedAttractors(sphere, _attractors);
}

void AttractorHandler::InsertAttractor(const AttractorObject& attrac)
{
	_attractors.push_back(attrac);
}

void AttractorHandler::GenerateBvh()
{
	_bvh = Bvh(_attractors);
}

bool ROOM_INFO::Active() const
{
	if (flipNumber == NO_VALUE)
		return true;

	// Since engine swaps whole room memory block but substitutes flippedRoom,
	// must check both original room number and flippedRoom equality,
	// as well as NO_VALUE if checking non-flipped rooms.
	return (!FlipStats[flipNumber] && flippedRoom != index && flippedRoom != NO_VALUE) ||
		   ( FlipStats[flipNumber] && flippedRoom == index);
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
		auto& item = g_Level.Items[itemNumber];
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
		{
			auto& bridge = GetBridgeObject(item);

			UpdateBridgeItem(item, true);
			bridge.GetAttractor().DetachAllPlayers();
		}
	}
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
		if (room.flippedRoom >= 0 && room.flipNumber == group)
		{
			auto& flippedRoom = g_Level.Rooms[room.flippedRoom];

			RemoveRoomFlipItems(room);

			// Detach players from attractors.
			for (auto& attrac : room.Attractors.GetAttractors())
				attrac.DetachAllPlayers();

			// Swap rooms.
			std::swap(room, flippedRoom);
			room.flippedRoom = flippedRoom.flippedRoom;
			flippedRoom.flippedRoom = NO_VALUE;
			room.itemNumber = flippedRoom.itemNumber;
			room.fxNumber = flippedRoom.fxNumber;

			AddRoomFlipItems(room);

			g_Renderer.FlipRooms(roomNumber, room.flippedRoom);

			// Update active room sectors.
			for (auto& sector : room.floor)
				sector.RoomNumber = roomNumber;

			// Update flipped room sectors.
			for (auto& sector : flippedRoom.floor)
				sector.RoomNumber = room.flippedRoom;
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

		if ((x > (room.x + BLOCK(1)) && x < (room.x + (room.xSize - 1) * BLOCK(1))) &&
			(y > room.maxceiling && y < room.minfloor) &&
			(z > (room.z + BLOCK(1)) && z < (room.z + (room.zSize - 1) * BLOCK(1))))
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
		int sectorX = std::clamp(x / BLOCK(1), 0, room->xSize - 1);
		int sectorZ = std::clamp(z / BLOCK(1), 0, room->zSize - 1);

		int sectorID = sectorZ + (sectorX * room->zSize);
		if (sectorID > room->floor.size())
			return nullptr;

		return &room->floor[sectorID];
	}
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
	if (startRoomNumber != NO_VALUE && startRoomNumber < g_Level.Rooms.size())
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

	return (startRoomNumber != NO_VALUE) ? startRoomNumber : 0;
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
		if (room.flippedRoom == NO_VALUE)
			continue;

		if (!Contains(room.neighbors, room.flippedRoom))
			room.neighbors.push_back(room.flippedRoom);

		auto& flippedRoom = g_Level.Rooms[room.flippedRoom];
		if (!Contains(flippedRoom.neighbors, roomNumber))
			flippedRoom.neighbors.push_back(roomNumber);
	}
}
