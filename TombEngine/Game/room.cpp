#include "framework.h"
#include "Game/room.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Renderer/Renderer.h"
#include "Math/Math.h"
#include "Objects/game_object_ids.h"
#include "Physics/Physics.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Physics;
using namespace TEN::Renderer;
using namespace TEN::Utils;

bool FlipStatus = false;
bool FlipStats[MAX_FLIPMAP];
int  FlipMap[MAX_FLIPMAP];

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];

BoundingOrientedBox MESH_INFO::GetObb() const
{
	return GetBoundsAccurate(*this, false).ToBoundingOrientedBox(pos);
}

BoundingOrientedBox MESH_INFO::GetVisibilityObb() const
{
	return GetBoundsAccurate(*this, true).ToBoundingOrientedBox(pos);
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
		auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		// Add bridge.
		if (item.IsBridge())
		{
			auto& bridge = GetBridgeObject(item);
			bridge.Initialize(item);
		}
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
			bridge.DeassignSectors(item);
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

			// Regenerate neighbor room collision meshes.
			for (int neightborRoomNumber : room.neighbors)
			{
				auto& neighborRoom = g_Level.Rooms[neightborRoomNumber];
				TEN::Collision::Room::GenerateRoomCollisionMesh(neighborRoom);
			}
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

	static int GetSurfaceTriangleVertexY(const FloorInfo& sector, int relX, int relZ, int triID, bool isFloor)
	{
		constexpr auto AXIS_OFFSET = -BLOCK(0.5f);
		constexpr auto HEIGHT_STEP = BLOCK(1 / 32.0f);

		const auto& tri = isFloor ? sector.FloorSurface.Triangles[triID] : sector.CeilingSurface.Triangles[triID];

		relX += AXIS_OFFSET;
		relZ += AXIS_OFFSET;

		auto normal = tri.Plane.Normal();
		float relPlaneHeight = -((normal.x * relX) + (normal.z * relZ)) / normal.y;
		return (int)RoundToStep(tri.Plane.D() + relPlaneHeight, HEIGHT_STEP);
	}

	static Vector3 GetRawSurfaceTriangleNormal(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
	{
		auto edge0 = vertex1 - vertex0;
		auto edge1 = vertex2 - vertex0;
		auto normal = edge0.Cross(edge1);
		normal.Normalize();
		return normal;
	}

	static void CollectSectorCollisionTriangles(CollisionMesh& collMesh, const FloorInfo& sector,
												const FloorInfo& prevSectorX, const FloorInfo& nextSectorX,
												const FloorInfo& prevSectorZ, const FloorInfo& nextSectorZ,
												bool isXEnd, bool isZEnd)
	{
		constexpr auto NORTH_WALL_NORMAL	  = Vector3(0.0f, 0.0f, 1.0f);
		constexpr auto SOUTH_WALL_NORMAL	  = Vector3(0.0f, 0.0f, -1.0f);
		constexpr auto EAST_WALL_NORMAL		  = Vector3(1.0f, 0.0f, 0.0f);
		constexpr auto WEST_WALL_NORMAL		  = Vector3(-1.0f, 0.0f, 0.0f);
		constexpr auto NORTH_EAST_WALL_NORMAL = Vector3(SQRT_2 / 2, 0.0f, SQRT_2 / 2);
		constexpr auto NORTH_WEST_WALL_NORMAL = Vector3(-SQRT_2 / 2, 0.0f, SQRT_2 / 2);
		constexpr auto SOUTH_EAST_WALL_NORMAL = Vector3(SQRT_2 / 2, 0.0f, -SQRT_2 / 2);
		constexpr auto SOUTH_WEST_WALL_NORMAL = Vector3(-SQRT_2 / 2, 0.0f, -SQRT_2 / 2);

		constexpr auto REL_CORNER_0 = Vector2i(0, 0);
		constexpr auto REL_CORNER_1 = Vector2i(0, BLOCK(1));
		constexpr auto REL_CORNER_2 = Vector2i(BLOCK(1), BLOCK(1));
		constexpr auto REL_CORNER_3 = Vector2i(BLOCK(1), 0);

		// Calculate 2D corner positions.
		auto corner0 = sector.Position + REL_CORNER_0;
		auto corner1 = sector.Position + REL_CORNER_1;
		auto corner2 = sector.Position + REL_CORNER_2;
		auto corner3 = sector.Position + REL_CORNER_3;

		// Collect triangles.
		bool isFloor = true;
		for (int i = 0; i < 2; i++)
		{
			const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;

			bool isSurfSplit = sector.IsSurfaceSplit(isFloor);
			bool isSurfSplitAngle0 = (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

			// 1) Generate surface triangle 0.
			auto surfTri0Vertices = std::array<Vector3, CollisionTriangle::VERTEX_COUNT>{};
			auto surfTri0Normal = Vector3::Zero;
			if (!isSurfSplit || isSurfSplitAngle0)
			{
				auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor), corner0.y);
				auto vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
				auto vertex2 = Vector3(corner2.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
				surfTri0Vertices = { vertex0, vertex1, vertex2 };
				surfTri0Normal = GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2)* (isFloor ? -1 : 1);
			}
			else
			{
				auto vertex0 = Vector3(corner1.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
				auto vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
				auto vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 0, isFloor), corner3.y);
				surfTri0Vertices = { vertex0, vertex1, vertex2 };
				surfTri0Normal = GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1);
			}
		
			// 2) Generate surface triangle 1.
			auto surfTri1Vertices = std::array<Vector3, CollisionTriangle::VERTEX_COUNT>{};
			auto surfTri1Normal = Vector3::Zero;
			if (!isSurfSplit || isSurfSplitAngle0)
			{
				auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
				auto vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 1, isFloor), corner2.y);
				auto vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
				surfTri1Vertices = { vertex0, vertex1, vertex2 };
				surfTri1Normal = GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1);
			}
			else
			{
				auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
				auto vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 1, isFloor), corner1.y);
				auto vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
				surfTri1Vertices = { vertex0, vertex1, vertex2 };
				surfTri1Normal = GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1);
			}

			bool isSurf0Wall = sector.IsWall(0);
			bool isSurf1Wall = sector.IsWall(1);
			bool isSurfTri0Portal = (surface.Triangles[0].PortalRoomNumber != NO_VALUE);
			bool isSurfTri1Portal = (surface.Triangles[1].PortalRoomNumber != NO_VALUE);

			// 3) Collect surface triangles.
			if (!isSurf0Wall)
			{
				int portalRoomNumber = isSurfTri0Portal ? surface.Triangles[0].PortalRoomNumber : NO_VALUE;
				collMesh.InsertTriangle(surfTri0Vertices[0], surfTri0Vertices[1], surfTri0Vertices[2], surfTri0Normal, portalRoomNumber);
			}
			if (!isSurf1Wall)
			{
				int portalRoomNumber = isSurfTri1Portal ? surface.Triangles[1].PortalRoomNumber : NO_VALUE;
				collMesh.InsertTriangle(surfTri1Vertices[0], surfTri1Vertices[1], surfTri1Vertices[2], surfTri1Normal, portalRoomNumber);
			}

			// 4) Generate and collect diagonal wall triangles.
			if (sector.IsSurfaceSplit(isFloor) && !(isSurf0Wall && isSurf1Wall)) // Can set wall.
			{
				// Full wall.
				if ((isSurf0Wall || isSurf1Wall) && isFloor)
				{
					if (isSurfSplitAngle0)
					{
						int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, isSurf0Wall ? 1 : 0, true);
						int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, isSurf0Wall ? 1 : 0, true);
						int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, isSurf0Wall ? 1 : 0, false);
						int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, isSurf0Wall ? 1 : 0, false);

						auto vertex0 = Vector3(corner0.x, floorHeight0, corner0.y);
						auto vertex1 = Vector3(corner2.x, floorHeight1, corner2.y);
						auto vertex2 = Vector3(corner0.x, ceilHeight0, corner0.y);
						auto vertex3 = Vector3(corner2.x, ceilHeight1, corner2.y);

						if (vertex0 != vertex2)
							collMesh.InsertTriangle(vertex0, vertex1, vertex2, isSurf0Wall ? SOUTH_EAST_WALL_NORMAL : NORTH_WEST_WALL_NORMAL);
						if (vertex1 != vertex3)
							collMesh.InsertTriangle(vertex1, vertex2, vertex3, isSurf0Wall ? SOUTH_EAST_WALL_NORMAL : NORTH_WEST_WALL_NORMAL);
					}
					else
					{
						int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, isSurf0Wall ? 1 : 0, true);
						int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, isSurf0Wall ? 1 : 0, true);
						int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, isSurf0Wall ? 1 : 0, false);
						int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, isSurf0Wall ? 1 : 0, false);

						auto vertex0 = Vector3(corner1.x, floorHeight0, corner1.y);
						auto vertex1 = Vector3(corner3.x, floorHeight1, corner3.y);
						auto vertex2 = Vector3(corner1.x, ceilHeight0, corner1.y);
						auto vertex3 = Vector3(corner3.x, ceilHeight1, corner3.y);

						if (vertex0 != vertex2)
							collMesh.InsertTriangle(vertex0, vertex1, vertex2, isSurf0Wall ? SOUTH_WEST_WALL_NORMAL : NORTH_EAST_WALL_NORMAL);
						if (vertex1 != vertex3)
							collMesh.InsertTriangle(vertex1, vertex2, vertex3, isSurf0Wall ? SOUTH_WEST_WALL_NORMAL : NORTH_EAST_WALL_NORMAL);
					}
				}
				// Step wall.
				else if (!(isSurf0Wall || isSurf1Wall) && !(isSurfTri0Portal && isSurfTri1Portal))
				{
					if (isSurfSplitAngle0)
					{
						// TODO: Check when diagonal criss-cross becomes possible.
						bool isSecondCrissCrossCase = (isFloor ? (surfTri0Vertices[2].y < surfTri1Vertices[1].y) : !(surfTri0Vertices[2].y < surfTri1Vertices[1].y));
						if (surfTri0Vertices[0] != surfTri1Vertices[0])
						{
							const auto& normal0 = ((surfTri0Vertices[0].y > surfTri1Vertices[0].y) ? NORTH_WEST_WALL_NORMAL : SOUTH_EAST_WALL_NORMAL) * (isFloor ? 1 : -1);
							isSecondCrissCrossCase ?
								collMesh.InsertTriangle(surfTri0Vertices[0], surfTri0Vertices[2], surfTri1Vertices[1], normal0) :
								collMesh.InsertTriangle(surfTri0Vertices[0], surfTri1Vertices[0], surfTri0Vertices[2], normal0);
						}
						if (surfTri0Vertices[2] != surfTri1Vertices[1])
						{
							const auto& normal1 = ((surfTri0Vertices[2].y > surfTri1Vertices[1].y) ? NORTH_WEST_WALL_NORMAL : SOUTH_EAST_WALL_NORMAL) * (isFloor ? 1 : -1);
							isSecondCrissCrossCase ?
								collMesh.InsertTriangle(surfTri0Vertices[0], surfTri1Vertices[0], surfTri1Vertices[1], normal1) :
								collMesh.InsertTriangle(surfTri1Vertices[0], surfTri0Vertices[2], surfTri1Vertices[1], normal1);
						}
					}
					else
					{
						// TODO: Check when diagonal criss-cross becomes possible.
						bool isSecondCrissCrossCase = (isFloor ? (surfTri1Vertices[2].y < surfTri0Vertices[2].y) : !(surfTri1Vertices[2].y < surfTri0Vertices[2].y));
						if (surfTri0Vertices[0] != surfTri1Vertices[1])
						{
							const auto& normal0 = ((surfTri0Vertices[0].y > surfTri1Vertices[1].y) ? NORTH_EAST_WALL_NORMAL : SOUTH_WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
							isSecondCrissCrossCase ?
								collMesh.InsertTriangle(surfTri1Vertices[1], surfTri1Vertices[2], surfTri0Vertices[2], normal0) :
								collMesh.InsertTriangle(surfTri1Vertices[1], surfTri0Vertices[0], surfTri1Vertices[2], normal0);
						}
						if (surfTri0Vertices[2] != surfTri1Vertices[2])
						{
							const auto& normal1 = ((surfTri0Vertices[2].y > surfTri1Vertices[2].y) ? NORTH_EAST_WALL_NORMAL : SOUTH_WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
							isSecondCrissCrossCase ?
								collMesh.InsertTriangle(surfTri1Vertices[1], surfTri0Vertices[0], surfTri0Vertices[2], normal1) :
								collMesh.InsertTriangle(surfTri0Vertices[0], surfTri1Vertices[2], surfTri0Vertices[2], normal1);
						}
					}
				}
			}

			// 5) Collect cardinal wall triangles on X axis.
			{
				const auto& prevSurface = isFloor ? prevSectorX.FloorSurface : prevSectorX.CeilingSurface;

				bool isPrevSurfSplit = prevSectorX.IsSurfaceSplit(isFloor);
				bool isPrevSurfSplitAngle0 = (prevSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

				bool isPrevSurf0Wall = prevSectorX.IsWall(0);
				bool isPrevSurf1Wall = prevSectorX.IsWall(1);
				bool isPrevSurfTri0Portal = (prevSurface.Triangles[0].PortalRoomNumber != NO_VALUE);
				bool isPrevSurfTri1Portal = (prevSurface.Triangles[1].PortalRoomNumber != NO_VALUE);

				bool useTri0 = (!isSurfSplit || isSurfSplitAngle0);
				bool usePrevTri1 = (!isPrevSurfSplit || isPrevSurfSplitAngle0);

				bool isSurfWall = (useTri0 ? isSurf0Wall : isSurf1Wall);
				bool isPrevSurfWall = (usePrevTri1 ? isPrevSurf1Wall : isPrevSurf0Wall);

				// Wall behind.
				if (!isSurfWall || !isPrevSurfWall) // Can set wall.
				{
					// TODO: Must keep full wall facing current room. The other one should be generated in the previous room.
					// Full wall referencing current sector.
					if ((!isSurfWall && isPrevSurfWall) && isFloor)
					{
						bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
						bool isCeilSurfSplitAngle0 = (sector.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
						bool useCeilTri0 = (!isCeilSurfSplit || isCeilSurfSplitAngle0);

						int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, true);
						int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, useTri0 ? 0 : 1, true);
						int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useCeilTri0 ? 0 : 1, false);
						int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, useCeilTri0 ? 0 : 1, false);

						auto vertex0 = Vector3(corner0.x, floorHeight0, corner0.y);
						auto vertex1 = Vector3(corner1.x, floorHeight1, corner1.y);
						auto vertex2 = Vector3(corner0.x, ceilHeight0, corner0.y);
						auto vertex3 = Vector3(corner1.x, ceilHeight1, corner1.y);

						if (vertex0 != vertex2)
							collMesh.InsertTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL);
						if (vertex1 != vertex3)
							collMesh.InsertTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL);
					}
					// Full wall referencing previous sector.
					else if ((isSurfWall && !isPrevSurfWall) && isFloor)
					{
						bool isPrevCeilSurfSplit = prevSectorX.IsSurfaceSplit(false);
						bool isPrevCeilSurfSplitAngle0 = (prevSectorX.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
						bool usePrevCeilTri1 = (!isPrevCeilSurfSplit || isPrevCeilSurfSplitAngle0);

						int floorHeight0 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevTri1 ? 1 : 0, true);
						int floorHeight1 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, true);
						int ceilHeight0 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevCeilTri1 ? 1 : 0, false);
						int ceilHeight1 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevCeilTri1 ? 1 : 0, false);

						auto vertex0 = Vector3(corner0.x, floorHeight0, corner0.y);
						auto vertex1 = Vector3(corner1.x, floorHeight1, corner1.y);
						auto vertex2 = Vector3(corner0.x, ceilHeight0, corner0.y);
						auto vertex3 = Vector3(corner1.x, ceilHeight1, corner1.y);

						if (vertex0 != vertex2)
							collMesh.InsertTriangle(vertex0, vertex1, vertex2, WEST_WALL_NORMAL);
						if (vertex1 != vertex3)
							collMesh.InsertTriangle(vertex1, vertex2, vertex3, WEST_WALL_NORMAL);
					}
					// Step wall.
					else if (!isSurfWall && !isPrevSurfWall)
					{
						int height0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, isFloor);
						int height1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, useTri0 ? 0 : 1, isFloor);
						int prevHeight0 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevTri1 ? 1 : 0, isFloor);
						int prevHeight1 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, isFloor);

						auto vertex0 = Vector3(corner0.x, height0, corner0.y);
						auto vertex1 = Vector3(corner1.x, height1, corner1.y);
						auto vertex2 = Vector3(corner0.x, prevHeight0, corner0.y);
						auto vertex3 = Vector3(corner1.x, prevHeight1, corner1.y);

						bool isSecondCrissCrossCase = (isFloor ? (vertex1.y < vertex3.y) : !(vertex1.y < vertex3.y));
						if (sector.RoomNumber == prevSectorX.RoomNumber)
						{
							if (vertex0 != vertex2)
							{
								const auto& normal0 = ((vertex0.y > vertex2.y) ? EAST_WALL_NORMAL : WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex2, vertex3, normal0) :
									collMesh.InsertTriangle(vertex0, vertex1, vertex2, normal0);
							}
							if (vertex1 != vertex3)
							{
								const auto& normal1 = ((vertex1.y > vertex3.y) ? EAST_WALL_NORMAL : WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex1, vertex3, normal1) :
									collMesh.InsertTriangle(vertex1, vertex2, vertex3, normal1);
							}
						}
						else // Avoid forming wall beloning to previous sector's room.
						{
							if (isFloor ? (vertex0.y > vertex2.y) : (vertex0.y < vertex2.y))
							{
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex2, vertex3, EAST_WALL_NORMAL) :
									collMesh.InsertTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL);
							}
							if (isFloor ? (vertex1.y > vertex3.y) : (vertex1.y < vertex3.y))
							{
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex1, vertex3, EAST_WALL_NORMAL) :
									collMesh.InsertTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL);
							}
						}
					}
				}

				// TODO: Inaccurate heights.
				// Collect wall portal to previous room's sector.
				if (sector.RoomNumber != prevSectorX.RoomNumber && isFloor)
				{
					bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
					bool isCeilSurfSplitAngle0 = (sector.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool useCeilTri0 = (!isCeilSurfSplit || isCeilSurfSplitAngle0);
					
					bool isPrevFloorSurfSplit = prevSectorX.IsSurfaceSplit(true);
					bool isPrevFloorSurfSplitAngle0 = (prevSectorX.FloorSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool usePrevFloorTri0 = !(!isPrevFloorSurfSplit || isPrevFloorSurfSplitAngle0);
					
					bool isPrevCeilSurfSplit = prevSectorX.IsSurfaceSplit(false);
					bool isPrevCeilSurfSplitAngle0 = (prevSectorX.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool usePrevCeilTri0 = !(!isPrevCeilSurfSplit || isPrevCeilSurfSplitAngle0);

					int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, true);
					int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, useTri0 ? 0 : 1, true);
					int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useCeilTri0 ? 0 : 1, false);
					int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, useCeilTri0 ? 0 : 1, false);

					int prevFloorHeight0 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevFloorTri0 ? 0 : 1, true);
					int prevFloorHeight1 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevFloorTri0 ? 0 : 1, true);
					int prevCeilHeight0 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevCeilTri0 ? 0 : 1, false);
					int prevCeilHeight1 = GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevCeilTri0 ? 0 : 1, false);

					// TODO: Criss-cross.
					auto vertex0 = Vector3(corner0.x, (floorHeight0 < prevFloorHeight0) ? floorHeight0 : prevFloorHeight0, corner0.y);
					auto vertex1 = Vector3(corner1.x, (floorHeight1 < prevFloorHeight1) ? floorHeight1 : prevFloorHeight1, corner1.y);
					auto vertex2 = Vector3(corner0.x, (ceilHeight0 > prevCeilHeight0) ? ceilHeight0 : prevCeilHeight0, corner0.y);
					auto vertex3 = Vector3(corner1.x, (ceilHeight1 > prevCeilHeight1) ? ceilHeight1 : prevCeilHeight1, corner1.y);

					collMesh.InsertTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL, prevSectorX.RoomNumber);
					collMesh.InsertTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL, prevSectorX.RoomNumber);
				}

				// Collect wall portal to next room's sector.
				if (sector.RoomNumber != nextSectorX.RoomNumber && isFloor)
				{
					bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
					bool isCeilSurfSplitAngle0 = (sector.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool useCeilTri0 = (!isCeilSurfSplit || isCeilSurfSplitAngle0);

					bool isNextFloorSurfSplit = nextSectorX.IsSurfaceSplit(true);
					bool isNextFloorSurfSplitAngle0 = (nextSectorX.FloorSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool useNextFloorTri0 = !(!isNextFloorSurfSplit || isNextFloorSurfSplitAngle0);

					bool isNextCeilSurfSplit = nextSectorX.IsSurfaceSplit(false);
					bool isNextCeilSurfSplitAngle0 = (nextSectorX.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool useNextCeilTri0 = !(!isNextCeilSurfSplit || isNextCeilSurfSplitAngle0);

					int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useTri0 ? 0 : 1, true);
					int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useTri0 ? 0 : 1, true);
					int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useCeilTri0 ? 0 : 1, false);
					int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useCeilTri0 ? 0 : 1, false);

					int nextFloorHeight0 = GetSurfaceTriangleVertexY(nextSectorX, REL_CORNER_0.x, REL_CORNER_0.y, !useNextFloorTri0 ? 0 : 1, true);
					int nextFloorHeight1 = GetSurfaceTriangleVertexY(nextSectorX, REL_CORNER_1.x, REL_CORNER_1.y, !useNextFloorTri0 ? 0 : 1, true);
					int nextCeilHeight0 = GetSurfaceTriangleVertexY(nextSectorX, REL_CORNER_0.x, REL_CORNER_0.y, !useNextCeilTri0 ? 0 : 1, false);
					int nextCeilHeight1 = GetSurfaceTriangleVertexY(nextSectorX, REL_CORNER_1.x, REL_CORNER_1.y, !useNextCeilTri0 ? 0 : 1, false);

					// TODO: Criss-cross.
					auto vertex0 = Vector3(corner3.x, (floorHeight0 < nextFloorHeight0) ? floorHeight0 : nextFloorHeight0, corner3.y);
					auto vertex1 = Vector3(corner2.x, (floorHeight1 < nextFloorHeight1) ? floorHeight1 : nextFloorHeight1, corner2.y);
					auto vertex2 = Vector3(corner3.x, (ceilHeight0 > nextCeilHeight0) ? ceilHeight0 : nextCeilHeight0, corner3.y);
					auto vertex3 = Vector3(corner2.x, (ceilHeight1 > nextCeilHeight1) ? ceilHeight1 : nextCeilHeight1, corner2.y);

					collMesh.InsertTriangle(vertex0, vertex1, vertex2, WEST_WALL_NORMAL, nextSectorX.RoomNumber);
					collMesh.InsertTriangle(vertex1, vertex2, vertex3, WEST_WALL_NORMAL, nextSectorX.RoomNumber);
				}

				// Collect end wall.
				if (isXEnd && isFloor)
				{
					bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
					bool isCeilSurfSplitAngle0 = (sector.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool useCeilTri0 = (!isCeilSurfSplit || isCeilSurfSplitAngle0);

					int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useTri0 ? 0 : 1, true);
					int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useTri0 ? 0 : 1, true);
					int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useCeilTri0 ? 0 : 1, false);
					int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useCeilTri0 ? 0 : 1, false);

					auto endVertex0 = Vector3(corner2.x, floorHeight0, corner2.y);
					auto endVertex1 = Vector3(corner3.x, floorHeight1, corner3.y);
					auto endVertex2 = Vector3(corner2.x, ceilHeight0, corner2.y);
					auto endVertex3 = Vector3(corner3.x, ceilHeight1, corner3.y);

					collMesh.InsertTriangle(endVertex0, endVertex1, endVertex2, WEST_WALL_NORMAL);
					collMesh.InsertTriangle(endVertex1, endVertex2, endVertex3, WEST_WALL_NORMAL);
				}
			}

			// 6) Collect cardinal wall triangles on Z axis.
			{
				const auto& prevSurface = isFloor ? prevSectorZ.FloorSurface : prevSectorZ.CeilingSurface;

				bool isPrevSurfSplit = prevSectorZ.IsSurfaceSplit(isFloor);
				bool isPrevSurfSplitAngle0 = (prevSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

				bool isPrevSurf0Wall = prevSectorZ.IsWall(0);
				bool isPrevSurf1Wall = prevSectorZ.IsWall(1);
				bool isPrevSurfTri0Portal = (prevSurface.Triangles[0].PortalRoomNumber != NO_VALUE);
				bool isPrevSurfTri1Portal = (prevSurface.Triangles[1].PortalRoomNumber != NO_VALUE);

				bool useTri0 = !(isSurfSplit || !isSurfSplitAngle0);
				bool usePrevTri1 = !(isPrevSurfSplit || !isPrevSurfSplitAngle0);

				bool isSurfWall = (useTri0 ? isSurf0Wall : isSurf1Wall);
				bool isPrevSurfWall = (usePrevTri1 ? isPrevSurf1Wall : isPrevSurf0Wall);

				// Wall behind.
				if (!isSurfWall || !isPrevSurfWall) // Can set wall.
				{
					// Full wall referencing current sector.
					if ((!isSurfWall && isPrevSurfWall) && isFloor)
					{
						bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
						bool isCeilSurfSplitAngle0 = (sector.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
						bool useCeilTri0 = !(isCeilSurfSplit || !isCeilSurfSplitAngle0);

						// TODO: Check.
						int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, useTri0 ? 0 : 1, true);
						int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, true);
						int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, useCeilTri0 ? 0 : 1, false);
						int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useCeilTri0 ? 0 : 1, false);

						auto vertex0 = Vector3(corner3.x, floorHeight0, corner3.y);
						auto vertex1 = Vector3(corner0.x, floorHeight1, corner0.y);
						auto vertex2 = Vector3(corner3.x, ceilHeight0, corner3.y);
						auto vertex3 = Vector3(corner0.x, ceilHeight1, corner0.y);

						if (vertex0 != vertex2)
							collMesh.InsertTriangle(vertex0, vertex1, vertex2, NORTH_WALL_NORMAL);
						if (vertex1 != vertex3)
							collMesh.InsertTriangle(vertex1, vertex2, vertex3, NORTH_WALL_NORMAL);
					}
					// Full wall referencing previous sector.
					else if ((isSurfWall && !isPrevSurfWall) && isFloor)
					{
						bool isPrevCeilSurfSplit = prevSectorZ.IsSurfaceSplit(false);
						bool isPrevCeilSurfSplitAngle0 = (prevSectorZ.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
						bool usePrevCeilTri1 = !(isPrevCeilSurfSplit || !isPrevCeilSurfSplitAngle0);

						int floorHeight0 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, true);
						int floorHeight1 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, usePrevTri1 ? 1 : 0, true);
						int ceilHeight0 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, usePrevCeilTri1 ? 1 : 0, false);
						int ceilHeight1 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, usePrevCeilTri1 ? 1 : 0, false);

						auto vertex0 = Vector3(corner3.x, floorHeight0, corner3.y);
						auto vertex1 = Vector3(corner0.x, floorHeight1, corner0.y);
						auto vertex2 = Vector3(corner3.x, ceilHeight0, corner3.y);
						auto vertex3 = Vector3(corner0.x, ceilHeight1, corner0.y);

						if (vertex0 != vertex2)
							collMesh.InsertTriangle(vertex0, vertex1, vertex2, SOUTH_WALL_NORMAL);
						if (vertex1 != vertex3)
							collMesh.InsertTriangle(vertex1, vertex2, vertex3, SOUTH_WALL_NORMAL);
					}
					// Step wall.
					else if (!isSurfWall && !isPrevSurfWall)
					{
						int height0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, useTri0 ? 0 : 1, isFloor);
						int height1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, isFloor);
						int prevHeight0 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, isFloor);
						int prevHeight1 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, usePrevTri1 ? 1 : 0, isFloor);

						auto vertex0 = Vector3(corner3.x, height0, corner3.y);
						auto vertex1 = Vector3(corner0.x, height1, corner0.y);
						auto vertex2 = Vector3(corner3.x, prevHeight0, corner3.y);
						auto vertex3 = Vector3(corner0.x, prevHeight1, corner0.y);

						bool isSecondCrissCrossCase = isFloor ? (vertex1.y < vertex3.y) : !(vertex1.y < vertex3.y);
						if (sector.RoomNumber == prevSectorZ.RoomNumber)
						{
							if (vertex0 != vertex2)
							{
								const auto& normal0 = ((vertex0.y > vertex2.y) ? NORTH_WALL_NORMAL : SOUTH_WALL_NORMAL) * (isFloor ? 1 : -1);
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex2, vertex3, normal0) :
									collMesh.InsertTriangle(vertex0, vertex1, vertex2, normal0);
							}
							if (vertex1 != vertex3)
							{
								const auto& normal1 = ((vertex1.y > vertex3.y) ? NORTH_WALL_NORMAL : SOUTH_WALL_NORMAL) * (isFloor ? 1 : -1);
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex1, vertex3, normal1) :
									collMesh.InsertTriangle(vertex1, vertex2, vertex3, normal1);
							}
						}
						else // Avoid forming wall beloning to previous sector's room.
						{
							if (isFloor ? (vertex0.y > vertex2.y) : (vertex0.y < vertex2.y))
							{
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex2, vertex3, NORTH_WALL_NORMAL) :
									collMesh.InsertTriangle(vertex0, vertex1, vertex2, NORTH_WALL_NORMAL);
							}
							if (isFloor ? (vertex1.y > vertex3.y) : (vertex1.y < vertex3.y))
							{
								isSecondCrissCrossCase ?
									collMesh.InsertTriangle(vertex0, vertex1, vertex3, NORTH_WALL_NORMAL) :
									collMesh.InsertTriangle(vertex1, vertex2, vertex3, NORTH_WALL_NORMAL);
							}
						}
					}
				}

				// Collect wall portal to previous sector.
				if (sector.RoomNumber != prevSectorZ.RoomNumber && isFloor)
				{
					// TODO: Check sectors with different triangle normals.
					int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, useTri0 ? 0 : 1, true);
					int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, true);
					int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useTri0 ? 0 : 1, false);
					int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, false);

					int prevFloorHeight0 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, useTri0 ? 0 : 1, true);
					int prevFloorHeight1 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, useTri0 ? 0 : 1, true);
					int prevCeilHeight0 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, !useTri0 ? 0 : 1, false);
					int prevCeilHeight1 = GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, useTri0 ? 0 : 1, false);

					// TODO: Criss-cross.
					auto vertex0 = Vector3(corner3.x, (floorHeight0 < prevFloorHeight0) ? floorHeight0 : prevFloorHeight0, corner3.y);
					auto vertex1 = Vector3(corner0.x, (floorHeight1 < prevFloorHeight1) ? floorHeight1 : prevFloorHeight1, corner0.y);
					auto vertex2 = Vector3(corner3.x, (ceilHeight0 > prevCeilHeight0) ? ceilHeight0 : prevCeilHeight0, corner3.y);
					auto vertex3 = Vector3(corner0.x, (ceilHeight1 > prevCeilHeight1) ? ceilHeight1 : prevCeilHeight1, corner0.y);

					collMesh.InsertTriangle(vertex0, vertex1, vertex2, NORTH_WALL_NORMAL, prevSectorZ.RoomNumber);
					collMesh.InsertTriangle(vertex1, vertex2, vertex3, NORTH_WALL_NORMAL, prevSectorZ.RoomNumber);
				}

				// Collect end wall.
				if (isZEnd && isFloor)
				{
					bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
					bool isCeilSurfSplitAngle0 = (sector.CeilingSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
					bool useCeilTri0 = !(isCeilSurfSplit || !isCeilSurfSplitAngle0);

					int floorHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, !useTri0 ? 0 : 1, true);
					int floorHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useTri0 ? 0 : 1, true);
					int ceilHeight0 = GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, !useCeilTri0 ? 0 : 1, false);
					int ceilHeight1 = GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useCeilTri0 ? 0 : 1, false);

					auto endVertex0 = Vector3(corner1.x, floorHeight0, corner1.y);
					auto endVertex1 = Vector3(corner2.x, floorHeight1, corner2.y);
					auto endVertex2 = Vector3(corner1.x, ceilHeight0, corner1.y);
					auto endVertex3 = Vector3(corner2.x, ceilHeight1, corner2.y);

					collMesh.InsertTriangle(endVertex0, endVertex1, endVertex2, SOUTH_WALL_NORMAL, nextSectorZ.SidePortalRoomNumber);
					collMesh.InsertTriangle(endVertex1, endVertex2, endVertex3, SOUTH_WALL_NORMAL, nextSectorZ.SidePortalRoomNumber);
				}
			}

			isFloor = false;
		}
	}

	void GenerateRoomCollisionMesh(ROOM_INFO& room)
	{
		room.CollisionMesh = CollisionMesh();

		// Run through room sectors (ignoring border).
		for (int x = 1; x < (room.xSize - 1); x++)
		{
			for (int z = 1; z < (room.zSize - 1); z++)
			{
				auto& sector = room.floor[(x * room.zSize) + z];

				// Get previous X sector.
				const auto* prevSectorX = &room.floor[((x - 1) * room.zSize) + z];
				if (prevSectorX->SidePortalRoomNumber != NO_VALUE)
				{
					const auto& prevRoomX = g_Level.Rooms[prevSectorX->SidePortalRoomNumber];
					auto prevRoomGridCoordX = GetRoomGridCoord(prevSectorX->SidePortalRoomNumber, prevSectorX->Position.x, prevSectorX->Position.y);

					prevSectorX = &prevRoomX.floor[(prevRoomGridCoordX.x * prevRoomX.zSize) + prevRoomGridCoordX.y];
				}

				// Get next X sector.
				const auto* nextSectorX = &room.floor[((x + 1) * room.zSize) + z];
				if (nextSectorX->SidePortalRoomNumber != NO_VALUE)
				{
					const auto& nextRoomX = g_Level.Rooms[nextSectorX->SidePortalRoomNumber];
					auto nextRoomGridCoordX = GetRoomGridCoord(nextSectorX->SidePortalRoomNumber, nextSectorX->Position.x, nextSectorX->Position.y);

					nextSectorX = &nextRoomX.floor[(nextRoomGridCoordX.x * nextRoomX.zSize) + nextRoomGridCoordX.y];
				}

				// Get previous Z sector.
				const auto* prevSectorZ = &room.floor[(x * room.zSize) + (z - 1)];
				if (prevSectorZ->SidePortalRoomNumber != NO_VALUE)
				{
					const auto& prevRoomZ = g_Level.Rooms[prevSectorZ->SidePortalRoomNumber];
					auto prevRoomGridCoordZ = GetRoomGridCoord(prevSectorZ->SidePortalRoomNumber, prevSectorZ->Position.x, prevSectorZ->Position.y);

					prevSectorZ = &prevRoomZ.floor[(prevRoomGridCoordZ.x * prevRoomZ.zSize) + prevRoomGridCoordZ.y];
				}

				// Get next Z sector.
				const auto* nextSectorZ = &room.floor[(x * room.zSize) + (z + 1)];
				if (nextSectorZ->SidePortalRoomNumber != NO_VALUE)
				{
					const auto& nextRoomZ = g_Level.Rooms[nextSectorZ->SidePortalRoomNumber];
					auto nextRoomGridCoordZ = GetRoomGridCoord(nextSectorZ->SidePortalRoomNumber, nextSectorZ->Position.x, nextSectorZ->Position.y);

					nextSectorZ = &nextRoomZ.floor[(nextRoomGridCoordZ.x * nextRoomZ.zSize) + nextRoomGridCoordZ.y];
				}

				// Get end status.
				bool isXEnd = (x == (room.xSize - 2));
				if (isXEnd)
				{
					const auto& nextSectorX = room.floor[((x + 1) * room.zSize) + z];
					isXEnd = (nextSectorX.SidePortalRoomNumber == NO_VALUE);
				}
				bool isZEnd = (z == (room.zSize - 2));
				if (isZEnd)
				{
					const auto& nextSectorZ = room.floor[(x * room.zSize) + (z + 1)];
					isZEnd = (nextSectorZ.SidePortalRoomNumber == NO_VALUE);
				}

				// Collect triangles for collision mesh.
				CollectSectorCollisionTriangles(room.CollisionMesh, sector, *prevSectorX, *nextSectorX , *prevSectorZ, *nextSectorZ, isXEnd, isZEnd);
			}
		}

		room.CollisionMesh.GenerateTree();
	}
}
