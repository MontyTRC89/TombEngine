#include "framework.h"
#include "Game/collision/Los.h"

#include "Game/collision/sphere.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Objects/game_object_ids.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Los
{
	struct SectorTraceData
	{
		struct InterceptData
		{
			const FloorInfo* SectorPtr = nullptr;
			Vector3i		 Position  = Vector3i::Zero;
		};

		std::vector<InterceptData>			   Intercepts = {};
		std::optional<std::pair<Vector3, int>> Intersect  = {};
	};

	static std::vector<ItemInfo*> GetNearbyMoveablePtrs(const std::set<int>& roomNumbers)
	{
		// Collect moveable pointers.
		auto movPtrs = std::vector<ItemInfo*>{};
		for (int movID = 0; movID < g_Level.NumItems; movID++)
		{
			auto& mov = g_Level.Items[movID];

			// 1) Check moveable status.
			if (mov.Status == ItemStatus::ITEM_INVISIBLE || mov.Status == ItemStatus::ITEM_DEACTIVATED)
				continue;

			// 2) Check if room is active.
			const auto& room = g_Level.Rooms[mov.RoomNumber];
			if (!room.Active())
				continue;

			// 3) Test if moveable is in nearby room.
			if (!Contains(room.neighbors, (int)mov.RoomNumber))
				continue;

			movPtrs.push_back(&mov);
		}

		return movPtrs;
	}

	static std::vector<MESH_INFO*> GetNearbyStaticPtrs(const std::set<int>& roomNumbers)
	{
		// Collect static pointers.
		auto staticPtrs = std::vector<MESH_INFO*>{};
		for (int roomNumber : roomNumbers)
		{
			const auto& room = g_Level.Rooms[roomNumber];
			for (auto& neighborRoomNumber : room.neighbors)
			{
				// 1) Check if room is active.
				auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
				if (!neighborRoom.Active())
					continue;

				// 2) Run through statics in room.
				for (auto& staticObj : neighborRoom.mesh)
					staticPtrs.push_back(&staticObj);
			}
		}

		return staticPtrs;
	}

	std::vector<LosInstanceData> GetLosInstances(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
												 bool collideMoveables, bool collideSpheres, bool collideStatics)
	{
		auto losInstances = std::vector<LosInstanceData>{};

		// 1) Collect room LOS instance.
		auto roomLos = GetRoomLos(origin, roomNumber, dir, dist);
		if (roomLos.Intersect.has_value())
		{
			dist = Vector3::Distance(origin, roomLos.Intersect->first);
			losInstances.push_back(LosInstanceData{ {}, NO_VALUE, roomLos.Intersect->first, roomLos.Intersect->second, dist });
		}

		if (collideMoveables || collideSpheres)
		{
			auto movPtrs = GetNearbyMoveablePtrs(roomLos.RoomNumbers);
			for (auto* movPtr : movPtrs)
			{
				// 2) Collect moveable LOS instances.
				if (collideMoveables)
				{
					auto box = GameBoundingBox(movPtr).ToBoundingOrientedBox(movPtr->Pose);

					float intersectDist = 0.0f;
					if (box.Intersects(origin, dir, intersectDist))
					{
						if (intersectDist <= dist)
						{
							auto intersectPos = Geometry::TranslatePoint(origin, dir, intersectDist);
							auto offset = intersectPos - movPtr->Pose.Position.ToVector3();
							int roomNumber = GetCollision(movPtr->Pose.Position, movPtr->RoomNumber, offset).RoomNumber;

							losInstances.push_back(LosInstanceData{ movPtr, NO_VALUE, intersectPos, roomNumber, intersectDist });
						}
					}
				}

				// 3) Collect moveable sphere LOS instances.
				if (collideSpheres)
				{
					int sphereCount = GetSpheres(movPtr, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
					for (int i = 0; i < sphereCount; i++)
					{
						auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);

						float intersectDist = 0.0f;
						if (sphere.Intersects(origin, dir, intersectDist))
						{
							if (intersectDist <= dist)
							{
								auto intersectPos = Geometry::TranslatePoint(origin, dir, intersectDist);
								auto offset = intersectPos - movPtr->Pose.Position.ToVector3();
								int roomNumber = GetCollision(movPtr->Pose.Position, movPtr->RoomNumber, offset).RoomNumber;

								losInstances.push_back(LosInstanceData{ movPtr, i, intersectPos, roomNumber, intersectDist });
							}
						}
					}
				}
			}
		}

		// 4) Collect static LOS instances.
		if (collideStatics)
		{
			auto staticPtrs = GetNearbyStaticPtrs(roomLos.RoomNumbers);
			for (auto* staticPtr : staticPtrs)
			{
				auto box = GetBoundsAccurate(*staticPtr, false).ToBoundingOrientedBox(staticPtr->pos);

				float intersectDist = 0.0f;
				if (box.Intersects(origin, dir, intersectDist))
				{
					if (intersectDist <= dist)
					{
						auto intersectPos = Geometry::TranslatePoint(origin, dir, intersectDist);
						auto offset = intersectPos - staticPtr->pos.Position.ToVector3();
						int roomNumber = GetCollision(staticPtr->pos.Position, staticPtr->roomNumber, offset).RoomNumber;

						losInstances.push_back(LosInstanceData{ staticPtr, NO_VALUE, intersectPos, roomNumber, intersectDist });
					}
				}
			}
		}

		// Sort LOS instances by distance.
		std::sort(
			losInstances.begin(), losInstances.end(),
			[](const auto& losInstance0, const auto& losInstance1)
			{
				return (losInstance0.Distance < losInstance1.Distance);
			});

		// Return room and object LOS instances sorted by distance.
		return losInstances;
	}
	
	static void SetSectorTraceIntercepts(SectorTraceData& trace, const Vector3i& origin, int originRoomNumber, const Vector3i& target)
	{
		auto deltaPos = target - origin;

		// 1) Collect origin.
		trace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, origin });

		// 2) Calculate and collect X axis positions.
		if (deltaPos.x != 0)
		{
			// Calculate step.
			auto step = Vector3i(
				deltaPos.x,
				BLOCK(deltaPos.y) / deltaPos.x,
				BLOCK(deltaPos.z) / deltaPos.x);

			bool isNegative = (step.x < 0);
			int sign = isNegative ? -1 : 1;

			// Calculate initial position.
			auto pos = Vector3i::Zero;
			pos.x = isNegative ? (origin.x & (UINT_MAX - WALL_MASK)) : (origin.x | WALL_MASK);
			pos.y = (((pos.x - origin.x) * step.y) / BLOCK(1)) + origin.y;
			pos.z = (((pos.x - origin.x) * step.z) / BLOCK(1)) + origin.z;

			// Collect intercept positions.
			while (isNegative ? (pos.x > target.x) : (pos.x < target.x))
			{
				trace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos });
				trace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos + Vector3i(sign, 0, 0) });

				pos += Vector3i(BLOCK(1), step.y, step.z) * sign;
			}
		}

		// 3) Calculate and collect Z axis positions.
		if (deltaPos.z != 0)
		{
			// Calculate step.
			auto step = Vector3i(
				BLOCK(deltaPos.x) / deltaPos.z,
				BLOCK(deltaPos.y) / deltaPos.z,
				deltaPos.z);

			bool isNegative = (step.z < 0);
			int sign = isNegative ? -1 : 1;

			// Calculate initial position.
			auto pos = Vector3i::Zero;
			pos.z = isNegative ? (origin.z & (UINT_MAX - WALL_MASK)) : (origin.z | WALL_MASK);
			pos.x = (((pos.z - origin.z) * step.x) / BLOCK(1)) + origin.x;
			pos.y = (((pos.z - origin.z) * step.y) / BLOCK(1)) + origin.y;

			// Collect positions.
			while (isNegative ? (pos.z > target.z) : (pos.z < target.z))
			{
				trace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos });
				trace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos + Vector3i(0, 0, sign) });

				pos += Vector3i(step.x, step.y, BLOCK(1)) * sign;
			}
		}

		// 4) Sort intercepts by distance from origin.
		std::sort(
			trace.Intercepts.begin(), trace.Intercepts.end(),
			[origin](const SectorTraceData::InterceptData& intercept0, const SectorTraceData::InterceptData& intercept1)
			{
				float distSqr0 = Vector3i::DistanceSquared(origin, intercept0.Position);
				float distSqr1 = Vector3i::DistanceSquared(origin, intercept1.Position);

				return (distSqr0 < distSqr1);
			});


		// 5) Set sector pointers.
		short roomNumber = originRoomNumber;
		for (auto& intercept : trace.Intercepts)
		{
			auto* sectorPtr = GetFloor(intercept.Position.x, intercept.Position.y, intercept.Position.z, &roomNumber);
			intercept.SectorPtr = sectorPtr;
		}
	}

	static int GetSurfaceTriangleHeight(const FloorInfo& sector, int relX, int relZ, int triID, bool isFloor)
	{
		const auto& tri = isFloor ? sector.FloorSurface.Triangles[triID] : sector.CeilingSurface.Triangles[triID];

		auto normal = tri.Plane.Normal();
		float relPlaneHeight = -((normal.x * relX) + (normal.z * relZ)) / normal.y;
		return (tri.Plane.D() + relPlaneHeight);
	}

	static std::vector<TriangleMesh> GenerateSectorTriangleMeshes(const Vector3& pos, const FloorInfo& sector, bool isFloor)
	{
		constexpr auto REL_CORNER_0 = Vector3(0.0f, 0.0f, 0.0f);
		constexpr auto REL_CORNER_1 = Vector3(0.0f, 0.0f, BLOCK(1));
		constexpr auto REL_CORNER_2 = Vector3(BLOCK(1), 0.0f, BLOCK(1));
		constexpr auto REL_CORNER_3 = Vector3(BLOCK(1), 0.0f, 0.0f);

		const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;
		bool isTri0Portal = surface.Triangles[0].PortalRoomNumber != NO_VALUE;
		bool isTri1Portal = surface.Triangles[1].PortalRoomNumber != NO_VALUE;

		auto base = Vector3(FloorToStep(pos.x, BLOCK(1)), 0.0f, FloorToStep(pos.z, BLOCK(1)));
		auto corner0 = base;
		auto corner1 = base + Vector3(0.0f, 0.0f, BLOCK(1));
		auto corner2 = base + Vector3(BLOCK(1), 0.0f, BLOCK(1));
		auto corner3 = base + Vector3(BLOCK(1), 0.0f, 0.0f);

		auto tris = std::vector<TriangleMesh>{};
		if (sector.IsSurfaceSplit(isFloor))
		{
			if (!isTri0Portal || !isTri1Portal)
			{
				if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
				{
					// Surface triangle 0.
					auto tri0 = TriangleMesh(
						Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 0), corner0.z),
						Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
						Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z));

					if (!isTri0Portal)
						tris.push_back(tri0);

					// Surface triangle 1.
					auto tri1 = TriangleMesh(
						Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
						Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 1), corner2.z),
						Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

					if (!isTri1Portal)
						tris.push_back(tri1);

					// Diagonal wall triangles.
					if (tri0.Vertices[0] != tri1.Vertices[0] && tri0.Vertices[2] != tri1.Vertices[1])
					{
						auto tri2 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[0], tri0.Vertices[2]);
						tris.push_back(tri2);

						auto tri3 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[1]);
						tris.push_back(tri3);
					}
					else if (tri0.Vertices[0] != tri1.Vertices[0] && tri0.Vertices[2] == tri1.Vertices[1])
					{
						auto tri2 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[0], tri0.Vertices[2]);
						tris.push_back(tri2);
					}
					else if (tri0.Vertices[2] == tri1.Vertices[1] && tri0.Vertices[2] != tri1.Vertices[1])
					{
						auto tri2 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[1]);
						tris.push_back(tri2);
					}
				}
				else if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_1)
				{
					// Surface triangle 0.
					auto tri0 = TriangleMesh(
						Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
						Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z),
						Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 0), corner3.z));

					if (!isTri0Portal)
						tris.push_back(tri0);

					// Surface triangle 1.
					auto tri1 = TriangleMesh(
						Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
						Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 1), corner1.z),
						Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

					if (!isTri1Portal)
						tris.push_back(tri1);

					// Diagonal wall triangles.
					if (tri1.Vertices[1] != tri0.Vertices[0] && tri1.Vertices[2] != tri0.Vertices[2])
					{
						auto tri2 = TriangleMesh(tri1.Vertices[1], tri0.Vertices[0], tri1.Vertices[2]);
						tris.push_back(tri2);

						auto tri3 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[2], tri0.Vertices[2]);
						tris.push_back(tri3);
					}
					else if (tri1.Vertices[1] != tri0.Vertices[0] && tri1.Vertices[2] == tri0.Vertices[2])
					{
						auto tri2 = TriangleMesh(tri1.Vertices[1], tri0.Vertices[0], tri1.Vertices[2]);
						tris.push_back(tri2);
					}
					else if (tri1.Vertices[2] == tri0.Vertices[2] && tri1.Vertices[2] != tri0.Vertices[2])
					{
						auto tri2 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[2], tri0.Vertices[2]);
						tris.push_back(tri2);
					}
				}
			}
		}
		else
		{
			// Surface triangle 0.
			if (!isTri0Portal)
			{
				auto tri0 = TriangleMesh(
					Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 0), corner0.z),
					Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
					Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z));
				tris.push_back(tri0);
			}

			// Surface triangle 1.
			if (!isTri1Portal)
			{
				auto tri1 = TriangleMesh(
					Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
					Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 1), corner2.z),
					Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));
				tris.push_back(tri1);
			}
		}

		// Debug
		for (const auto& tri : tris)
		{
			auto offset = (Vector3::UnitY * 4) * (isFloor ? -1 : 1);
			g_Renderer.AddDebugTriangle(tri.Vertices[0] + offset, tri.Vertices[1] + offset, tri.Vertices[2] + offset, Color(1.0f, 1.0f, 0.0f, 0.1f));
		}

		return tris;
	}

	static std::vector<TriangleMesh> GenerateBridgeTriangleMeshes(const BoundingOrientedBox& box, const Vector3& offset)
	{
		// Get box corners.
		auto corners = std::array<Vector3, 8>{};
		box.GetCorners(corners.data());

		// Offset key corners.
		corners[1] += offset;
		corners[3] -= offset;
		corners[5] += offset;
		corners[7] -= offset;

		// Calculate and return collision mesh.
		return std::vector<TriangleMesh>
		{
			TriangleMesh(corners[0], corners[1], corners[4]),
			TriangleMesh(corners[1], corners[4], corners[5]),
			TriangleMesh(corners[2], corners[3], corners[6]),
			TriangleMesh(corners[3], corners[6], corners[7]),
			TriangleMesh(corners[0], corners[1], corners[2]),
			TriangleMesh(corners[0], corners[2], corners[3]),
			TriangleMesh(corners[0], corners[3], corners[4]),
			TriangleMesh(corners[3], corners[4], corners[7]),
			TriangleMesh(corners[1], corners[2], corners[5]),
			TriangleMesh(corners[2], corners[5], corners[6]),
			TriangleMesh(corners[4], corners[5], corners[6]),
			TriangleMesh(corners[4], corners[6], corners[7])
		};
	}

	static void ClipSectorTrace(SectorTraceData& trace, const Ray& ray, float distMax, bool collideBridges)
	{
		float closestDist = INFINITY;
		int roomNumber = NO_VALUE;

		// Run through intercepts sorted by distance.
		const FloorInfo* prevSectorPtr = nullptr;
		for (const auto& intercept : trace.Intercepts)
		{
			// Debug
			g_Renderer.AddDebugTarget(intercept.Position.ToVector3(), Quaternion::Identity, 30, Color(1.0f, 0.0f, 0.0f));

			// 1) Clip wall.
			if (intercept.Position.y > GetFloorHeight(intercept.SectorPtr, intercept.Position.x, intercept.Position.y, intercept.Position.z) ||
				intercept.Position.y < GetCeiling(intercept.SectorPtr, intercept.Position.x, intercept.Position.y, intercept.Position.z))
			{
				float dist = Vector3::Distance(ray.position, intercept.Position.ToVector3());
				if (dist < closestDist)
					closestDist = dist;
			}

			if (intercept.SectorPtr != prevSectorPtr)
			{
				// 2) Clip floor.
				auto floorTris = GenerateSectorTriangleMeshes(intercept.Position.ToVector3(), *intercept.SectorPtr, true);
				for (const auto& tri : floorTris)
				{
					float dist = 0.0f;
					if (tri.Intersects(ray, dist) && dist < closestDist && dist <= distMax)
						closestDist = dist;
				}

				// 3) Clip ceiling.
				auto ceilTris = GenerateSectorTriangleMeshes(intercept.Position.ToVector3(), *intercept.SectorPtr, false);
				for (const auto& tri : ceilTris)
				{
					float dist = 0.0f;
					if (tri.Intersects(ray, dist) && dist < closestDist && dist <= distMax)
						closestDist = dist;
				}

				// 4) Clip bridge (if applicable).
				if (collideBridges)
				{
					for (int movID : intercept.SectorPtr->BridgeItemNumbers)
					{
						const auto& bridgeMov = g_Level.Items[movID];

						if (bridgeMov.Status == ItemStatus::ITEM_INVISIBLE || bridgeMov.Status == ItemStatus::ITEM_DEACTIVATED)
							continue;

						// Determine relative tilt offset.
						auto offset = Vector3::Zero;
						switch (bridgeMov.ObjectNumber)
						{
						default:
							break;

						case ID_BRIDGE_TILT1:
							offset = Vector3(0.0f, CLICK(1), 0.0f);
							break;

						case ID_BRIDGE_TILT2:
							offset = Vector3(0.0f, CLICK(2), 0.0f);
							break;

						case ID_BRIDGE_TILT3:
							offset = Vector3(0.0f, CLICK(3), 0.0f);
							break;

						case ID_BRIDGE_TILT4:
							offset = Vector3(0.0f, CLICK(4), 0.0f);
							break;
						}

						// Calculate absolute tilt offset.
						auto rotMatrix = bridgeMov.Pose.Orientation.ToRotationMatrix();
						offset = Vector3::Transform(offset, rotMatrix);

						// Collide bridge mesh.
						auto box = GameBoundingBox(&bridgeMov).ToBoundingOrientedBox(bridgeMov.Pose);
						auto bridgeTris = GenerateBridgeTriangleMeshes(box, offset);
						for (const auto& tri : bridgeTris)
						{
							float dist = 0.0f;
							if (tri.Intersects(ray, dist) && dist < closestDist && dist <= distMax)
								closestDist = dist;
						}
					}
				}
			}
			prevSectorPtr = intercept.SectorPtr;

			// Has clip; set intersection ang return early.
			if (closestDist != INFINITY)
			{
				auto intersectPos = Geometry::TranslatePoint(ray.position, ray.direction, closestDist);
				trace.Intersect = std::pair(intersectPos, intercept.SectorPtr->RoomNumber);
			}
		}
	}

	static SectorTraceData GetSectorTrace(const Vector3& origin, int originRoomNumber, const Vector3& target, bool collideBridges)
	{
		// Create ray.
		auto dir = target - origin;
		dir.Normalize();
		auto ray = Ray(origin, dir);

		// Calculate max ray distance.
		float distMax = Vector3::Distance(origin, target);

		// Calculate trace.
		auto trace = SectorTraceData{};
		SetSectorTraceIntercepts(trace, origin, originRoomNumber, target);
		ClipSectorTrace(trace, ray, distMax, collideBridges);

		// Return trace.
		return trace;
	}

	RoomLosData GetRoomLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges)
	{
		auto roomLos = RoomLosData{};

		// Get trace.
		auto target = Geometry::TranslatePoint(origin, dir, dist);
		auto trace = GetSectorTrace(origin, roomNumber, target, collideBridges);

		// Set room LOS.
		roomLos.Intersect = trace.Intersect;
		for (const auto& intercept : trace.Intercepts)
			roomLos.RoomNumbers.insert(intercept.SectorPtr->RoomNumber);

		return roomLos;
	}

	std::optional<MoveableLosData> GetMoveableLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer)
	{
		auto losInstances = GetLosInstances(origin, roomNumber, dir, dist, true, false, false);
		for (auto& losInstance : losInstances)
		{
			// 1) FAILSAFE: Ignore sphere LOS.
			if (losInstance.SphereID != NO_VALUE)
				continue;

			// 2) Check for object.
			if (!losInstance.ObjectPtr.has_value())
				continue;

			// 3) Check if object is moveable.
			if (!std::holds_alternative<ItemInfo*>(*losInstance.ObjectPtr))
				continue;

			auto& item = *std::get<ItemInfo*>(*losInstance.ObjectPtr);

			// 4) Check if moveable is not player (if applicable).
			if (!collidePlayer && item.ObjectNumber == ID_LARA)
				continue;

			return MoveableLosData{ item, std::pair(losInstance.Position, losInstance.RoomNumber), NO_VALUE };
		}

		return std::nullopt;
	}

	std::optional<MoveableLosData> GetMoveableSphereLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer)
	{
		auto losInstances = GetLosInstances(origin, roomNumber, dir, dist, false, true, false);
		for (auto& losInstance : losInstances)
		{
			// 1) Check for sphere LOS.
			if (losInstance.SphereID == NO_VALUE)
				continue;

			// 2) Check for object.
			if (!losInstance.ObjectPtr.has_value())
				continue;

			// 3) Check if object is moveable.
			if (!std::holds_alternative<ItemInfo*>(*losInstance.ObjectPtr))
				continue;

			auto& mov = *std::get<ItemInfo*>(*losInstance.ObjectPtr);

			// 4) Check if moveable is not player (if applicable).
			if (!collidePlayer && mov.ObjectNumber == ID_LARA)
				continue;

			return MoveableLosData{ mov, std::pair(losInstance.Position, losInstance.RoomNumber), losInstance.SphereID };
		}

		return std::nullopt;
	}

	std::optional<StaticLosData> GetStaticLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid)
	{
		auto losInstances = GetLosInstances(origin, roomNumber, dir, dist, false, false, true);
		for (auto& losInstance : losInstances)
		{
			// 1) FAILSAFE: Ignore sphere LOS.
			if (losInstance.SphereID != NO_VALUE)
				continue;

			// 2) Check for object.
			if (!losInstance.ObjectPtr.has_value())
				continue;

			// 3) Check if object is static.
			if (!std::holds_alternative<MESH_INFO*>(*losInstance.ObjectPtr))
				continue;

			auto& staticObj = *std::get<MESH_INFO*>(*losInstance.ObjectPtr);

			// 4) Check if static is solid (if applicable).
			if (collideOnlySolid && !(staticObj.flags & StaticMeshFlags::SM_SOLID))
				continue;

			return StaticLosData{ staticObj, std::pair(losInstance.Position, losInstance.RoomNumber) };
		}

		return std::nullopt;
	}

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos)
	{
		auto posPair = g_Renderer.GetRay(screenPos);

		auto origin = GameVector(posPair.first, Camera.RoomNumber);
		auto target = GameVector(posPair.second, Camera.RoomNumber);

		// TODO
		//LOS(&origin, &target);
		return std::pair<GameVector, GameVector>(origin, target);
	}
}
