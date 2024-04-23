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
		bool IsIntersected = false;

		std::pair<Vector3, int> Intersect	= {};
		std::vector<const FloorInfo*> SectorPtrs = {};
		std::set<int>			RoomNumbers = {};

		// temp
		std::vector<Vector3i> Positions = {};
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
							int intersectRoomNumber = GetCollision(movPtr->Pose.Position, movPtr->RoomNumber, offset).RoomNumber;

							losInstances.push_back(LosInstanceData{ movPtr, NO_VALUE, intersectPos, intersectRoomNumber, intersectDist });
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
								int intersectRoomNumber = GetCollision(movPtr->Pose.Position, movPtr->RoomNumber, offset).RoomNumber;

								losInstances.push_back(LosInstanceData{ movPtr, i, intersectPos, intersectRoomNumber, intersectDist });
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
						int intersectRoomNumber = GetCollision(staticPtr->pos.Position, staticPtr->roomNumber, offset).RoomNumber;

						losInstances.push_back(LosInstanceData{ staticPtr, NO_VALUE, intersectPos, intersectRoomNumber, intersectDist });
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

	static std::vector<TriangleMesh> GenerateSectorTriangleMeshes(const Vector3& pos, const FloorInfo& sector, bool isFloor)
	{
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
					Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 0), corner0.z),
					Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
					Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z));

				// Calculate triangle 1.
				auto tri1 = TriangleMesh(
					Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
					Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 1), corner2.z),
					Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

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
					Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
					Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z),
					Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 0), corner3.z));

				// Calculate triangle 1.
				auto tri0 = TriangleMesh(
					Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
					Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 1), corner1.z),
					Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

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
				Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 0), corner0.z),
				Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
				Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z));

			// Calculate triangle 1.
			auto tri1 = TriangleMesh(
				Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
				Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 1), corner2.z),
				Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

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

	static void ClipSectorTrace(SectorTraceData& trace, const Ray& ray)
	{
		// Run through sectors sorted by distance.
		float closestDist = INFINITY;
		for (const auto* sectorPtr : trace.SectorPtrs)
		{
			// 1) Clip bridge.
			for (int movID : sectorPtr->BridgeItemNumbers)
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
						trace.Intersect.second = sectorPtr->RoomNumber;
					}
				}
			}

			// 2) Clip floor.
			/*auto floorTris = GenerateSectorTriangleMeshes(posList[i].ToVector3(), *sectorPtr, true);
			for (const auto& tri : floorTris)
			{
			float dist = 0.0f;
			if (tri.Intersects(ray, dist) && dist < closestDist)
			closestDist = dist;
			}

			// 3) Clip ceiling.
			auto ceilTris = GenerateSectorTriangleMeshes(posList[i].ToVector3(), *sectorPtr, false);
			for (const auto& tri : ceilTris)
			{
			float dist = 0.0f;
			if (tri.Intersects(ray, dist) && dist < closestDist)
			closestDist = dist;
			}*/
		}

		// Clip trace intersection (if applicable).
		if (closestDist != INFINITY)
			trace.Intersect = std::pair(Geometry::TranslatePoint(ray.position, ray.direction, closestDist), trace.Intersect.second);
	}

	static std::vector<Vector3i> GetSectorTracePositions()
	{
		return {};
	}

	static SectorTraceData GetSectorTrace(const Vector3& origin, const Vector3& target)
	{
		auto trace = SectorTraceData{};

		// TODO

		// Create ray.
		auto dir = target - origin;
		dir.Normalize();
		auto ray = Ray(origin, dir);

		// Clip and return trace.
		ClipSectorTrace(trace, ray);
		return trace;
	}

	RoomLosData GetRoomLos(const Vector3& origin, int originRoomNumber, const Vector3& target)
	{
		auto roomLos = RoomLosData{};

		auto trace = GetSectorTrace(origin, target);
		if (trace.IsIntersected)
			roomLos.Intersect = trace.Intersect;

		roomLos.RoomNumbers = trace.RoomNumbers;
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
