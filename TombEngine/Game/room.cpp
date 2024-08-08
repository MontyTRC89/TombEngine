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
	CollisionMesh = {};

	// Run through room sectors (ignoring border).
	for (int x = 1; x < (XSize - 1); x++)
	{
		for (int z = 1; z < (ZSize - 1); z++)
		{
			const auto& sector = Sectors[(x * ZSize) + z];

			// Get previous X sector.
			const auto* prevSectorX = &Sectors[((x - 1) * ZSize) + z];
			if (prevSectorX->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomX = g_Level.Rooms[prevSectorX->SidePortalRoomNumber];
				auto prevRoomGridCoordX = GetRoomGridCoord(prevSectorX->SidePortalRoomNumber, prevSectorX->Position.x, prevSectorX->Position.y);

				prevSectorX = &prevRoomX.Sectors[(prevRoomGridCoordX.x * prevRoomX.ZSize) + prevRoomGridCoordX.y];
			}

			// Get next X sector.
			const auto* nextSectorX = &Sectors[((x + 1) * ZSize) + z];
			if (nextSectorX->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& nextRoomX = g_Level.Rooms[nextSectorX->SidePortalRoomNumber];
				auto nextRoomGridCoordX = GetRoomGridCoord(nextSectorX->SidePortalRoomNumber, nextSectorX->Position.x, nextSectorX->Position.y);

				nextSectorX = &nextRoomX.Sectors[(nextRoomGridCoordX.x * nextRoomX.ZSize) + nextRoomGridCoordX.y];
			}

			// Get previous Z sector.
			const auto* prevSectorZ = &Sectors[(x * ZSize) + (z - 1)];
			if (prevSectorZ->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomZ = g_Level.Rooms[prevSectorZ->SidePortalRoomNumber];
				auto prevRoomGridCoordZ = GetRoomGridCoord(prevSectorZ->SidePortalRoomNumber, prevSectorZ->Position.x, prevSectorZ->Position.y);

				prevSectorZ = &prevRoomZ.Sectors[(prevRoomGridCoordZ.x * prevRoomZ.ZSize) + prevRoomGridCoordZ.y];
			}

			// Get next Z sector.
			const auto* nextSectorZ = &Sectors[(x * ZSize) + (z + 1)];
			if (nextSectorZ->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& nextRoomZ = g_Level.Rooms[nextSectorZ->SidePortalRoomNumber];
				auto nextRoomGridCoordZ = GetRoomGridCoord(nextSectorZ->SidePortalRoomNumber, nextSectorZ->Position.x, nextSectorZ->Position.y);

				nextSectorZ = &nextRoomZ.Sectors[(nextRoomGridCoordZ.x * nextRoomZ.ZSize) + nextRoomGridCoordZ.y];
			}

			// Collect collision mesh triangles for sector.
			CollectSectorCollisionMeshTriangles(sector, *prevSectorX, *nextSectorX , *prevSectorZ, *nextSectorZ);
		}
	}

	// Initialize collision mesh.
	CollisionMesh.Initialize();
}

void RoomData::CollectSectorCollisionMeshTriangles(const FloorInfo& sector,
												   const FloorInfo& prevSectorX, const FloorInfo& nextSectorX,
												   const FloorInfo& prevSectorZ, const FloorInfo& nextSectorZ)
{
	constexpr auto NORTH_WALL_NORMAL	  = Vector3(0.0f, 0.0f, 1.0f);
	constexpr auto SOUTH_WALL_NORMAL	  = Vector3(0.0f, 0.0f, -1.0f);
	constexpr auto EAST_WALL_NORMAL		  = Vector3(1.0f, 0.0f, 0.0f);
	constexpr auto WEST_WALL_NORMAL		  = Vector3(-1.0f, 0.0f, 0.0f);
	constexpr auto NORTH_EAST_WALL_NORMAL = Vector3(SQRT_2 / 2, 0.0f, SQRT_2 / 2);
	constexpr auto NORTH_WEST_WALL_NORMAL = Vector3(-SQRT_2 / 2, 0.0f, SQRT_2 / 2);
	constexpr auto SOUTH_EAST_WALL_NORMAL = Vector3(SQRT_2 / 2, 0.0f, -SQRT_2 / 2);
	constexpr auto SOUTH_WEST_WALL_NORMAL = Vector3(-SQRT_2 / 2, 0.0f, -SQRT_2 / 2);

	struct VertexData
	{
		struct SurfaceData
		{
			struct TriangleData
			{
				bool IsWall = false;

				Vector3 Vertex0 = Vector3::Zero;
				Vector3 Vertex1 = Vector3::Zero;
				Vector3 Vertex2 = Vector3::Zero;
				Vector3 Normal	= Vector3::Zero;
			};

			struct NeighborData
			{
				bool UseTri0 = false;
				bool IsWall	 = false;

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
	
	auto GetSurfaceTriangleVertexY = [](const FloorInfo& sector, int relX, int relZ, int triID, bool isFloor)
	{
		constexpr auto AXIS_OFFSET = -BLOCK(0.5f);
		constexpr auto HEIGHT_STEP = BLOCK(1 / 32.0f);

		const auto& tri = isFloor ? sector.FloorSurface.Triangles[triID] : sector.CeilingSurface.Triangles[triID];

		relX += AXIS_OFFSET;
		relZ += AXIS_OFFSET;

		auto normal = tri.Plane.Normal();
		float relPlaneHeight = -((normal.x * relX) + (normal.z * relZ)) / normal.y;
		return (int)RoundToStep(tri.Plane.D() + relPlaneHeight, HEIGHT_STEP);
	};

	auto GetRawSurfaceTriangleNormal = [](const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
	{
		auto edge0 = vertex1 - vertex0;
		auto edge1 = vertex2 - vertex0;

		auto normal = edge0.Cross(edge1);
		normal.Normalize();
		return normal;
	};

	auto GetVertices = [&]()
	{
		constexpr auto REL_CORNER_0 = Vector2i(0, 0);
		constexpr auto REL_CORNER_1 = Vector2i(0, BLOCK(1));
		constexpr auto REL_CORNER_2 = Vector2i(BLOCK(1), BLOCK(1));
		constexpr auto REL_CORNER_3 = Vector2i(BLOCK(1), 0);

		auto vertices = VertexData{};

		// 1) Calculate 2D corner positions.
		auto corner0 = sector.Position + REL_CORNER_0;
		auto corner1 = sector.Position + REL_CORNER_1;
		auto corner2 = sector.Position + REL_CORNER_2;
		auto corner3 = sector.Position + REL_CORNER_3;

		// 2) Set vertex data for floor and ceiling.
		bool isFloor = true;
		for (int i = 0; i < 2; i++)
		{
			const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;
			auto& surfVertices = isFloor ? vertices.Floor : vertices.Ceil;
			int sign = isFloor ? -1 : 1;

			// 2.1) Set surface status data.
			surfVertices.IsSplit = sector.IsSurfaceSplit(isFloor);
			surfVertices.IsSplitAngle0 = (!surfVertices.IsSplit || surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.Tri0.IsWall = sector.IsWall(0);
			surfVertices.Tri1.IsWall = sector.IsWall(1);

			// 2.2) Set surface triangle vertex data.
			if (surfVertices.IsSplitAngle0)
			{
				surfVertices.Tri0.Vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor), corner0.y);
				surfVertices.Tri0.Vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
				surfVertices.Tri0.Vertex2 = Vector3(corner2.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
				surfVertices.Tri0.Normal = GetRawSurfaceTriangleNormal(surfVertices.Tri0.Vertex0, surfVertices.Tri0.Vertex1, surfVertices.Tri0.Vertex2) * sign;
			
				surfVertices.Tri1.Vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
				surfVertices.Tri1.Vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 1, isFloor), corner2.y);
				surfVertices.Tri1.Vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
				surfVertices.Tri1.Normal = GetRawSurfaceTriangleNormal(surfVertices.Tri1.Vertex0, surfVertices.Tri1.Vertex1, surfVertices.Tri1.Vertex2) * sign;
			}
			else
			{
				surfVertices.Tri0.Vertex0 = Vector3(corner1.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
				surfVertices.Tri0.Vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
				surfVertices.Tri0.Vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 0, isFloor), corner3.y);
				surfVertices.Tri0.Normal = GetRawSurfaceTriangleNormal(surfVertices.Tri0.Vertex0, surfVertices.Tri0.Vertex1, surfVertices.Tri0.Vertex2) * sign;
			
				surfVertices.Tri1.Vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
				surfVertices.Tri1.Vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_1.x, REL_CORNER_1.y, 1, isFloor), corner1.y);
				surfVertices.Tri1.Vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexY(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
				surfVertices.Tri1.Normal = GetRawSurfaceTriangleNormal(surfVertices.Tri1.Vertex0, surfVertices.Tri1.Vertex1, surfVertices.Tri1.Vertex2) * sign;
			}

			// 2.3) Set previous neighbor data on X axis.
			const auto& prevSurfaceX = isFloor ? prevSectorX.FloorSurface : prevSectorX.CeilingSurface;
			surfVertices.PrevNeighborX.UseTri0 = !(!prevSectorX.IsSurfaceSplit(isFloor) || prevSurfaceX.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.PrevNeighborX.IsWall = prevSectorX.IsWall(surfVertices.PrevNeighborX.UseTri0 ? 0 : 1);
			surfVertices.PrevNeighborX.Vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, surfVertices.PrevNeighborX.UseTri0 ? 0 : 1, isFloor), corner0.y);
			surfVertices.PrevNeighborX.Vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexY(prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, surfVertices.PrevNeighborX.UseTri0 ? 0 : 1, isFloor), corner1.y);

			// 2.4) Set next neighbor data on X axis.
			const auto& nextSurfaceX = isFloor ? nextSectorX.FloorSurface : nextSectorX.CeilingSurface;
			surfVertices.NextNeighborX.UseTri0 = (!nextSectorX.IsSurfaceSplit(isFloor) || nextSurfaceX.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.NextNeighborX.IsWall = nextSectorX.IsWall(surfVertices.NextNeighborX.UseTri0 ? 0 : 1);
			surfVertices.NextNeighborX.Vertex0 = Vector3(corner3.x, GetSurfaceTriangleVertexY(nextSectorX, REL_CORNER_0.x, REL_CORNER_0.y, surfVertices.NextNeighborX.UseTri0 ? 0 : 1, isFloor), corner3.y);
			surfVertices.NextNeighborX.Vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexY(nextSectorX, REL_CORNER_1.x, REL_CORNER_1.y, surfVertices.NextNeighborX.UseTri0 ? 0 : 1, isFloor), corner2.y);

			// 2.5) Set previous neighbor data on Z axis.
			const auto& prevSurfaceZ = isFloor ? prevSectorZ.FloorSurface : prevSectorZ.CeilingSurface;
			surfVertices.PrevNeighborZ.UseTri0 = (!prevSectorZ.IsSurfaceSplit(isFloor) || prevSurfaceZ.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.PrevNeighborZ.IsWall = prevSectorZ.IsWall(surfVertices.PrevNeighborZ.UseTri0 ? 0 : 1);
			surfVertices.PrevNeighborZ.Vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_0.x, REL_CORNER_0.y, surfVertices.PrevNeighborZ.UseTri0 ? 0 : 1, isFloor), corner0.y);
			surfVertices.PrevNeighborZ.Vertex1 = Vector3(corner3.x, GetSurfaceTriangleVertexY(prevSectorZ, REL_CORNER_3.x, REL_CORNER_3.y, surfVertices.PrevNeighborZ.UseTri0 ? 0 : 1, isFloor), corner3.y);

			// 2.6) Set next neighbor data on Z axis.
			const auto& nextSurfaceZ = isFloor ? nextSectorZ.FloorSurface : nextSectorZ.CeilingSurface;
			surfVertices.NextNeighborZ.UseTri0 = !(!nextSectorZ.IsSurfaceSplit(isFloor) || nextSurfaceZ.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
			surfVertices.NextNeighborZ.IsWall = nextSectorZ.IsWall(surfVertices.NextNeighborZ.UseTri0 ? 0 : 1);
			surfVertices.NextNeighborZ.Vertex0 = Vector3(corner1.x, GetSurfaceTriangleVertexY(nextSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, surfVertices.NextNeighborZ.UseTri0 ? 0 : 1, isFloor), corner1.y);
			surfVertices.NextNeighborZ.Vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexY(nextSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, surfVertices.NextNeighborZ.UseTri0 ? 0 : 1, isFloor), corner2.y);

			isFloor = !isFloor;
		}

		return vertices;
	};

	// 1) Generate surface triangle vertices.
	auto vertices = GetVertices();

	// 2) Collect collision mesh triangles.
	bool isFloor = true;
	for (int i = 0; i < 2; i++)
	{
		const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;
		const auto& surfVerts = isFloor ? vertices.Floor : vertices.Ceil;
		int sign = isFloor ? 1 : -1;

		// Determine surface portal status.
		bool isSurfTri0Portal = (surface.Triangles[0].PortalRoomNumber != NO_VALUE);
		bool isSurfTri1Portal = (surface.Triangles[1].PortalRoomNumber != NO_VALUE);

		// 2.1) Collect surface triangles.
		if (!surfVerts.Tri0.IsWall)
			CollisionMesh.InsertTriangle(surfVerts.Tri0.Vertex0, surfVerts.Tri0.Vertex1, surfVerts.Tri0.Vertex2, surfVerts.Tri0.Normal, surface.Triangles[0].PortalRoomNumber);
		if (!surfVerts.Tri1.IsWall)
			CollisionMesh.InsertTriangle(surfVerts.Tri1.Vertex0, surfVerts.Tri1.Vertex1, surfVerts.Tri1.Vertex2, surfVerts.Tri1.Normal, surface.Triangles[1].PortalRoomNumber);

		// 2.2) Collect diagonal wall triangles.
		if (surfVerts.IsSplit && !(surfVerts.Tri0.IsWall && surfVerts.Tri1.IsWall))
		{
			// Full wall.
			if ((surfVerts.Tri0.IsWall || surfVerts.Tri1.IsWall) && isFloor)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? vertices.Floor.Tri1.Vertex0 : vertices.Floor.Tri0.Vertex0) :
					(surfVerts.Tri0.IsWall ? vertices.Floor.Tri1.Vertex1 : vertices.Floor.Tri0.Vertex0);
				const auto& vertex1 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? vertices.Floor.Tri1.Vertex1 : vertices.Floor.Tri0.Vertex2) :
					(surfVerts.Tri0.IsWall ? vertices.Floor.Tri1.Vertex2 : vertices.Floor.Tri0.Vertex2);
				const auto& vertex2 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? vertices.Ceil.Tri1.Vertex0 : vertices.Ceil.Tri0.Vertex0) :
					(surfVerts.Tri0.IsWall ? vertices.Ceil.Tri1.Vertex1 : vertices.Ceil.Tri0.Vertex0);
				const auto& vertex3 = surfVerts.IsSplitAngle0 ?
					(surfVerts.Tri0.IsWall ? vertices.Ceil.Tri1.Vertex1 : vertices.Ceil.Tri0.Vertex2) :
					(surfVerts.Tri0.IsWall ? vertices.Ceil.Tri1.Vertex2 : vertices.Ceil.Tri0.Vertex2);

				if (vertex0 != vertex2)
				{
					const auto& normal = surfVerts.IsSplitAngle0 ?
						(surfVerts.Tri0.IsWall ? SOUTH_EAST_WALL_NORMAL : NORTH_WEST_WALL_NORMAL) :
						(surfVerts.Tri0.IsWall ? SOUTH_WEST_WALL_NORMAL : NORTH_EAST_WALL_NORMAL);
					CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, normal);
				}
				if (vertex1 != vertex3)
				{
					const auto& normal = surfVerts.IsSplitAngle0 ?
						(surfVerts.Tri0.IsWall ? SOUTH_EAST_WALL_NORMAL : NORTH_WEST_WALL_NORMAL) :
						(surfVerts.Tri0.IsWall ? SOUTH_WEST_WALL_NORMAL : NORTH_EAST_WALL_NORMAL);
					CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, normal);
				}
			}
			// Step wall.
			else if (!(surfVerts.Tri0.IsWall || surfVerts.Tri1.IsWall) && !(isSurfTri0Portal && isSurfTri1Portal))
			{
				// TODO: Check when diagonal criss-cross becomes possible.
				if (surfVerts.IsSplitAngle0)
				{
					bool isSecondCrissCrossCase = (isFloor ? (surfVerts.Tri0.Vertex2.y < surfVerts.Tri0.Vertex1.y) : !(surfVerts.Tri0.Vertex2.y < surfVerts.Tri0.Vertex1.y));
					if (surfVerts.Tri0.Vertex0 != surfVerts.Tri1.Vertex0)
					{
						const auto& normal = ((surfVerts.Tri0.Vertex0.y > surfVerts.Tri1.Vertex0.y) ? NORTH_WEST_WALL_NORMAL : SOUTH_EAST_WALL_NORMAL) * sign;
						isSecondCrissCrossCase ?
							CollisionMesh.InsertTriangle(surfVerts.Tri0.Vertex0, surfVerts.Tri0.Vertex2, surfVerts.Tri1.Vertex1, normal) :
							CollisionMesh.InsertTriangle(surfVerts.Tri0.Vertex0, surfVerts.Tri1.Vertex0, surfVerts.Tri0.Vertex2, normal);
					}
					if (surfVerts.Tri0.Vertex2 != surfVerts.Tri0.Vertex1)
					{
						const auto& normal = ((surfVerts.Tri0.Vertex2.y > surfVerts.Tri1.Vertex1.y) ? NORTH_WEST_WALL_NORMAL : SOUTH_EAST_WALL_NORMAL) * sign;
						isSecondCrissCrossCase ?
							CollisionMesh.InsertTriangle(surfVerts.Tri0.Vertex0, surfVerts.Tri1.Vertex0, surfVerts.Tri1.Vertex1, normal) :
							CollisionMesh.InsertTriangle(surfVerts.Tri1.Vertex0, surfVerts.Tri0.Vertex2, surfVerts.Tri1.Vertex1, normal);
					}
				}
				else
				{
					bool isSecondCrissCrossCase = (isFloor ? (surfVerts.Tri1.Vertex2.y < surfVerts.Tri0.Vertex2.y) : !(surfVerts.Tri1.Vertex2.y <surfVerts.Tri0.Vertex2.y));
					if (surfVerts.Tri0.Vertex0 != surfVerts.Tri1.Vertex1)
					{
						const auto& normal = ((surfVerts.Tri0.Vertex0.y > surfVerts.Tri1.Vertex1.y) ? NORTH_EAST_WALL_NORMAL : SOUTH_WEST_WALL_NORMAL) * sign;
						isSecondCrissCrossCase ?
							CollisionMesh.InsertTriangle(surfVerts.Tri1.Vertex1, surfVerts.Tri1.Vertex2, surfVerts.Tri0.Vertex2, normal) :
							CollisionMesh.InsertTriangle(surfVerts.Tri1.Vertex1, surfVerts.Tri0.Vertex0, surfVerts.Tri1.Vertex2, normal);
					}
					if (surfVerts.Tri0.Vertex2 != surfVerts.Tri1.Vertex2)
					{
						const auto& normal = ((surfVerts.Tri0.Vertex2.y > surfVerts.Tri1.Vertex2.y) ? NORTH_EAST_WALL_NORMAL : SOUTH_WEST_WALL_NORMAL) * sign;
						isSecondCrissCrossCase ?
							CollisionMesh.InsertTriangle(surfVerts.Tri1.Vertex1, surfVerts.Tri0.Vertex0, surfVerts.Tri0.Vertex2, normal) :
							CollisionMesh.InsertTriangle(surfVerts.Tri0.Vertex0, surfVerts.Tri1.Vertex2, surfVerts.Tri0.Vertex2, normal);
					}
				}
			}
		}

		// 2.3) Collect previous cardinal wall triangles on X axis.
		bool isPrevSurfWall = (surfVerts.IsSplitAngle0 ? surfVerts.Tri0.IsWall : surfVerts.Tri1.IsWall);
		if (!isPrevSurfWall || !surfVerts.PrevNeighborX.IsWall)
		{
			// Full wall.
			if (isFloor && (!isPrevSurfWall && surfVerts.PrevNeighborX.IsWall))
			{
				bool useFloorTri0 = surfVerts.IsSplitAngle0;
				bool useCeilTri0 = vertices.Ceil.IsSplitAngle0;

				const auto& vertex0 = useFloorTri0 ? vertices.Floor.Tri0.Vertex0 : vertices.Floor.Tri1.Vertex0;
				const auto& vertex1 = useFloorTri0 ? vertices.Floor.Tri0.Vertex1 : vertices.Floor.Tri1.Vertex1;
				const auto& vertex2 = useCeilTri0 ? vertices.Ceil.Tri0.Vertex0 : vertices.Ceil.Tri1.Vertex0;
				const auto& vertex3 = useCeilTri0 ? vertices.Ceil.Tri0.Vertex1 : vertices.Ceil.Tri1.Vertex1;

				if (vertex0 != vertex2)
					CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL);
				if (vertex1 != vertex3)
					CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL);
			}
			// Step wall.
			else if (!isPrevSurfWall && !surfVerts.PrevNeighborX.IsWall)
			{
				const auto& vertex0 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex0 : surfVerts.Tri1.Vertex0;
				const auto& vertex1 = surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex1 : surfVerts.Tri1.Vertex1;
				const auto& vertex2 = surfVerts.IsSplitAngle0 ? surfVerts.PrevNeighborX.Vertex0 : surfVerts.PrevNeighborX.Vertex0;
				const auto& vertex3 = surfVerts.IsSplitAngle0 ? surfVerts.PrevNeighborX.Vertex1 : surfVerts.PrevNeighborX.Vertex1;

				bool isSecondCrissCrossCase = (isFloor ? (vertex1.y < vertex3.y) : !(vertex1.y < vertex3.y));
				if (isFloor ? (vertex0.y > vertex2.y) : (vertex0.y < vertex2.y))
				{
					isSecondCrissCrossCase ?
						CollisionMesh.InsertTriangle(vertex0, vertex2, vertex3, EAST_WALL_NORMAL) :
						CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL);
				}
				if (isFloor ? (vertex1.y > vertex3.y) : (vertex1.y < vertex3.y))
				{
					isSecondCrissCrossCase ?
						CollisionMesh.InsertTriangle(vertex0, vertex1, vertex3, EAST_WALL_NORMAL) :
						CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL);
				}
			}
		}

		// 2.4) Collect next cardinal wall triangles on X axis.
		bool isNextSurfWall = (surfVerts.IsSplitAngle0 ? surfVerts.Tri1.IsWall : surfVerts.Tri0.IsWall);
		if (!isNextSurfWall || !surfVerts.NextNeighborX.IsWall)
		{
			// Full wall.
			if (isFloor && (!isNextSurfWall && surfVerts.NextNeighborX.IsWall))
			{
				bool useFloorTri0 = !surfVerts.IsSplitAngle0;
				bool useCeilTri0 = !vertices.Ceil.IsSplitAngle0;

				const auto& vertex0 = useFloorTri0 ? vertices.Floor.Tri0.Vertex1 : vertices.Floor.Tri1.Vertex1;
				const auto& vertex1 = useFloorTri0 ? vertices.Floor.Tri0.Vertex2 : vertices.Floor.Tri1.Vertex2;
				const auto& vertex2 = useCeilTri0 ? vertices.Ceil.Tri0.Vertex1 : vertices.Ceil.Tri1.Vertex1;
				const auto& vertex3 = useCeilTri0 ? vertices.Ceil.Tri0.Vertex2 : vertices.Ceil.Tri1.Vertex2;

				if (vertex0 != vertex2)
					CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, WEST_WALL_NORMAL);
				if (vertex1 != vertex3)
					CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, WEST_WALL_NORMAL);
			}
			// Step wall.
			else if (!isNextSurfWall && !surfVerts.NextNeighborX.IsWall)
			{
				const auto& vertex1 = !surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex1 : surfVerts.Tri1.Vertex1;
				const auto& vertex0 = !surfVerts.IsSplitAngle0 ? surfVerts.Tri0.Vertex2 : surfVerts.Tri1.Vertex2;
				const auto& vertex2 = !surfVerts.IsSplitAngle0 ? surfVerts.NextNeighborX.Vertex0 : surfVerts.NextNeighborX.Vertex0;
				const auto& vertex3 = !surfVerts.IsSplitAngle0 ? surfVerts.NextNeighborX.Vertex1 : surfVerts.NextNeighborX.Vertex1;

				bool isSecondCrissCrossCase = (isFloor ? (vertex1.y < vertex3.y) : !(vertex1.y < vertex3.y));
				if (isFloor ? (vertex0.y > vertex2.y) : (vertex0.y < vertex2.y))
				{
					isSecondCrissCrossCase ?
						CollisionMesh.InsertTriangle(vertex0, vertex2, vertex3, WEST_WALL_NORMAL) :
						CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, WEST_WALL_NORMAL);
				}
				if (isFloor ? (vertex1.y > vertex3.y) : (vertex1.y < vertex3.y))
				{
					isSecondCrissCrossCase ?
						CollisionMesh.InsertTriangle(vertex0, vertex1, vertex3, WEST_WALL_NORMAL) :
						CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, WEST_WALL_NORMAL);
				}
			}
		}

		isFloor = !isFloor;
	}

	return;

	// TODO: Old version below. Refactoring in progress because it's overworked and I'm getting lost.
	//-------------------------------------

	// Calculate 2D corner positions.
	constexpr auto REL_CORNER_0 = Vector2i(0, 0);
	constexpr auto REL_CORNER_1 = Vector2i(0, BLOCK(1));
	constexpr auto REL_CORNER_2 = Vector2i(BLOCK(1), BLOCK(1));
	constexpr auto REL_CORNER_3 = Vector2i(BLOCK(1), 0);
	auto corner0 = sector.Position + REL_CORNER_0;
	auto corner1 = sector.Position + REL_CORNER_1;
	auto corner2 = sector.Position + REL_CORNER_2;
	auto corner3 = sector.Position + REL_CORNER_3;

	// Collect triangles.
	isFloor = true;
	for (int i = 0; i < 2; i++)
	{
		const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;

		bool isSurfSplit = sector.IsSurfaceSplit(isFloor);
		bool isSurfSplitAngle0 = (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

		bool isSurf0Wall = sector.IsWall(0);
		bool isSurf1Wall = sector.IsWall(1);

		// 5) Collect cardinal wall triangles on X axis.
		{
			const auto& prevSurface = isFloor ? prevSectorX.FloorSurface : prevSectorX.CeilingSurface;

			bool useTri0 = (!isSurfSplit || isSurfSplitAngle0);

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

				CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL, prevSectorX.RoomNumber);
				CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL, prevSectorX.RoomNumber);
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

				CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, WEST_WALL_NORMAL, nextSectorX.RoomNumber);
				CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, WEST_WALL_NORMAL, nextSectorX.RoomNumber);
			}
		}

		// 6) Collect cardinal wall triangles on Z axis.
		{
			const auto& prevSurface = isFloor ? prevSectorZ.FloorSurface : prevSectorZ.CeilingSurface;

			bool useTri0 = !(isSurfSplit || !isSurfSplitAngle0);

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

				CollisionMesh.InsertTriangle(vertex0, vertex1, vertex2, NORTH_WALL_NORMAL, prevSectorZ.RoomNumber);
				CollisionMesh.InsertTriangle(vertex1, vertex2, vertex3, NORTH_WALL_NORMAL, prevSectorZ.RoomNumber);
			}
		}

		isFloor = false;
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

			auto& room = g_Level.Rooms[item.RoomNumber];
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
			for (auto& sector : room.Sectors)
				sector.RoomNumber = roomNumber;

			// Update flipped room sectors.
			for (auto& sector : flippedRoom.Sectors)
				sector.RoomNumber = room.flippedRoom;

			// Regenerate neighbor room collision meshes.
			for (int neightborRoomNumber : room.NeighborRoomNumbers)
			{
				auto& neighborRoom = g_Level.Rooms[neightborRoomNumber];
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
		const auto& room = g_Level.Rooms[startRoomNumber];
		for (int neighborRoomNumber : room.NeighborRoomNumbers)
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

	int halfLength = BLOCK(room.XSize) / 2;
	int halfDepth = BLOCK(room.ZSize) / 2;
	int halfHeight = (room.TopHeight - room.BottomHeight) / 2;

	// Calculate and return center.
	return Vector3i(
		room.Position.x + halfLength,
		room.BottomHeight + halfHeight,
		room.Position.z + halfDepth);
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

namespace TEN::Collision::Room
{
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
