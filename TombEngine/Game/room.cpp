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

			// Get previous X sector.
			const auto* prevSectorX = &Sectors[((x - 1) * ZSize) + z];
			if (prevSectorX->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomX = GetRoom(prevSectorX->SidePortalRoomNumber);
				auto prevRoomGridCoordX = GetRoomGridCoord(prevSectorX->SidePortalRoomNumber, prevSectorX->Position.x, prevSectorX->Position.y);

				prevSectorX = &prevRoomX.Sectors[(prevRoomGridCoordX.x * prevRoomX.ZSize) + prevRoomGridCoordX.y];
			}

			// Get next X sector.
			const auto* nextSectorX = &Sectors[((x + 1) * ZSize) + z];
			if (nextSectorX->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& nextRoomX = GetRoom(nextSectorX->SidePortalRoomNumber);
				auto nextRoomGridCoordX = GetRoomGridCoord(nextSectorX->SidePortalRoomNumber, nextSectorX->Position.x, nextSectorX->Position.y);

				nextSectorX = &nextRoomX.Sectors[(nextRoomGridCoordX.x * nextRoomX.ZSize) + nextRoomGridCoordX.y];
			}

			// Get previous Z sector.
			const auto* prevSectorZ = &Sectors[(x * ZSize) + (z - 1)];
			if (prevSectorZ->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomZ = GetRoom(prevSectorZ->SidePortalRoomNumber);
				auto prevRoomGridCoordZ = GetRoomGridCoord(prevSectorZ->SidePortalRoomNumber, prevSectorZ->Position.x, prevSectorZ->Position.y);

				prevSectorZ = &prevRoomZ.Sectors[(prevRoomGridCoordZ.x * prevRoomZ.ZSize) + prevRoomGridCoordZ.y];
			}

			// Get next Z sector.
			const auto* nextSectorZ = &Sectors[(x * ZSize) + (z + 1)];
			if (nextSectorZ->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& nextRoomZ = GetRoom(nextSectorZ->SidePortalRoomNumber);
				auto nextRoomGridCoordZ = GetRoomGridCoord(nextSectorZ->SidePortalRoomNumber, nextSectorZ->Position.x, nextSectorZ->Position.y);

				nextSectorZ = &nextRoomZ.Sectors[(nextRoomGridCoordZ.x * nextRoomZ.ZSize) + nextRoomGridCoordZ.y];
			}

			CollectSectorCollisionMeshTriangles(desc, sector, *prevSectorX, *nextSectorX, *prevSectorZ, *nextSectorZ);
		}
	}

	// Create collision mesh.
	CollisionMesh = TEN::Physics::CollisionMesh(Vector3::Zero, Quaternion::Identity, desc);
}

void RoomData::CollectSectorCollisionMeshTriangles(CollisionMeshDesc& desc,
												   const FloorInfo& sector,
												   const FloorInfo& prevSectorX, const FloorInfo& nextSectorX,
												   const FloorInfo& prevSectorZ, const FloorInfo& nextSectorZ)
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

			NeighborData PrevNeighborX = {};
			NeighborData NextNeighborX = {};
			NeighborData PrevNeighborZ = {};
			NeighborData NextNeighborZ = {};
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

			// 2.3) Set previous X axis neighbor data.
			//      +---1 +---+
			// Prev |   | |   | Current
			//      +---0 +---+
			const auto& prevSurfX = isFloor ? prevSectorX.FloorSurface : prevSectorX.CeilingSurface;
			bool usePrevSurfXTri0 = !(!prevSectorX.IsSurfaceSplit(isFloor) || prevSurfX.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.PrevNeighborX.IsWall = prevSectorX.IsWall(usePrevSurfXTri0 ? 0 : 1);
			surfVertices.PrevNeighborX.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevSurfXTri0 ? 0 : 1, isFloor), corner0.y);
			surfVertices.PrevNeighborX.Vertex1 = Vector3(corner1.x, getSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevSurfXTri0 ? 0 : 1, isFloor), corner1.y);

			// 2.4) Set next X axis neighbor data.
			//         +---+ 1---+
			// Current |   | |   | Next
			//         +---+ 0---+
			const auto& nextSurfX = isFloor ? nextSectorX.FloorSurface : nextSectorX.CeilingSurface;
			bool useNextSurfXTri0 = (!nextSectorX.IsSurfaceSplit(isFloor) || nextSurfX.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.NextNeighborX.IsWall = nextSectorX.IsWall(useNextSurfXTri0 ? 0 : 1);
			surfVertices.NextNeighborX.Vertex0 = Vector3(corner2.x, getSurfaceTriangleVertexY(nextSectorX, REL_CORNER_1.x, REL_CORNER_1.y, useNextSurfXTri0 ? 0 : 1, isFloor), corner2.y);
			surfVertices.NextNeighborX.Vertex1 = Vector3(corner3.x, getSurfaceTriangleVertexY(nextSectorX, REL_CORNER_0.x, REL_CORNER_0.y, useNextSurfXTri0 ? 0 : 1, isFloor), corner3.y);

			// 2.5) Set previous Z axis neighbor data.
			// +---+
			// |   | Current
			// +---+
			// 0---1
			// |   | Prev
			// +---+
			const auto& prevSurfZ = isFloor ? prevSectorZ.FloorSurface : prevSectorZ.CeilingSurface;
			surfVertices.PrevNeighborZ.IsWall = prevSectorZ.IsWall(0);
			surfVertices.PrevNeighborZ.Vertex0 = Vector3(corner0.x, getSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner0.y);
			surfVertices.PrevNeighborZ.Vertex1 = Vector3(corner3.x, getSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner3.y);

			// 2.6) Set next Z axis neighbor data.
			// +---+
			// |   | Next
			// 0---1
			// +---+
			// |   | Current
			// +---+
			const auto& nextSurfZ = isFloor ? nextSectorZ.FloorSurface : nextSectorZ.CeilingSurface;
			surfVertices.NextNeighborZ.IsWall = nextSectorZ.IsWall(1);
			surfVertices.NextNeighborZ.Vertex0 = Vector3(corner1.x, getSurfaceTriangleVertexY(nextSectorZ, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner1.y);
			surfVertices.NextNeighborZ.Vertex1 = Vector3(corner2.x, getSurfaceTriangleVertexY(nextSectorZ, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner2.y);

			isFloor = !isFloor;
		}

		return sectorVerts;
	};

	// Vertex input:
	// 3---2
	// |   |
	// 0---1
	auto insertWallTriangles = [&](bool isFloor, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3)
	{
		//     2     0       3            1
		//   / | and | \  or | \  and  / |
		// 3---1     3---1   0---2    0---2
		bool isCrissCross = ((vertex0.y < vertex3.y && vertex1.y > vertex2.y) || (vertex0.y > vertex3.y && vertex1.y < vertex2.y));
		if (isCrissCross)
		{
			bool isFirstCrissCross = isFloor ? (vertex0.y < vertex3.y) : (vertex0.y > vertex3.y);
			if (isFloor ? (vertex0.y > vertex3.y) : (vertex0.y < vertex3.y))
			{
				isFloor ?
					isFirstCrissCross ? desc.InsertTriangle(vertex0, vertex1, vertex2) : desc.InsertTriangle(vertex0, vertex2, vertex3) :
					isFirstCrissCross ? desc.InsertTriangle(vertex2, vertex1, vertex0) : desc.InsertTriangle(vertex3, vertex2, vertex0);
			}
			if (isFloor ? (vertex1.y > vertex2.y) : (vertex1.y < vertex2.y))
			{
				isFloor ?
					isFirstCrissCross ? desc.InsertTriangle(vertex1, vertex2, vertex3) : desc.InsertTriangle(vertex0, vertex1, vertex3) :
					isFirstCrissCross ? desc.InsertTriangle(vertex3, vertex2, vertex1) : desc.InsertTriangle(vertex3, vertex1, vertex0);
			}
		}
		//     2     3---2
		//   / | and | /
		// 0---1     0
		else
		{
			if (vertex1.y != vertex2.y)
				isFloor ? desc.InsertTriangle(vertex0, vertex1, vertex2) : desc.InsertTriangle(vertex2, vertex1, vertex0);
			if (vertex0.y != vertex3.y)
				isFloor ? desc.InsertTriangle(vertex0, vertex2, vertex3) : desc.InsertTriangle(vertex3, vertex2, vertex0);
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
				desc.InsertTriangle(surfVerts.Tri0.Vertex2, surfVerts.Tri0.Vertex1, surfVerts.Tri0.Vertex0) :
				desc.InsertTriangle(surfVerts.Tri0.Vertex0, surfVerts.Tri0.Vertex1, surfVerts.Tri0.Vertex2);
		}
		if (!surfVerts.Tri1.IsWall && !isSurfTri1Portal)
		{
			isFloor ?
				desc.InsertTriangle(surfVerts.Tri1.Vertex2, surfVerts.Tri1.Vertex1, surfVerts.Tri1.Vertex0) :
				desc.InsertTriangle(surfVerts.Tri1.Vertex0, surfVerts.Tri1.Vertex1, surfVerts.Tri1.Vertex2);
		}

		// 2.2) Collect diagonal wall triangles.
		if (surfVerts.IsSplit && !(surfVerts.Tri0.IsWall && surfVerts.Tri1.IsWall))
		{
			// Full wall.
			if (isFloor && (surfVerts.Tri0.IsWall || surfVerts.Tri1.IsWall))
			{
				// TODO: Fix some walls.
				const auto& vertex0 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex0 : sectorVerts.Floor.Tri0.Vertex0) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex1 : sectorVerts.Floor.Tri0.Vertex0);
				const auto& vertex1 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex1 : sectorVerts.Floor.Tri0.Vertex2) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Floor.Tri1.Vertex2 : sectorVerts.Floor.Tri0.Vertex2);
				const auto& vertex2 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex1 : sectorVerts.Ceil.Tri0.Vertex2) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex2 : sectorVerts.Ceil.Tri0.Vertex2);
				const auto& vertex3 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex0 : sectorVerts.Ceil.Tri0.Vertex0) :
					(surfVerts.Tri0.IsWall ? sectorVerts.Ceil.Tri1.Vertex1 : sectorVerts.Ceil.Tri0.Vertex0);

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!(surfVerts.Tri0.IsWall || surfVerts.Tri1.IsWall) && !(isSurfTri0Portal && isSurfTri1Portal))
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex0 : surfVerts.Tri1.Vertex1;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex1 : surfVerts.Tri1.Vertex2;
				const auto& vertex2 = surfVerts.Tri0.Vertex2;
				const auto& vertex3 = surfVerts.Tri0.Vertex0;

				// TODO: Check various diagonal step arrangements.
				//if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
				{
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
				}
				//else
				{
				//	insertStepWallTriangles(isFloor, vertex1, vertex0, vertex3, vertex2);
				}
			}
		}

		// 2.3) Collect previous X axis cardinal wall triangles.
		bool isPrevXTriWall = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.IsWall : surfVerts.Tri1.IsWall;
		if (!isPrevXTriWall || !surfVerts.PrevNeighborX.IsWall)
		{
			// Full wall.
			if (isFloor && (!isPrevXTriWall && surfVerts.PrevNeighborX.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex0 : sectorVerts.Floor.Tri1.Vertex0;
				const auto& vertex1 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex1 : sectorVerts.Floor.Tri1.Vertex1;
				const auto& vertex2 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex1 : sectorVerts.Ceil.Tri1.Vertex1;
				const auto& vertex3 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex0 : sectorVerts.Ceil.Tri1.Vertex0;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isPrevXTriWall && !surfVerts.PrevNeighborX.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex0 : surfVerts.Tri1.Vertex0;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex1 : surfVerts.Tri1.Vertex1;
				const auto& vertex2 = surfVerts.PrevNeighborX.Vertex1;
				const auto& vertex3 = surfVerts.PrevNeighborX.Vertex0;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.4) Collect next X axis cardinal wall triangles.
		bool isNextXTriWall = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.IsWall : surfVerts.Tri0.IsWall;
		if (!isNextXTriWall || !surfVerts.NextNeighborX.IsWall)
		{
			// Full wall.
			if (isFloor && (!isNextXTriWall && surfVerts.NextNeighborX.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri1.Vertex1 : sectorVerts.Floor.Tri0.Vertex1;
				const auto& vertex1 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri1.Vertex2 : sectorVerts.Floor.Tri0.Vertex2;
				const auto& vertex2 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri1.Vertex2 : sectorVerts.Ceil.Tri0.Vertex2;
				const auto& vertex3 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri1.Vertex1 : sectorVerts.Ceil.Tri0.Vertex1;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isNextXTriWall && !surfVerts.NextNeighborX.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex1 : surfVerts.Tri0.Vertex1;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri1.Vertex2 : surfVerts.Tri0.Vertex2;
				const auto& vertex2 = surfVerts.NextNeighborX.Vertex1;
				const auto& vertex3 = surfVerts.NextNeighborX.Vertex0;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.5) Collect previous Z axis cardinal wall triangles.
		bool isPrevZTriWall = surfVerts.Tri1.IsWall;
		if (!isPrevZTriWall || !surfVerts.PrevNeighborZ.IsWall)
		{
			// Full wall.
			if (isFloor && (!isPrevZTriWall && surfVerts.PrevNeighborZ.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.Tri1.Vertex2;
				const auto& vertex1 = sectorVerts.Floor.Tri1.Vertex0;
				const auto& vertex2 = sectorVerts.Ceil.Tri1.Vertex0;
				const auto& vertex3 = sectorVerts.Ceil.Tri1.Vertex2;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isPrevZTriWall && !surfVerts.PrevNeighborZ.IsWall)
			{
				const auto& vertex0 = surfVerts.Tri1.Vertex2;
				const auto& vertex1 = surfVerts.Tri1.Vertex0;
				const auto& vertex2 = surfVerts.PrevNeighborZ.Vertex0;
				const auto& vertex3 = surfVerts.PrevNeighborZ.Vertex1;

				if (isFloor ? !(vertex0.y <= vertex3.y && vertex1.y <= vertex2.y) : !(vertex0.y >= vertex3.y && vertex1.y >= vertex2.y))
					insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
		}

		// 2.6) Collect next Z axis cardinal wall triangles.
		bool isNextZTriWall = surfVerts.Tri0.IsWall;
		if (!isNextZTriWall || !surfVerts.NextNeighborZ.IsWall)
		{
			// Full wall.
			if (isFloor && (!isNextZTriWall && surfVerts.NextNeighborZ.IsWall))
			{
				const auto& vertex0 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex1 : sectorVerts.Floor.Tri0.Vertex0;
				const auto& vertex1 = sectorVerts.Floor.IsSplitAngle0 ? sectorVerts.Floor.Tri0.Vertex2 : sectorVerts.Floor.Tri0.Vertex1;
				const auto& vertex2 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex2 : sectorVerts.Ceil.Tri0.Vertex1;
				const auto& vertex3 = sectorVerts.Ceil.IsSplitAngle0 ? sectorVerts.Ceil.Tri0.Vertex1 : sectorVerts.Ceil.Tri0.Vertex0;

				insertWallTriangles(isFloor, vertex0, vertex1, vertex2, vertex3);
			}
			// Step wall.
			else if (!isNextZTriWall && !surfVerts.NextNeighborZ.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex1 : surfVerts.Tri0.Vertex0;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex2 : surfVerts.Tri0.Vertex1;
				const auto& vertex2 = surfVerts.NextNeighborZ.Vertex1;
				const auto& vertex3 = surfVerts.NextNeighborZ.Vertex0;

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
