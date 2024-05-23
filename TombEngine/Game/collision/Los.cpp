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
			Vector3i		 Position = Vector3i::Zero;
		};

		std::vector<InterceptData> Intercepts = {};
	};

	struct RoomTraceData
	{
		std::optional<const CollisionTriangle*> Triangle	= std::nullopt;
		std::pair<Vector3, int>					Position	= {};
		std::vector<int>						RoomNumbers = {};

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
	
	LosData GetLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
				   bool collideMoveables, bool collideSpheres, bool collideStatics)
	{
		// FAILSAFE.
		if (dir == Vector3::Zero)
		{
			TENLog("GetLos(): dir is not a unit vector.", LogLevel::Warning);
			return LosData{ RoomLosData{ std::nullopt, std::pair(origin, roomNumber), {}, false, 0.0f }, {}, {}, {} };
		}

		auto los = LosData{};

		// 1) Collect room LOS instance.
		los.Room = GetRoomLos(origin, roomNumber, dir, dist);
		dist = los.Room.Distance;

		if (collideMoveables || collideSpheres)
		{
			auto movs = GetNearbyMoveables(los.Room.RoomNumbers);
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

				// 2) Collect moveable LOS instances.
				if (collideMoveables)
				{
					auto box = mov->GetBox();

					float intersectDist = 0.0f;
					if (box.Intersects(origin, dir, intersectDist) && intersectDist <= dist)
					{
						auto pos = Geometry::TranslatePoint(origin, dir, intersectDist);
						auto offset = pos - mov->Pose.Position.ToVector3();
						int roomNumber = GetPointCollision(mov->Pose.Position, mov->RoomNumber, offset).GetRoomNumber();

						auto movLos = MoveableLosData{};
						movLos.Moveable = mov;
						movLos.Position = std::pair(pos, roomNumber);
						movLos.IsOriginContained = (bool)box.Contains(origin);
						movLos.Distance = intersectDist;
						los.Moveables.push_back(movLos);
					}
				}

				// 3) Collect moveable sphere LOS instances.
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

							auto sphereLos = SphereLosData{};
							sphereLos.Moveable = mov;
							sphereLos.SphereID = i;
							sphereLos.Position = std::pair(pos, roomNumber);
							sphereLos.IsOriginContained = (bool)sphere.Contains(origin);
							sphereLos.Distance = intersectDist;
							los.Spheres.push_back(sphereLos);
						}
					}
				}
			}

			// Sort moveable LOS instances.
			std::sort(
				los.Moveables.begin(), los.Moveables.end(),
				[](const auto& movLos0, const auto& movLos1)
				{
					return (movLos0.Distance < movLos1.Distance);
				});

			// Sort moveable sphere LOS instances.
			std::sort(
				los.Spheres.begin(), los.Spheres.end(),
				[](const auto& sphereLos0, const auto& sphereLos1)
				{
					return (sphereLos0.Distance < sphereLos1.Distance);
				});
		}

		// 4) Collect static LOS instances.
		if (collideStatics)
		{
			auto statics = GetNearbyStatics(los.Room.RoomNumbers);
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

					auto staticLos = StaticLosData{};
					staticLos.Static = staticObj;
					staticLos.Position = std::pair(pos, roomNumber);
					staticLos.IsOriginContained = (bool)box.Contains(origin);
					staticLos.Distance = intersectDist;
					los.Statics.push_back(staticLos);
				}
			}

			// Sort static LOS instances.
			std::sort(
				los.Statics.begin(), los.Statics.end(),
				[](const auto& staticLos0, const auto& staticLos1)
				{
					return (staticLos0.Distance < staticLos1.Distance);
				});
		}

		// 5) Return sorted LOS.
		return los;
	}
	
	static SectorTraceData GetSectorTrace(const Vector3& origin, int roomNumber, const Vector3& target)
	{
		auto sectorTrace = SectorTraceData{};

		auto deltaPos = target - origin;

		// 1) Collect origin as intercept.
		sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, origin });

		// 2) Collect X axis intercepts.
		if (deltaPos.x != 0.0f)
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
			pos.x = isNegative ? ((int)round(origin.x) & (UINT_MAX - WALL_MASK)) : ((int)round(origin.x) | WALL_MASK);
			pos.y = (((pos.x - origin.x) * step.y) / BLOCK(1)) + origin.y;
			pos.z = (((pos.x - origin.x) * step.z) / BLOCK(1)) + origin.z;

			// Collect intercept positions.
			while (isNegative ? (pos.x > target.x) : (pos.x < target.x))
			{
				sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos });
				sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos + Vector3i(sign, 0, 0) });

				pos += Vector3i(BLOCK(1), step.y, step.z) * sign;
			}
		}

		// 3) Collect Z axis intercepts.
		if (deltaPos.z != 0.0f)
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
			pos.z = isNegative ? ((int)round(origin.z) & (UINT_MAX - WALL_MASK)) : ((int)round(origin.z) | WALL_MASK);
			pos.x = (((pos.z - origin.z) * step.x) / BLOCK(1)) + origin.x;
			pos.y = (((pos.z - origin.z) * step.y) / BLOCK(1)) + origin.y;

			// Collect positions.
			while (isNegative ? (pos.z > target.z) : (pos.z < target.z))
			{
				sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos });
				sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, pos + Vector3i(0, 0, sign) });

				pos += Vector3i(step.x, step.y, BLOCK(1)) * sign;
			}
		}

		// 4) Collect target as intercept.
		sectorTrace.Intercepts.push_back(SectorTraceData::InterceptData{ nullptr, target });

		// 5) Sort intercepts by distance from origin.
		std::sort(
			sectorTrace.Intercepts.begin(), sectorTrace.Intercepts.end(),
			[origin](const SectorTraceData::InterceptData& intercept0, const SectorTraceData::InterceptData& intercept1)
			{
				float distSqr0 = Vector3i::DistanceSquared(origin, intercept0.Position);
				float distSqr1 = Vector3i::DistanceSquared(origin, intercept1.Position);
				return (distSqr0 < distSqr1);
			});

		// 6) Clean intercepts.
		for (int i = 0; i < sectorTrace.Intercepts.size();)
		{
			auto& intercept = sectorTrace.Intercepts[i];

			auto pointColl = GetPointCollision(intercept.Position, roomNumber);
			intercept.Sector = &pointColl.GetSector();

			// 6.1) Trim intercepts beyond current room.
			if (intercept.Sector->RoomNumber != roomNumber)
			{
				sectorTrace.Intercepts.erase(sectorTrace.Intercepts.begin() + i, sectorTrace.Intercepts.end());
				break;
			}

			// TODO: Pass bound to this function so I don't have to do this.
			// 6.2) Remove intercepts of duplicate sectors.
			if (i > 0 && intercept.Sector == sectorTrace.Intercepts[i - 1].Sector)
			{
				sectorTrace.Intercepts.erase(sectorTrace.Intercepts.begin() + i);
				continue;
			}

			i++;
		}

		// 7) Return sector trace.
		return sectorTrace;
	}

	static RoomTraceData GetRoomTrace(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges)
	{
		auto roomTrace = RoomTraceData{};

		// 1) Set base data.
		auto ray = Ray(origin, dir);
		int rayRoomNumber = roomNumber;
		float rayDist = dist;
		auto target = Geometry::TranslatePoint(origin, dir, dist);

		// 2) Find room trace.
		bool contTrace = true;
		while (contTrace)
		{
			// 2.1) Collect room number.
			roomTrace.RoomNumbers.push_back(rayRoomNumber);

			const CollisionTriangle* closestTri = nullptr;
			float closestDist = rayDist;

			// 2.2) Clip room.
			const auto& room = g_Level.Rooms[rayRoomNumber];
			auto meshColl = room.CollisionMesh.GetCollision(ray, closestDist);
			if (meshColl.has_value())
			{
				closestTri = &meshColl->Triangle;
				closestDist = meshColl->Distance;
			}

			// 2.3) Clip bridges (if applicable).
			if (collideBridges)
			{
				auto sectorTrace = GetSectorTrace(ray.position, rayRoomNumber, target);

				// Run through intercepts in trace.
				auto visitedBridgeMovIds = std::set<int>{};
				for (int i = 0; i < sectorTrace.Intercepts.size(); i++)
				{
					// Run through bridges in sector.
					const auto& intercept = sectorTrace.Intercepts[i];
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
							contTrace = false;
						}
					}

					if (!contTrace)
						break;
				}
			}

			// 3) Set room trace or continue from new room.
			if (closestTri != nullptr)
			{
				auto intersectPos = Geometry::TranslatePoint(ray.position, ray.direction, closestDist);

				// Triangle is room portal; continue trace from new room.
				if (closestTri->IsPortal())
				{
					ray.position = intersectPos;
					rayRoomNumber = closestTri->GetPortalRoomNumber();
					rayDist = Vector3::Distance(intersectPos, target);
				}
				// Triangle is tangible; fill remaining room trace data.
				else
				{
					roomTrace.Triangle = closestTri;
					roomTrace.Position = std::pair(intersectPos, rayRoomNumber);
					roomTrace.IsIntersected = true;
					contTrace = false;
				}
			}
			else
			{
				roomTrace.Position = std::pair(target, rayRoomNumber);
				contTrace = false;
			}
		}

		// 4) Return room trace.
		return roomTrace;
	}

	RoomLosData GetRoomLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges)
	{
		// FAILSAFE.
		if (dir == Vector3::Zero)
		{
			TENLog("GetRoomLos(): dir is not a unit vector.", LogLevel::Warning);
			return RoomLosData{ {}, std::pair(origin, roomNumber), {}, false, 0.0f };
		}

		// Get room trace.
		auto roomTrace = GetRoomTrace(origin, roomNumber, dir, dist, collideBridges);

		// Calculate position data.
		float losDist = roomTrace.IsIntersected ? Vector3::Distance(origin, roomTrace.Position.first) : dist;
		auto losPos = Geometry::TranslatePoint(origin, dir, losDist);
		int losRoomNumber = roomTrace.Position.second;

		// Create and return room LOS.
		auto roomLos = RoomLosData{};
		roomLos.Triangle = roomTrace.Triangle;
		roomLos.Position = std::pair(losPos, losRoomNumber);
		roomLos.RoomNumbers = roomTrace.RoomNumbers;
		roomLos.IsIntersected = roomTrace.IsIntersected;
		roomLos.Distance = losDist;
		return roomLos;
	}

	std::optional<MoveableLosData> GetMoveableLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer)
	{
		auto los = GetLos(origin, roomNumber, dir, dist, true, false, false);
		for (auto& movLos : los.Moveables)
		{
			// Check if moveable is not player (if applicable).
			if (!collidePlayer && movLos.Moveable->IsLara())
				continue;

			return movLos;
		}

		return std::nullopt;
	}

	std::optional<SphereLosData> GetSphereLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer)
	{
		auto los = GetLos(origin, roomNumber, dir, dist, false, true, false);
		for (auto& sphereLos : los.Spheres)
		{
			// Check if moveable is not player (if applicable).
			if (!collidePlayer && sphereLos.Moveable->IsLara())
				continue;

			return sphereLos;
		}

		return std::nullopt;
	}

	std::optional<StaticLosData> GetStaticLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid)
	{
		auto los = GetLos(origin, roomNumber, dir, dist, false, false, true);
		for (auto& staticLos : los.Statics)
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

		auto roomLos = GetRoomLos(posPair.first, Camera.RoomNumber, dir, dist);

		auto origin = GameVector(posPair.first, Camera.RoomNumber);
		auto target = GameVector(roomLos.Position.first, roomLos.Position.second);
		return std::pair(origin, target);
	}
}
