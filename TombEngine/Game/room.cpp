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
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Collision::Room;
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

bool RoomData::Active() const
{
	if (flipNumber == NO_VALUE)
		return true;

	// Since engine swaps whole room memory block but substitutes flippedRoom,
	// must check both original room number and flippedRoom equality,
	// as well as NO_VALUE if checking non-flipped rooms.
	return (!FlipStats[flipNumber] && flippedRoom != RoomNumber && flippedRoom != NO_VALUE) ||
		   ( FlipStats[flipNumber] && flippedRoom == RoomNumber);
}

void RoomData::GenerateCollisionMesh()
{
	// Define collision mesh description.
	auto desc = CollisionMeshDesc();
	for (int x = 1; x < (XSize - 1); x++)
	{
		for (int z = 1; z < (ZSize - 1); z++)
		{
			const auto& sector = Sectors[(x * ZSize) + z];

			// Get north sector (Z+).
			const auto* sectorNorth = &Sectors[(x * ZSize) + (z + 1)];
			if (sectorNorth->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& room = GetRoom(sectorNorth->SidePortalRoomNumber);
				auto gridCoord = GetRoomGridCoord(sectorNorth->SidePortalRoomNumber, sectorNorth->Position.x, sectorNorth->Position.y);

				sectorNorth = &room.Sectors[(gridCoord.x * room.ZSize) + gridCoord.y];
			}

			// Get south sector (Z-).
			const auto* sectorSouth = &Sectors[(x * ZSize) + (z - 1)];
			if (sectorSouth->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomZ = GetRoom(sectorSouth->SidePortalRoomNumber);
				auto prevRoomGridCoordZ = GetRoomGridCoord(sectorSouth->SidePortalRoomNumber, sectorSouth->Position.x, sectorSouth->Position.y);

				sectorSouth = &prevRoomZ.Sectors[(prevRoomGridCoordZ.x * prevRoomZ.ZSize) + prevRoomGridCoordZ.y];
			}

			// Get east sector (X+).
			const auto* sectorEast = &Sectors[((x + 1) * ZSize) + z];
			if (sectorEast->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& room = GetRoom(sectorEast->SidePortalRoomNumber);
				auto gridCoord = GetRoomGridCoord(sectorEast->SidePortalRoomNumber, sectorEast->Position.x, sectorEast->Position.y);

				sectorEast = &room.Sectors[(gridCoord.x * room.ZSize) + gridCoord.y];
			}

			// Get west sector (X-).
			const auto* sectorWest = &Sectors[((x - 1) * ZSize) + z];
			if (sectorWest->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& room = GetRoom(sectorWest->SidePortalRoomNumber);
				auto gridCoord = GetRoomGridCoord(sectorWest->SidePortalRoomNumber, sectorWest->Position.x, sectorWest->Position.y);

				sectorWest = &room.Sectors[(gridCoord.x * room.ZSize) + gridCoord.y];
			}

			CollectSectorCollisionMeshTriangles(desc, -Position.ToVector3(), sector, *sectorNorth, *sectorSouth, *sectorEast, *sectorWest);
		}
	}

	// Create collision mesh.
	CollisionMesh = TEN::Physics::CollisionMesh(Position.ToVector3(), Quaternion::Identity, desc);
}

void RoomData::CollectSectorCollisionMeshTriangles(CollisionMeshDesc& desc, const Vector3& offset,
												   const FloorInfo& sector,
												   const FloorInfo& sectorNorth, const FloorInfo& sectorSouth,
												   const FloorInfo& sectorEast, const FloorInfo& sectorWest)
{
	constexpr auto SECTOR_SURFACE_COUNT = 2;

	struct SectorVertexData
	{
		struct SurfaceData
		{
			struct TriangleData
			{
				bool IsWall = false;

				Vector3 Vertex0 = Vector3::Zero;
				Vector3 Vertex1 = Vector3::Zero;
				Vector3 Vertex2 = Vector3::Zero;
			};

			struct NeighborData
			{
				bool IsWall = false;

				Vector3 Vertex0 = Vector3::Zero;
				Vector3 Vertex1 = Vector3::Zero;
			};

			bool IsSplit	   = false;
			bool IsSplitAngle0 = false;

			TriangleData Tri0 = {};
			TriangleData Tri1 = {};

			NeighborData NorthNeighbor = {};
			NeighborData SouthNeighbor = {};
			NeighborData EastNeighbor  = {};
			NeighborData WestNeighbor  = {};
		};

		SurfaceData Floor = {};
		SurfaceData Ceil  = {};
	};
	
	auto getSurfaceTriangleVertexY = [](const FloorInfo& sector, int relX, int relZ, int triID, bool isFloor)
	{
		constexpr auto AXIS_OFFSET = -BLOCK(0.5f);
		constexpr auto HEIGHT_STEP = BLOCK(1 / 32.0f);

		const auto& tri = isFloor ? sector.FloorSurface.Triangles[triID] : sector.CeilingSurface.Triangles[triID];

		relX += AXIS_OFFSET;
		relZ += AXIS_OFFSET;

		auto normal = tri.Plane.Normal();
		float relPlaneHeight = -((normal.x * relX) + (normal.z * relZ)) / normal.y;
		return (int)RoundToStep(tri.Plane.D() + relPlaneHeight, HEIGHT_STEP); // FAILSAFE: Round to circumvent plane query error.
	};

	auto generateSectorVertices = [&]()
	{
		constexpr auto REL_CORNER_0 = Vector2i(0, 0);
		constexpr auto REL_CORNER_1 = Vector2i(0, BLOCK(1));
		constexpr auto REL_CORNER_2 = Vector2i(BLOCK(1), BLOCK(1));
		constexpr auto REL_CORNER_3 = Vector2i(BLOCK(1), 0);

		auto sectorVerts = SectorVertexData{};

		// 1) Calculate 2D corner positions.
		// 1---2
		// |   |
		// 0---3
		auto corner0 = sector.Position + REL_CORNER_0;
		auto corner1 = sector.Position + REL_CORNER_1;
		auto corner2 = sector.Position + REL_CORNER_2;
		auto corner3 = sector.Position + REL_CORNER_3;

		// 2) Set vertex data for floor and ceiling.
		bool isFloor = true;
		for (int i = 0; i < SECTOR_SURFACE_COUNT; i++)
		{
			const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;
			auto& surfVertices = isFloor ? sectorVerts.Floor : sectorVerts.Ceil;

			// 2.1) Set surface status data.
			surfVertices.IsSplit = sector.IsSurfaceSplit(isFloor);
			surfVertices.IsSplitAngle0 = (!surfVertices.IsSplit || surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.Tri0.IsWall = sector.IsWall(0);
			surfVertices.Tri1.IsWall = sector.IsWall(1);

			// 2.2) Set surface triangle vertex data.
			// Tri 0: Tri 1:
			// 1---2     1
			// | /     / |
			// 0     0---2
			if (surfVertices.IsSplitAngle0)
			{
				surfVertices.Tri0.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor), corner0.y);
				surfVertices.Tri0.Vertex1 = Vector3(corner1.x, getSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
				surfVertices.Tri0.Vertex2 = Vector3(corner2.x, getSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
			
				surfVertices.Tri1.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
				surfVertices.Tri1.Vertex1 = Vector3(corner2.x, getSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 1, isFloor), corner2.y);
				surfVertices.Tri1.Vertex2 = Vector3(corner3.x, getSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
			}
			// Tri 0: Tri 1:
			// 0---1  1
			//   \ |  | \
			//     2  0---2
			else
			{
				surfVertices.Tri0.Vertex0 = Vector3(corner1.x, getSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
				surfVertices.Tri0.Vertex1 = Vector3(corner2.x, getSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
				surfVertices.Tri0.Vertex2 = Vector3(corner3.x, getSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 0, isFloor), corner3.y);
			
				surfVertices.Tri1.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
				surfVertices.Tri1.Vertex1 = Vector3(corner1.x, getSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 1, isFloor), corner1.y);
				surfVertices.Tri1.Vertex2 = Vector3(corner3.x, getSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
			}

			// 2.3) Set north neighbor data.
			// +---+
			// |   | North
			// 0---1
			// +---+
			// |   | Current
			// +---+
			const auto& surfNorth = isFloor ? sectorNorth.FloorSurface : sectorNorth.CeilingSurface;
			surfVertices.NorthNeighbor.IsWall = sectorNorth.IsWall(1);
			surfVertices.NorthNeighbor.Vertex0 = Vector3(corner1.x, getSurfaceTriangleVertexY(sectorNorth, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner1.y);
			surfVertices.NorthNeighbor.Vertex1 = Vector3(corner2.x, getSurfaceTriangleVertexY(sectorNorth, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner2.y);

			// 2.4) Set south neighbor data.
			// +---+
			// |   | Current
			// +---+
			// 0---1
			// |   | South
			// +---+
			const auto& surfSouth = isFloor ? sectorSouth.FloorSurface : sectorSouth.CeilingSurface;
			surfVertices.SouthNeighbor.IsWall = sectorSouth.IsWall(0);
			surfVertices.SouthNeighbor.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(sectorSouth, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner0.y);
			surfVertices.SouthNeighbor.Vertex1 = Vector3(corner3.x, getSurfaceTriangleVertexY(sectorSouth, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner3.y);

			// 2.5) Set east neighbor data.
			//         +---+ 1---+
			// Current |   | |   | East
			//         +---+ 0---+
			const auto& surfEast = isFloor ? sectorEast.FloorSurface : sectorEast.CeilingSurface;
			bool useEastSurfTri0 = (!sectorEast.IsSurfaceSplit(isFloor) || surfEast.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.EastNeighbor.IsWall = sectorEast.IsWall(useEastSurfTri0 ? 0 : 1);
			surfVertices.EastNeighbor.Vertex0 = Vector3(corner2.x, getSurfaceTriangleVertexY(sectorEast, REL_CORNER_1.x, REL_CORNER_1.y, useEastSurfTri0 ? 0 : 1, isFloor), corner2.y);
			surfVertices.EastNeighbor.Vertex1 = Vector3(corner3.x, getSurfaceTriangleVertexY(sectorEast, REL_CORNER_0.x, REL_CORNER_0.y, useEastSurfTri0 ? 0 : 1, isFloor), corner3.y);

			// 2.6) Set west neighbor data.
			//      +---1 +---+
			// West |   | |   | Current
			//      +---0 +---+
			const auto& surfWest = isFloor ? sectorWest.FloorSurface : sectorWest.CeilingSurface;
			bool useWestSurfTri0 = !(!sectorWest.IsSurfaceSplit(isFloor) || surfWest.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.WestNeighbor.IsWall = sectorWest.IsWall(useWestSurfTri0 ? 0 : 1);
			surfVertices.WestNeighbor.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(sectorWest, REL_CORNER_3.x, REL_CORNER_3.y, useWestSurfTri0 ? 0 : 1, isFloor), corner0.y);
			surfVertices.WestNeighbor.Vertex1 = Vector3(corner1.x, getSurfaceTriangleVertexY(sectorWest, REL_CORNER_2.x, REL_CORNER_2.y, useWestSurfTri0 ? 0 : 1, isFloor), corner1.y);

			isFloor = !isFloor;
		}

		return sectorVerts;
	};

	// Input:
	// 3---2
	// |   |
	// 0---1
	auto insertWallTriangles = [&](bool isFloor, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3)
	{
		//     2     0       3          1
		//   / | and | \  or | \  and / |
		// 3---1     3---1   0---2   0---2
		bool isCrissCross = ((vertex0.y < vertex3.y && vertex1.y > vertex2.y) || (vertex0.y > vertex3.y && vertex1.y < vertex2.y));
		if (isCrissCross)
		{
			bool isFirstCrissCross = isFloor ? (vertex0.y < vertex3.y) : (vertex0.y > vertex3.y);
			if (isFloor ? (vertex0.y > vertex3.y) : (vertex0.y < vertex3.y))
			{
				isFloor ?
					isFirstCrissCross ? desc.InsertTriangle(vertex0 + offset, vertex1 + offset, vertex2 + offset) : desc.InsertTriangle(vertex0 + offset, vertex2 + offset, vertex3 + offset) :
					isFirstCrissCross ? desc.InsertTriangle(vertex2 + offset, vertex1 + offset, vertex0 + offset) : desc.InsertTriangle(vertex3 + offset, vertex2 + offset, vertex0 + offset);
			}
			if (isFloor ? (vertex1.y > vertex2.y) : (vertex1.y < vertex2.y))
			{
				isFloor ?
					isFirstCrissCross ? desc.InsertTriangle(vertex1 + offset, vertex2 + offset, vertex3 + offset) : desc.InsertTriangle(vertex0 + offset, vertex1 + offset, vertex3 + offset) :
					isFirstCrissCross ? desc.InsertTriangle(vertex3 + offset, vertex2 + offset, vertex1 + offset) : desc.InsertTriangle(vertex3 + offset, vertex1 + offset, vertex0 + offset);
			}
		}
		//     2     3---2
		//   / | and | /
		// 0---1     0
		else
		{
			if (vertex1.y != vertex2.y)
				isFloor ? desc.InsertTriangle(vertex0 + offset, vertex1 + offset, vertex2 + offset) : desc.InsertTriangle(vertex2 + offset, vertex1 + offset, vertex0 + offset);
			if (vertex0.y != vertex3.y)
				isFloor ? desc.InsertTriangle(vertex0 + offset, vertex2 + offset, vertex3 + offset) : desc.InsertTriangle(vertex3 + offset, vertex2 + offset, vertex0 + offset);
		}
	};

	// 1) Generate sector vertices.
	auto sectorVerts = generateSectorVertices();

	// 2) Collect collision mesh triangles.
	bool isFloor = true;
	for (int i = 0; i < SECTOR_SURFACE_COUNT; i++)
	{
		const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;
		const auto& surfVerts = isFloor ? sectorVerts.Floor : sectorVerts.Ceil;

		bool isSurfTri0Portal = (surface.Triangles[0].PortalRoomNumber != NO_VALUE);
		bool isSurfTri1Portal = (surface.Triangles[1].PortalRoomNumber != NO_VALUE);

		// 2.1) Collect surface triangles.
		if (!surfVerts.Tri0.IsWall && !isSurfTri0Portal)
		{
			isFloor ?
				desc.InsertTriangle(surfVerts.Tri0.Vertex2 + offset, surfVerts.Tri0.Vertex1 + offset, surfVerts.Tri0.Vertex0 + offset) :
				desc.InsertTriangle(surfVerts.Tri0.Vertex0 + offset, surfVerts.Tri0.Vertex1 + offset, surfVerts.Tri0.Vertex2 + offset);
		}
		if (!surfVerts.Tri1.IsWall && !isSurfTri1Portal)
		{
			isFloor ?
				desc.InsertTriangle(surfVerts.Tri1.Vertex2 + offset, surfVerts.Tri1.Vertex1 + offset, surfVerts.Tri1.Vertex0 + offset) :
				desc.InsertTriangle(surfVerts.Tri1.Vertex0 + offset, surfVerts.Tri1.Vertex1 + offset, surfVerts.Tri1.Vertex2 + offset);
		}

		// 2.2) Collect diagonal wall triangles.
		if (surfVerts.IsSplit && !(surfVerts.Tri0.IsWall && surfVerts.Tri1.IsWall))
		{
			// Full wall.
			if (isFloor && (surfVerts.Tri0.IsWall || surfVerts.Tri1.IsWall))
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex0 : sectorVerts.Floor.Tri0.Vertex2) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex1 : sectorVerts.Floor.Tri0.Vertex2);
				const auto& vertex1 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex1 : sectorVerts.Floor.Tri0.Vertex0) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex2 : sectorVerts.Floor.Tri0.Vertex0);
				const auto& vertex2 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex1 : sectorVerts.Ceil.Tri0.Vertex0) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex2 : sectorVerts.Ceil.Tri0.Vertex0);
				const auto& vertex3 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex0 : sectorVerts.Ceil.Tri0.Vertex2) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex1 : sectorVerts.Ceil.Tri0.Vertex2);

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!(surfVerts.Tri0.IsWall || surfVerts.Tri1.IsWall) && !(isSurfTri0Portal && isSurfTri1Portal))
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex0 : surfVerts.Tri1.Vertex1;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex1 : surfVerts.Tri1.Vertex2;
				const auto& vertex2 = surfVerts.Tri0.Vertex2;
				const auto& vertex3 = surfVerts.Tri0.Vertex0;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.3) Collect north cardinal wall triangles.
		bool isTriWallNorth = surfVerts.Tri0.IsWall;
		if (!isTriWallNorth || !surfVerts.NorthNeighbor.IsWall)
		{
			// Full wall.
			if (isFloor && (!isTriWallNorth && surfVerts.NorthNeighbor.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex1 : sectorVerts.Floor.Tri0.Vertex0;
				const auto& vertex1 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex2 : sectorVerts.Floor.Tri0.Vertex1;
				const auto& vertex2 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex2 : sectorVerts.Ceil.Tri0.Vertex1;
				const auto& vertex3 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex1 : sectorVerts.Ceil.Tri0.Vertex0;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isTriWallNorth && !surfVerts.NorthNeighbor.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex1 : surfVerts.Tri0.Vertex0;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex2 : surfVerts.Tri0.Vertex1;
				const auto& vertex2 = surfVerts.NorthNeighbor.Vertex1;
				const auto& vertex3 = surfVerts.NorthNeighbor.Vertex0;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.4) Collect south cardinal wall triangles.
		bool isTriWallSouth = surfVerts.Tri1.IsWall;
		if (!isTriWallSouth || !surfVerts.SouthNeighbor.IsWall)
		{
			// Full wall.
			if (isFloor && (!isTriWallSouth && surfVerts.SouthNeighbor.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.Tri1.Vertex2;
				const auto& vertex1 = sectorVerts.Floor.Tri1.Vertex0;
				const auto& vertex2 = sectorVerts.Ceil.Tri1.Vertex0;
				const auto& vertex3 = sectorVerts.Ceil.Tri1.Vertex2;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isTriWallSouth && !surfVerts.SouthNeighbor.IsWall)
			{
				const auto& vertex0 = surfVerts.Tri1.Vertex2;
				const auto& vertex1 = surfVerts.Tri1.Vertex0;
				const auto& vertex2 = surfVerts.SouthNeighbor.Vertex0;
				const auto& vertex3 = surfVerts.SouthNeighbor.Vertex1;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.5) Collect west cardinal wall triangles.
		bool isTriWallWest = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.IsWall : surfVerts.Tri1.IsWall;
		if (!isTriWallWest || !surfVerts.WestNeighbor.IsWall)
		{
			// Full wall.
			if (isFloor && (!isTriWallWest && surfVerts.WestNeighbor.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex0 : sectorVerts.Floor.Tri1.Vertex0;
				const auto& vertex1 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex1 : sectorVerts.Floor.Tri1.Vertex1;
				const auto& vertex2 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex1 : sectorVerts.Ceil.Tri1.Vertex1;
				const auto& vertex3 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex0 : sectorVerts.Ceil.Tri1.Vertex0;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isTriWallWest && !surfVerts.WestNeighbor.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex0 : surfVerts.Tri1.Vertex0;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex1 : surfVerts.Tri1.Vertex1;
				const auto& vertex2 = surfVerts.WestNeighbor.Vertex1;
				const auto& vertex3 = surfVerts.WestNeighbor.Vertex0;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.6) Collect east cardinal wall triangles.
		bool isTriWallEast = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.IsWall : surfVerts.Tri0.IsWall;
		if (!isTriWallEast || !surfVerts.EastNeighbor.IsWall)
		{
			// Full wall.
			if (isFloor && (!isTriWallEast && surfVerts.EastNeighbor.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri1.Vertex1 : sectorVerts.Floor.Tri0.Vertex1;
				const auto& vertex1 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri1.Vertex2 : sectorVerts.Floor.Tri0.Vertex2;
				const auto& vertex2 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri1.Vertex2 : sectorVerts.Ceil.Tri0.Vertex2;
				const auto& vertex3 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri1.Vertex1 : sectorVerts.Ceil.Tri0.Vertex1;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isTriWallEast && !surfVerts.EastNeighbor.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex1 : surfVerts.Tri0.Vertex1;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex2 : surfVerts.Tri0.Vertex2;
				const auto& vertex2 = surfVerts.EastNeighbor.Vertex1;
				const auto& vertex3 = surfVerts.EastNeighbor.Vertex0;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		isFloor = !isFloor;
	}
}

std::vector<int> RoomObjectHandler::GetIds() const
{
	return _tree.GetBoundedObjectIds();
}

std::vector<int> RoomObjectHandler::GetBoundedIds(const Ray& ray, float dist) const
{
	return _tree.GetBoundedObjectIds(ray, dist);
}

std::vector<int> RoomObjectHandler::GetBoundedIds(const BoundingSphere& sphere) const
{
	return _tree.GetBoundedObjectIds(sphere);
}

void RoomObjectHandler::Insert(int id, const BoundingBox& aabb)
{
	_tree.Insert(id, aabb, AABB_BOUNDARY);
}

void RoomObjectHandler::Move(int id, const BoundingBox& aabb)
{
	_tree.Move(id, aabb, AABB_BOUNDARY);
}

void RoomObjectHandler::Remove(int id)
{
	_tree.Remove(id);
}

static void AddRoomFlipItems(const RoomData& room)
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

static void RemoveRoomFlipItems(const RoomData& room)
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

			auto& room = GetRoom(item.RoomNumber);
			room.Bridges.Remove(item.Index);
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
		auto& room = GetRoom(roomNumber);

		// Handle flipmap.
		if (room.flippedRoom >= 0 && room.flipNumber == group)
		{
			auto& flippedRoom = GetRoom(room.flippedRoom);

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
			for (auto& sector : room.Sectors)
				sector.RoomNumber = roomNumber;

			// Update flipped room sectors.
			for (auto& sector : flippedRoom.Sectors)
				sector.RoomNumber = room.flippedRoom;

			// Regenerate neighbor room collision meshes.
			for (int neightborRoomNumber : room.NeighborRoomNumbers)
			{
				auto& neighborRoom = GetRoom(neightborRoomNumber);
				neighborRoom.GenerateCollisionMesh();
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
	int itemNumber = GetRoom(roomNumber).itemNumber;
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
		const auto& room = GetRoom(roomNumber);

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
	const auto& room = GetRoom(roomNumber);

	if (pos.z >= (room.Position.z + BLOCK(1)) && pos.z <= (room.Position.z + BLOCK(room.ZSize - 1)) &&
		pos.y <= room.BottomHeight && pos.y > room.TopHeight &&
		pos.x >= (room.Position.x + BLOCK(1)) && pos.x <= (room.Position.x + BLOCK(room.XSize - 1)))
	{
		return true;
	}

	return false;
}

int FindRoomNumber(const Vector3i& pos, int startRoomNumber)
{
	if (startRoomNumber != NO_VALUE && startRoomNumber < g_Level.Rooms.size())
	{
		const auto& room = GetRoom(startRoomNumber);
		for (int neighborRoomNumber : room.NeighborRoomNumbers)
		{
			const auto& neighborRoom = GetRoom(neighborRoomNumber);
			if (neighborRoomNumber != startRoomNumber && neighborRoom.Active() &&
				IsPointInRoom(pos, neighborRoomNumber))
			{
				return neighborRoomNumber;
			}
		}
	}

	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		if (IsPointInRoom(pos, roomNumber) && GetRoom(roomNumber).Active())
			return roomNumber;
	}

	return (startRoomNumber != NO_VALUE) ? startRoomNumber : 0;
}

Vector3i GetRoomCenter(int roomNumber)
{
	const auto& room = GetRoom(roomNumber);

	int halfLength = BLOCK(room.XSize) / 2;
	int halfDepth = BLOCK(room.ZSize) / 2;
	int halfHeight = (room.TopHeight - room.BottomHeight) / 2;

	// Calculate and return center.
	return Vector3i(
		room.Position.x + halfLength,
		room.BottomHeight + halfHeight,
		room.Position.z + halfDepth);
}

static std::set<int> GetNeighborRoomNumbers(int roomNumber, unsigned int searchDepth, std::set<int>& visitedRoomNumbers = std::set<int>{})
{
	auto neighborRoomNumbers = std::set<int>{};

	// Search depth limit reached; return early.
	if (searchDepth == 0)
		return neighborRoomNumbers;

	// Invalid room; return early.
	if (g_Level.Rooms.size() <= roomNumber)
		return neighborRoomNumbers;

	// Collect current room number as neighbor of itself.
	visitedRoomNumbers.insert(roomNumber);

	// Recursively collect neighbors of current neighbor.
	const auto& room = GetRoom(roomNumber);
	if (room.Portals.empty())
	{
		neighborRoomNumbers.insert(roomNumber);
	}
	else
	{
		for (int i = 0; i < room.Portals.size(); i++)
		{
			int neighborRoomNumber = room.Portals[i].RoomNumber;
			neighborRoomNumbers.insert(neighborRoomNumber);

			auto nextNeighborRoomNumbers = GetNeighborRoomNumbers(neighborRoomNumber, searchDepth - 1, visitedRoomNumbers);
			neighborRoomNumbers.insert(nextNeighborRoomNumbers.begin(), nextNeighborRoomNumbers.end());
		}
	}

	return neighborRoomNumbers;
}

void InitializeNeighborRoomList()
{
	constexpr auto SEARCH_DEPTH = 2;

	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = GetRoom(roomNumber);
		room.NeighborRoomNumbers = GetNeighborRoomNumbers(roomNumber, SEARCH_DEPTH);
	}

	// Add flipped variations of itself.
	for (int roomNumber = 0; roomNumber < g_Level.Rooms.size(); roomNumber++)
	{
		auto& room = GetRoom(roomNumber);
		if (room.flippedRoom == NO_VALUE)
			continue;

		if (!Contains(room.NeighborRoomNumbers, room.flippedRoom))
			room.NeighborRoomNumbers.insert(room.flippedRoom);

		auto& flippedRoom = GetRoom(room.flippedRoom);
		if (!Contains(flippedRoom.NeighborRoomNumbers, roomNumber))
			flippedRoom.NeighborRoomNumbers.insert(roomNumber);
	}
}

namespace TEN::Collision::Room
{
	RoomData& GetRoom(int roomNumber)
	{
		if (roomNumber < 0 || roomNumber >= g_Level.Rooms.size())
		{
			TENLog("Attempted to get invalid room " + std::to_string(roomNumber) + ". Returning room 0.", LogLevel::Warning);
			return g_Level.Rooms.front();
		}

		return g_Level.Rooms[roomNumber];
	}

	// TODO: Can use floordata's GetRoomGridCoord()?
	FloorInfo* GetSector(RoomData* room, int x, int z)
	{
		int sectorX = std::clamp(x / BLOCK(1), 0, room->XSize - 1);
		int sectorZ = std::clamp(z / BLOCK(1), 0, room->ZSize - 1);

		int sectorID = sectorZ + (sectorX * room->ZSize);
		if (sectorID > room->Sectors.size())
			return nullptr;

		return &room->Sectors[sectorID];
	}
}
