#include "framework.h"
#include "Game/collision/Los.h"

#include "Game/collision/Point.h"
#include "Game/collision/sphere.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Objects/game_object_ids.h"
#include "Math/Math.h"
#include "Physics/Physics.h"
#include "Renderer/Renderer.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Physics;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Los
{
	struct SectorTraceData
	{
		struct InterceptData
		{
			const FloorInfo* Sector	  = nullptr;
			float			 Distance = 0.0f;
		};

		std::vector<InterceptData> Intercepts = {};

		void Sort()
		{
			std::sort(
				Intercepts.begin(), Intercepts.end(),
				[](const InterceptData& intercept0, const InterceptData& intercept1)
				{
					return (intercept0.Distance < intercept1.Distance);
				});
		}
	};

	struct RoomTraceData
	{
		const CollisionTriangle* Triangle	 = nullptr;
		Vector3					 Position	 = Vector3::Zero;
		int						 RoomNumber	 = 0;
		std::vector<int>		 RoomNumbers = {};

		bool IsIntersected = false;
	};

	static std::vector<ItemInfo*> GetNearbyMoveables(const std::vector<int>& roomNumbers)
	{
		// Collect moveables.
		auto movs = std::vector<ItemInfo*>{};
		for (int movID = 0; movID < g_Level.NumItems; movID++)
		{
			auto& mov = g_Level.Items[movID];

			// 1) Check moveable status.
			if (mov.Status == ItemStatus::ITEM_INVISIBLE || mov.Status == ItemStatus::ITEM_DEACTIVATED)
				continue;

			// TODO: Must check roomNumbers vector.
			
			// 2) Check if room is active.
			const auto& room = g_Level.Rooms[mov.RoomNumber];
			if (!room.Active())
				continue;

			// 3) Test if moveable is in nearby room.
			if (!Contains(room.neighbors, (int)mov.RoomNumber))
				continue;

			movs.push_back(&mov);
		}

		return movs;
	}

	static std::vector<MESH_INFO*> GetNearbyStatics(const std::vector<int>& roomNumbers)
	{
		// Collect statics.
		auto statics = std::vector<MESH_INFO*>{};
		for (int roomNumber : roomNumbers)
		{
			// Run through neighbor rooms.
			const auto& room = g_Level.Rooms[roomNumber];
			for (auto& neighborRoomNumber : room.neighbors)
			{
				// 1) Check if room is active.
				auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
				if (!neighborRoom.Active())
					continue;

				// Run through statics.
				for (auto& staticObj : neighborRoom.mesh)
				{
					// 2) Check if static is visible.
					if (!(staticObj.flags & StaticMeshFlags::SM_VISIBLE))
						continue;

					statics.push_back(&staticObj);
				}
			}
		}

		return statics;
	}
	
	LosCollisionData GetLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
								 bool collideMoveables, bool collideSpheres, bool collideStatics)
	{
		// FAILSAFE.
		if (dir == Vector3::Zero)
		{
			TENLog("GetLos(): dir is not a unit vector.", LogLevel::Warning);
			return LosCollisionData{ RoomLosCollisionData{ nullptr, origin, roomNumber, {}, false, 0.0f }, {}, {}, {} };
		}

		auto losColl = LosCollisionData{};

		// 1) Collect room LOS collision.
		losColl.Room = GetRoomLosCollision(origin, roomNumber, dir, dist);
		dist = losColl.Room.Distance;

		if (collideMoveables || collideSpheres)
		{
			auto movs = GetNearbyMoveables(losColl.Room.RoomNumbers);
			for (auto* mov : movs)
			{
				// Check if moveable is collidable.
				if (!mov->Collidable)
					continue;

				// Check moveable status.
				if (mov->Status == ItemStatus::ITEM_INVISIBLE || mov->Status == ItemStatus::ITEM_DEACTIVATED)
					continue;

				// Ignore bridges (handled as part of room).
				if (mov->IsBridge())
					continue;

				// 2) Collect moveable LOS collisions.
				if (collideMoveables)
				{
					auto box = mov->GetObb();

					float intersectDist = 0.0f;
					if (box.Intersects(origin, dir, intersectDist) && intersectDist <= dist)
					{
						auto pos = Geometry::TranslatePoint(origin, dir, intersectDist);
						auto offset = pos - mov->Pose.Position.ToVector3();
						int roomNumber = GetPointCollision(mov->Pose.Position, mov->RoomNumber, offset).GetRoomNumber();

						auto movLosColl = MoveableLosCollisionData{};
						movLosColl.Moveable = mov;
						movLosColl.Position = pos;
						movLosColl.RoomNumber = roomNumber;
						movLosColl.IsOriginContained = (bool)box.Contains(origin);
						movLosColl.Distance = intersectDist;
						losColl.Moveables.push_back(movLosColl);
					}
				}

				// 3) Collect moveable sphere LOS collisions.
				if (collideSpheres)
				{
					int sphereCount = GetSpheres(mov, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
					for (int i = 0; i < sphereCount; i++)
					{
						auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);

						float intersectDist = 0.0f;
						if (sphere.Intersects(origin, dir, intersectDist) && intersectDist <= dist)
						{
							auto pos = Geometry::TranslatePoint(origin, dir, intersectDist);
							auto offset = pos - mov->Pose.Position.ToVector3();
							int roomNumber = GetPointCollision(mov->Pose.Position, mov->RoomNumber, offset).GetRoomNumber();

							auto sphereLosColl = SphereLosCollisionData{};
							sphereLosColl.Moveable = mov;
							sphereLosColl.SphereID = i;
							sphereLosColl.Position = pos;
							sphereLosColl.RoomNumber = roomNumber;
							sphereLosColl.IsOriginContained = (bool)sphere.Contains(origin);
							sphereLosColl.Distance = intersectDist;
							losColl.Spheres.push_back(sphereLosColl);
						}
					}
				}
			}

			// Sort moveable LOS collisions.
			std::sort(
				losColl.Moveables.begin(), losColl.Moveables.end(),
				[](const auto& movLos0, const auto& movLos1)
				{
					return (movLos0.Distance < movLos1.Distance);
				});

			// Sort moveable sphere LOS collisions.
			std::sort(
				losColl.Spheres.begin(), losColl.Spheres.end(),
				[](const auto& sphereLos0, const auto& sphereLos1)
				{
					return (sphereLos0.Distance < sphereLos1.Distance);
				});
		}

		// 4) Collect static LOS collisions.
		if (collideStatics)
		{
			auto statics = GetNearbyStatics(losColl.Room.RoomNumbers);
			for (auto* staticObj : statics)
			{
				// Check static visibility.
				if (!(staticObj->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				auto box = staticObj->GetBox();

				float intersectDist = 0.0f;
				if (box.Intersects(origin, dir, intersectDist) && intersectDist <= dist)
				{
					auto pos = Geometry::TranslatePoint(origin, dir, intersectDist);
					auto offset = pos - staticObj->pos.Position.ToVector3();
					int roomNumber = GetPointCollision(staticObj->pos.Position, staticObj->roomNumber, offset).GetRoomNumber();

					auto staticLosColl = StaticLosCollisionData{};
					staticLosColl.Static = staticObj;
					staticLosColl.Position = pos;
					staticLosColl.RoomNumber = roomNumber;
					staticLosColl.IsOriginContained = (bool)box.Contains(origin);
					staticLosColl.Distance = intersectDist;
					losColl.Statics.push_back(staticLosColl);
				}
			}

			// Sort static LOS collisions.
			std::sort(
				losColl.Statics.begin(), losColl.Statics.end(),
				[](const auto& staticLos0, const auto& staticLos1)
				{
					return (staticLos0.Distance < staticLos1.Distance);
				});
		}

		// 5) Return sorted LOS collision data.
		return losColl;
	}
	
	static SectorTraceData GetSectorTrace(const Ray& ray, int roomNumber, float dist)
	{
		auto sectorTrace = SectorTraceData{};

		// 1) Collect sector intercepts.
		float sectorSearchDepth = ceil(dist / BLOCK(1));
		auto sectors = GetNeighborSectors(ray.position, roomNumber, sectorSearchDepth, false);
		for (const auto& sector : sectors)
		{
			float intersectDist = 0.0f;
			if (ray.Intersects(sector->Aabb, intersectDist) && intersectDist <= dist)
				sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ sector, intersectDist });
		}

		// 2) Sort intercepts.
		sectorTrace.Sort();

		// 3) Return sector trace.
		return sectorTrace;
	}

	static RoomTraceData GetRoomTrace(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges)
	{
		auto roomTrace = RoomTraceData{};

		// 1) Initialize ray.
		auto ray = Ray(origin, dir);
		int rayRoomNumber = roomNumber;
		float rayDist = dist;

		// 2) Fill room trace.
		while (true)
		{
			const CollisionTriangle* closestTri = nullptr;
			float closestDist = rayDist;

			// 2.1) Clip room.
			const auto& room = g_Level.Rooms[rayRoomNumber];
			auto meshColl = room.CollisionMesh.GetCollision(ray, closestDist);
			if (meshColl.has_value())
			{
				closestTri = &meshColl->Triangle;
				closestDist = meshColl->Distance;
			}

			// 2.2) Clip bridge (if applicable).
			if (collideBridges)
			{
				auto visitedBridgeMovIds = std::set<int>{};
				bool hasBridge = false;

				// Run through intercepts in sector trace.
				auto sectorTrace = GetSectorTrace(ray, rayRoomNumber, closestDist);
				for (const auto& intercept : sectorTrace.Intercepts)
				{
					// Run through bridges in sector.
					for (int bridgeMovID : intercept.Sector->BridgeItemNumbers)
					{
						// Check if bridge was already visited.
						if (Contains(visitedBridgeMovIds, bridgeMovID))
							continue;
						visitedBridgeMovIds.insert(bridgeMovID);

						const auto& bridgeMov = g_Level.Items[bridgeMovID];
						const auto& bridge = GetBridgeObject(bridgeMov);

						// Check bridge status.
						if (bridgeMov.Status == ItemStatus::ITEM_INVISIBLE || bridgeMov.Status == ItemStatus::ITEM_DEACTIVATED)
							continue;

						// Clip bridge.
						auto meshColl = bridge.GetCollisionMesh().GetCollision(ray, dist);
						if (meshColl.has_value() && meshColl->Distance < closestDist)
						{
							closestTri = &meshColl->Triangle;
							closestDist = meshColl->Distance;
							hasBridge = true;
						}
					}

					if (hasBridge)
						break;
				}
			}

			// 2.4) Collect room number.
			roomTrace.RoomNumbers.push_back(rayRoomNumber);

			// 2.3) Return room trace or retrace new room from portal.
			if (closestTri != nullptr)
			{
				auto intersectPos = Geometry::TranslatePoint(ray.position, ray.direction, closestDist);

				// Room portal triangle; update ray to retrace from new room.
				if (closestTri->IsPortal() &&
					rayRoomNumber != closestTri->GetPortalRoomNumber()) // FAILSAFE: Prevent infinite loop.
				{
					ray.position = intersectPos;
					rayRoomNumber = closestTri->GetPortalRoomNumber();
					rayDist -= closestDist;
				}
				// Tangible triangle; collect remaining room trace data.
				else
				{
					if (closestTri->IsPortal())
						TENLog("GetRoomTrace(): Room portal cannot link back to itself.", LogLevel::Warning);

					roomTrace.Triangle = closestTri;
					roomTrace.Position = intersectPos;
					roomTrace.RoomNumber = rayRoomNumber;
					roomTrace.IsIntersected = true;
					return roomTrace;
				}
			}
			else
			{
				roomTrace.Triangle = nullptr;
				roomTrace.Position = Geometry::TranslatePoint(ray.position, ray.direction, rayDist);
				roomTrace.RoomNumber = rayRoomNumber;
				roomTrace.IsIntersected = false;
				return roomTrace;
			}
		}

		// FAILSAFE.
		return roomTrace;
	}

	RoomLosCollisionData GetRoomLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges)
	{
		// FAILSAFE.
		if (dir == Vector3::Zero)
		{
			TENLog("GetRoomLos(): Direction is not a unit vector.", LogLevel::Warning);
			return RoomLosCollisionData{ {}, origin, roomNumber, {}, false, 0.0f };
		}

		// Get room trace.
		auto roomTrace = GetRoomTrace(origin, roomNumber, dir, dist, collideBridges);

		// Calculate position data.
		float losDist = roomTrace.IsIntersected ? Vector3::Distance(origin, roomTrace.Position) : dist;
		auto losPos = Geometry::TranslatePoint(origin, dir, losDist);
		int losRoomNumber = roomTrace.RoomNumber;

		// Create and return room LOS collision.
		auto roomLosColl = RoomLosCollisionData{};
		roomLosColl.Triangle = roomTrace.Triangle;
		roomLosColl.Position = losPos;
		roomLosColl.RoomNumber = losRoomNumber;
		roomLosColl.RoomNumbers = roomTrace.RoomNumbers;
		roomLosColl.IsIntersected = roomTrace.IsIntersected;
		roomLosColl.Distance = losDist;
		return roomLosColl;
	}

	std::optional<MoveableLosCollisionData> GetMoveableLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer)
	{
		auto losColl = GetLosCollision(origin, roomNumber, dir, dist, true, false, false);
		for (auto& movLos : losColl.Moveables)
		{
			// Check if moveable is not player (if applicable).
			if (!collidePlayer && movLos.Moveable->IsLara())
				continue;

			return movLos;
		}

		return std::nullopt;
	}

	std::optional<SphereLosCollisionData> GetSphereLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer)
	{
		auto losColl = GetLosCollision(origin, roomNumber, dir, dist, false, true, false);
		for (auto& sphereLos : losColl.Spheres)
		{
			// Check if moveable is not player (if applicable).
			if (!collidePlayer && sphereLos.Moveable->IsLara())
				continue;

			return sphereLos;
		}

		return std::nullopt;
	}

	std::optional<StaticLosCollisionData> GetStaticLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid)
	{
		auto losColl = GetLosCollision(origin, roomNumber, dir, dist, false, false, true);
		for (auto& staticLos : losColl.Statics)
		{
			// Check if static is solid (if applicable).
			if (collideOnlySolid && !(staticLos.Static->flags & StaticMeshFlags::SM_SOLID))
				continue;

			return staticLos;
		}

		return std::nullopt;
	}

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos)
	{
		auto posPair = g_Renderer.GetRay(screenPos);

		auto dir = posPair.second - posPair.first;
		dir.Normalize();
		float dist = Vector3::Distance(posPair.first, posPair.second);

		auto roomLosColl = GetRoomLosCollision(posPair.first, Camera.RoomNumber, dir, dist);

		auto origin = GameVector(posPair.first, Camera.RoomNumber);
		auto target = GameVector(roomLosColl.Position, roomLosColl.RoomNumber);
		return std::pair(origin, target);
	}
}
