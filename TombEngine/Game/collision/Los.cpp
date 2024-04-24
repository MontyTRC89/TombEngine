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
	struct SectorInterceptData
	{
		const FloorInfo* SectorPtr	= nullptr;
		Vector3i		 Position	= Vector3i::Zero;
	};

	struct SectorTraceData
	{
		bool							 IsIntersected = false;
		std::pair<Vector3, int>			 Intersect	   = {};
		std::vector<SectorInterceptData> Intercepts	   = {};
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

	std::vector<LosInstanceData> GetLosInstances(const Vector3& origin, int originRoomNumber, const Vector3& dir, float dist,
												 bool collideMoveables, bool collideSpheres, bool collideStatics)
	{
		auto losInstances = std::vector<LosInstanceData>{};

		// Calculate target.
		auto target = Geometry::TranslatePoint(origin, dir, dist);

		// 1) Collect room LOS instance.
		auto roomLos = GetRoomLos(origin, originRoomNumber, target);
		if (roomLos.Intersect.has_value())
		{
			target = roomLos.Intersect->first;
			dist = Vector3::Distance(origin, target);

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

		auto base = Vector3(FloorToStep(pos.x, BLOCK(1)), 0.0f, FloorToStep(pos.z, BLOCK(1)));
		auto corner0 = base;
		auto corner1 = base + Vector3(0.0f, 0.0f, BLOCK(1));
		auto corner2 = base + Vector3(BLOCK(1), 0.0f, BLOCK(1));
		auto corner3 = base + Vector3(BLOCK(1), 0.0f, 0.0f);

		auto tris = std::vector<TriangleMesh>{};
		if (sector.IsSurfaceSplit(isFloor))
		{
			const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;

			if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
			{
				// Calculate triangle 0.
				auto tri0 = TriangleMesh(
					Vector3(corner0.x, GetSurfaceTriangleHeight(sector, REL_CORNER_0.x, REL_CORNER_0.z, 0, isFloor), corner0.z),
					Vector3(corner1.x, GetSurfaceTriangleHeight(sector, REL_CORNER_1.x, REL_CORNER_1.z, 0, isFloor), corner1.z),
					Vector3(corner2.x, GetSurfaceTriangleHeight(sector, REL_CORNER_2.x, REL_CORNER_2.z, 0, isFloor), corner2.z));

				// Calculate triangle 1.
				auto tri1 = TriangleMesh(
					Vector3(corner0.x, GetSurfaceTriangleHeight(sector, REL_CORNER_0.x, REL_CORNER_0.z, 1, isFloor), corner0.z),
					Vector3(corner2.x, GetSurfaceTriangleHeight(sector, REL_CORNER_2.x, REL_CORNER_2.z, 1, isFloor), corner2.z),
					Vector3(corner3.x, GetSurfaceTriangleHeight(sector, REL_CORNER_3.x, REL_CORNER_3.z, 1, isFloor), corner3.z));

				// Collect surface triangles.
				tris.push_back(tri0);
				tris.push_back(tri1);

				// Calculate and collect diagonal wall triangles.
				if (tri0.Vertices[0] != tri1.Vertices[0] && tri0.Vertices[2] != tri1.Vertices[1])
				{
					auto tri2 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[0], tri0.Vertices[2]);
					auto tri3 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[1]);

					tris.push_back(tri2);
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
				// Calculate triangle 0.
				auto tri1 = TriangleMesh(
					Vector3(corner1.x, GetSurfaceTriangleHeight(sector, REL_CORNER_1.x, REL_CORNER_1.z, 0, isFloor), corner1.z),
					Vector3(corner2.x, GetSurfaceTriangleHeight(sector, REL_CORNER_2.x, REL_CORNER_2.z, 0, isFloor), corner2.z),
					Vector3(corner3.x, GetSurfaceTriangleHeight(sector, REL_CORNER_3.x, REL_CORNER_3.z, 0, isFloor), corner3.z));

				// Calculate triangle 1.
				auto tri0 = TriangleMesh(
					Vector3(corner0.x, GetSurfaceTriangleHeight(sector, REL_CORNER_0.x, REL_CORNER_0.z, 1, isFloor), corner0.z),
					Vector3(corner1.x, GetSurfaceTriangleHeight(sector, REL_CORNER_1.x, REL_CORNER_1.z, 1, isFloor), corner1.z),
					Vector3(corner3.x, GetSurfaceTriangleHeight(sector, REL_CORNER_3.x, REL_CORNER_3.z, 1, isFloor), corner3.z));

				// Collect surface triangles.
				tris.push_back(tri0);
				tris.push_back(tri1);

				// Calculate and collect diagonal wall triangles.
				if (tri0.Vertices[1] != tri1.Vertices[0] && tri0.Vertices[2] != tri1.Vertices[2])
				{
					auto tri2 = TriangleMesh(tri0.Vertices[1], tri1.Vertices[0], tri0.Vertices[2]);
					auto tri3 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[2]);

					tris.push_back(tri2);
					tris.push_back(tri3);
				}
				else if (tri0.Vertices[1] != tri1.Vertices[0] && tri0.Vertices[2] == tri1.Vertices[2])
				{
					auto tri2 = TriangleMesh(tri0.Vertices[1], tri1.Vertices[0], tri0.Vertices[2]);
					tris.push_back(tri2);
				}
				else if (tri0.Vertices[2] == tri1.Vertices[2] && tri0.Vertices[2] != tri1.Vertices[2])
				{
					auto tri2 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[2]);
					tris.push_back(tri2);
				}
			}
		}
		else
		{
			// Calculate triangle 0.
			auto tri0 = TriangleMesh(
				Vector3(corner0.x, GetSurfaceTriangleHeight(sector, REL_CORNER_0.x, REL_CORNER_0.z, 0, isFloor), corner0.z),
				Vector3(corner1.x, GetSurfaceTriangleHeight(sector, REL_CORNER_1.x, REL_CORNER_1.z, 0, isFloor), corner1.z),
				Vector3(corner2.x, GetSurfaceTriangleHeight(sector, REL_CORNER_2.x, REL_CORNER_2.z, 0, isFloor), corner2.z));

			// Calculate triangle 1.
			auto tri1 = TriangleMesh(
				Vector3(corner0.x, GetSurfaceTriangleHeight(sector, REL_CORNER_0.x, REL_CORNER_0.z, 1, isFloor), corner0.z),
				Vector3(corner2.x, GetSurfaceTriangleHeight(sector, REL_CORNER_2.x, REL_CORNER_2.z, 1, isFloor), corner2.z),
				Vector3(corner3.x, GetSurfaceTriangleHeight(sector, REL_CORNER_3.x, REL_CORNER_3.z, 1, isFloor), corner3.z));

			// Collect surface triangles.
			tris.push_back(tri0);
			tris.push_back(tri1);
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

	static std::vector<Vector3i> GetSectorTraceInterceptPositions(const Vector3i& origin, const Vector3i& target)
	{
		auto posList = std::vector<Vector3i>{};

		auto deltaPos = target - origin;
		if (deltaPos.x == 0 || deltaPos.z == 0)
			return posList; //temp

		// Calculate steps.
		auto xStep = Vector3i(
			deltaPos.x,
			BLOCK(deltaPos.y) / deltaPos.x,
			BLOCK(deltaPos.z) / deltaPos.x);
		auto zStep = Vector3i(
			BLOCK(deltaPos.x) / deltaPos.z,
			BLOCK(deltaPos.y) / deltaPos.z,
			deltaPos.z);

		bool xIsNegative = (xStep.x < 0);
		bool zIsNegative = (zStep.z < 0);
		int xSign = xIsNegative ? -1 : 1;
		int zSign = zIsNegative ? -1 : 1;

		// Calculate initial positions.
		auto xIntercept = Vector3i::Zero;
		xIntercept.x = xIsNegative ? (origin.x & (UINT_MAX - WALL_MASK)) : (origin.x | WALL_MASK);
		xIntercept.y = (((xIntercept.x - origin.x) * xStep.y) / BLOCK(1)) + origin.y;
		xIntercept.z = (((xIntercept.x - origin.x) * xStep.z) / BLOCK(1)) + origin.z;

		auto zIntercept = Vector3i::Zero;
		zIntercept.z = zIsNegative ? (origin.z & (UINT_MAX - WALL_MASK)) : (origin.z | WALL_MASK);
		zIntercept.x = (((zIntercept.z - origin.z) * zStep.x) / BLOCK(1)) + origin.x;
		zIntercept.y = (((zIntercept.z - origin.z) * zStep.y) / BLOCK(1)) + origin.y;

		//g_Renderer.AddDebugTarget(xIntercept.ToVector3(), Quaternion::Identity, 50, Color(0, 0, 1));
		//g_Renderer.AddDebugTarget(Vector3(xIntercept.x + (isX ? sign : 0), xIntercept.y, xIntercept.z + (isX ? 0 : sign)), Quaternion::Identity, 50, Color(0, 1, 0));

		while ((xIsNegative ? (xIntercept.x > target.x) : (xIntercept.x < target.x)) ||
			   (zIsNegative ? (zIntercept.z > target.z) : (zIntercept.z < target.z)))
		{
			if ((xIsNegative ? (xIntercept.x > target.x) : (xIntercept.x < target.x)))
			{
				posList.push_back(xIntercept);
				posList.push_back(xIntercept + Vector3i(xSign, 0, 0));

				xIntercept += Vector3i(BLOCK(1), xStep.y, xStep.z) * xSign;
			}

			if (zIsNegative ? (zIntercept.z > target.z) : (zIntercept.z < target.z))
			{
				posList.push_back(zIntercept);
				posList.push_back(zIntercept + Vector3i(0, 0, zSign));

				zIntercept += Vector3i(zStep.x, zStep.y, BLOCK(1)) * zSign;
			}
		}

		// Temp sort.
		std::sort(
			posList.begin(), posList.end(),
			[origin](const Vector3i& intercept0, const Vector3i& intercept1)
			{
				float dist0 = Vector3i::Distance(origin, intercept0);
				float dist1 = Vector3i::Distance(origin, intercept1);

				return (dist0 < dist1);
			});

		return posList;
	}

	static void ClipSectorTrace(SectorTraceData& trace, const Ray& ray)
	{
		float closestDist = INFINITY;
		int roomNumber = trace.Intersect.second;

		// Run through intercepts sorted by distance.
		for (const auto& intercept : trace.Intercepts)
		{
			// 1) Clip bridge.
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
				auto tris = GenerateBridgeTriangleMeshes(box, offset);
				for (const auto& tri : tris)
				{
					float dist = 0.0f;
					if (tri.Intersects(ray, dist) && dist < closestDist)
					{
						closestDist = dist;
						roomNumber = intercept.SectorPtr->RoomNumber;
					}
				}
			}

			// 2) Clip floor.
			auto floorTris = GenerateSectorTriangleMeshes(intercept.Position.ToVector3(), *intercept.SectorPtr, true);
			for (const auto& tri : floorTris)
			{
				float dist = 0.0f;
				if (tri.Intersects(ray, dist) && dist < closestDist)
				{	
					closestDist = dist;
					roomNumber = intercept.SectorPtr->RoomNumber;
				}
			}

			// 3) Clip ceiling.
			auto ceilTris = GenerateSectorTriangleMeshes(intercept.Position.ToVector3(), *intercept.SectorPtr, false);
			for (const auto& tri : ceilTris)
			{
				float dist = 0.0f;
				if (tri.Intersects(ray, dist) && dist < closestDist)
				{	
					closestDist = dist;
					roomNumber = intercept.SectorPtr->RoomNumber;
				}
			}

			// Has clip; break early.
			if (closestDist != INFINITY)
				break;
		}

		// Clip trace intersection (if applicable).
		if (closestDist != INFINITY)
		{
			auto intersectPos = Geometry::TranslatePoint(ray.position, ray.direction, closestDist);
			trace.Intersect = std::pair(intersectPos, roomNumber);
		}
	}

	static SectorTraceData GetSectorTrace(const Vector3& origin, int originRoomNumber, const Vector3& target)
	{
		auto trace = SectorTraceData{};

		// Create ray.
		auto dir = target - origin;
		dir.Normalize();
		auto ray = Ray(origin, dir);

		// Establish base data.
		FloorInfo* sectorPtr = nullptr;
		FloorInfo* prevSectorPtr = nullptr;
		short roomNumber = originRoomNumber;
		short prevRoomNumber = originRoomNumber;

		// Run through sector trace intercepts.
		auto posList = GetSectorTraceInterceptPositions(origin, target);
		for (const auto& pos : posList)
		{
			sectorPtr = GetFloor(pos.x, pos.y, pos.z, &prevRoomNumber);

			// TODO: Duplicates.
			// Collect sector.
			//if (sectorPtr != prevSectorPtr)
				trace.Intercepts.push_back(SectorInterceptData{ sectorPtr, pos });
			prevSectorPtr = sectorPtr;

			// Collect room number.
			//if (roomNumber != prevRoomNumber)
				//trace.front().RoomNumbers.insert(prevRoomNumber);
			roomNumber = prevRoomNumber;

			// Test for obstruction.
			if (pos.y > GetFloorHeight(sectorPtr, pos.x, pos.y, pos.z) ||
				pos.y < GetCeiling(sectorPtr, pos.x, pos.y, pos.z))
			{
				trace.IsIntersected = true;
				trace.Intersect = std::pair(pos.ToVector3(), roomNumber);
				break;
			}
		}

		// Sort by distance.
		/*std::sort(
			trace.Intercepts.begin(), trace.Intercepts.end(),
			[origin](const SectorInterceptData& intercept0, const SectorInterceptData& intercept1)
			{
				float dist0 = Vector3i::DistanceSquared(origin, intercept0.Position);
				float dist1 = Vector3i::DistanceSquared(origin, intercept1.Position);
				return (dist0 < dist1);
			});

		// Remove duplicates.
		trace.Intercepts.erase(
			std::unique(trace.Intercepts.begin(), trace.Intercepts.end(),
				[](const SectorInterceptData& intercept0, const SectorInterceptData& intercept1)
				{
					return (intercept0.SectorPtr == intercept1.SectorPtr);
				}),
			trace.Intercepts.end());*/

		// Clip and return trace.
		ClipSectorTrace(trace, ray);
		return trace;
	}

	RoomLosData GetRoomLos(const Vector3& origin, int originRoomNumber, const Vector3& target)
	{
		auto roomLos = RoomLosData{};

		auto trace = GetSectorTrace(origin, originRoomNumber, target);
		if (trace.IsIntersected)
			roomLos.Intersect = trace.Intersect;

		for (const auto& intercept : trace.Intercepts)
			roomLos.RoomNumbers.insert(intercept.SectorPtr->RoomNumber);

		return roomLos;

		// Legacy wrapper version.
		/*auto losOrigin = GameVector(origin, originRoomNumber);
		auto losTarget = GameVector(target, originRoomNumber);

		float dist = 0.0f;
		auto roomNumbers = std::set<int>{};

		if (!LOS(&losOrigin, &losTarget, &roomNumbers))
			dist = Vector3::Distance(origin, losTarget.ToVector3());

		// TODO: Check.
		// Calculate intersection (if applicable).
		auto intersect = std::optional<std::pair<Vector3, int>>();
		if (dist != 0.0f)
		{
			auto dir = target - origin;
			dir.Normalize();

			intersect = std::pair(Geometry::TranslatePoint(origin, dir, dist), losTarget.RoomNumber);
		}

		// Return room LOS.
		return RoomLosData{ intersect, roomNumbers };*/
	}

	std::optional<MoveableLosData> GetMoveableLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool ignorePlayer)
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
			if (ignorePlayer && item.ObjectNumber == ID_LARA)
				continue;

			return MoveableLosData{ item, std::pair(losInstance.Position, losInstance.RoomNumber), NO_VALUE };
		}

		return std::nullopt;
	}

	std::optional<MoveableLosData> GetMoveableSphereLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool ignorePlayer)
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
			if (ignorePlayer && mov.ObjectNumber == ID_LARA)
				continue;

			return MoveableLosData{ mov, std::pair(losInstance.Position, losInstance.RoomNumber), losInstance.SphereID };
		}

		return std::nullopt;
	}

	std::optional<StaticLosData> GetStaticLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool onlySolid)
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
			if (onlySolid && !(staticObj.flags & StaticMeshFlags::SM_SOLID))
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
