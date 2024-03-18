#include "framework.h"
#include "Game/collision/Los.h"

#include "Game/collision/sphere.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Objects/game_object_ids.h"
#include "Renderer/Renderer.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Los
{
	static std::vector<ItemInfo*> GetNearbyItemPtrs(const std::set<int>& roomNumbers)
	{
		// Collect item pointers.
		auto itemPtrs = std::vector<ItemInfo*>{};
		for (int itemNumber = 0; itemNumber < g_Level.NumItems; itemNumber++)
		{
			auto& item = g_Level.Items[itemNumber];

			// 1) Check if room is active.
			const auto& room = g_Level.Rooms[item.RoomNumber];
			if (!room.Active())
				continue;

			// 2) Test if item is in nearby room.
			if (!Contains(roomNumbers, (int)item.RoomNumber))
				continue;

			itemPtrs.push_back(&item);
		}

		return itemPtrs;
	}

	static std::vector<MESH_INFO*> GetNearbyStaticPtrs(const std::set<int>& roomNumbers)
	{
		// Collect static pointers.
		auto staticPtrs = std::vector<MESH_INFO*>{};
		for (int roomNumber : roomNumbers)
		{
			// 1) Check if room is active.
			auto& room = g_Level.Rooms[roomNumber];
			if (!room.Active())
				continue;

			// 2) Run through statics in room.
			for (auto& staticObject : room.mesh)
				staticPtrs.push_back(&staticObject);
		}

		return staticPtrs;
	}

	std::vector<LosInstanceData> GetLosInstances(const Vector3& origin, int originRoomNumber, const Vector3& dir, float dist,
												 bool collideItems, bool collideStatics, bool collideSpheres)
	{
		auto losInstances = std::vector<LosInstanceData>{};

		// Calculate target.
		auto target = Geometry::TranslatePoint(origin, dir, dist);
		int targetRoomNumber = GetCollision(origin, originRoomNumber, dir, dist).RoomNumber;

		// 1) Collect room LOS instance.
		auto roomLos = GetRoomLos(origin, originRoomNumber, target, targetRoomNumber);
		if (roomLos.Intersect.has_value())
		{
			target = roomLos.Intersect->first;
			targetRoomNumber = roomLos.Intersect->second;
			dist = Vector3::Distance(origin, target);

			losInstances.push_back(LosInstanceData{ {}, NO_VALUE, roomLos.Intersect->first, roomLos.Intersect->second, dist });
		}

		if (collideItems || collideSpheres)
		{
			auto itemPtrs = GetNearbyItemPtrs(roomLos.RoomNumbers);
			for (auto* itemPtr : itemPtrs)
			{
				// 2) Collect item LOS instances.
				if (collideItems)
				{
					auto box = GameBoundingBox(itemPtr).ToBoundingOrientedBox(itemPtr->Pose);

					float intersectDist = 0.0f;
					if (box.Intersects(origin, dir, intersectDist))
					{
						if (intersectDist <= dist)
							losInstances.push_back(LosInstanceData{ itemPtr, NO_VALUE, Geometry::TranslatePoint(origin, dir, intersectDist), itemPtr->RoomNumber, intersectDist });
					}
				}

				// 3) Collect item sphere LOS instances.
				if (collideSpheres)
				{
					int sphereCount = GetSpheres(itemPtr, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
					for (int i = 0; i < sphereCount; i++)
					{
						auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);

						float intersectDist = 0.0f;
						if (sphere.Intersects(origin, dir, intersectDist))
						{
							if (intersectDist <= dist)
								losInstances.push_back(LosInstanceData{ itemPtr, i, Geometry::TranslatePoint(origin, dir, intersectDist), itemPtr->RoomNumber, intersectDist });
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
						losInstances.push_back(LosInstanceData{ staticPtr, NO_VALUE, Geometry::TranslatePoint(origin, dir, intersectDist), staticPtr->roomNumber, intersectDist });
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

	// TODO: Accurate room LOS. For now, it simply wraps legacy functions.
	RoomLosData GetRoomLos(const Vector3& origin, int originRoomNumber, const Vector3& target, int targetRoomNumber)
	{
		auto losOrigin = GameVector(origin, originRoomNumber);
		auto losTarget = GameVector(target, targetRoomNumber);

		float dist = 0.0f;
		auto roomNumbers = std::set<int>{};

		// 1) Collide axis-aligned walls.
		if (!LOS(&losOrigin, &losTarget, &roomNumbers))
			dist = Vector3::Distance(origin, losTarget.ToVector3());

		// 2) Collide diagonal walls and floors/ceilings.
		if (!LOSAndReturnTarget(&losOrigin, &losTarget, 0))
			dist = Vector3::Distance(origin, losTarget.ToVector3());

		// Calculate intersection (if applicable).
		auto intersect = std::optional<std::pair<Vector3, int>>();
		if (dist != 0.0f)
		{
			auto dir = target - origin;
			dir.Normalize();

			auto closestIntersect = Geometry::TranslatePoint(origin, dir, dist);
			int intersectRoomNumber = losTarget.RoomNumber;
			intersect = std::pair(closestIntersect, intersectRoomNumber);
		}

		// Return room LOS.
		return RoomLosData{ intersect, roomNumbers };
	}

	std::optional<std::pair<Vector3, int>> GetItemLosIntersect(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool ignorePlayer)
	{
		auto losInstances = GetLosInstances(origin, roomNumber, dir, dist, true, false);
		for (const auto& losInstance : losInstances)
		{
			// 1) FAILSAFE: Ignore sphere.
			if (losInstance.SphereID != NO_VALUE)
				continue;

			// 2) Check for object.
			if (!losInstance.ObjectPtr.has_value())
				continue;

			// 3) Check if object is item.
			if (!std::holds_alternative<ItemInfo*>(*losInstance.ObjectPtr))
				continue;

			// 4) Check if item is not player (if applicable).
			if (ignorePlayer)
			{
				const auto& item = std::get<ItemInfo*>(*losInstance.ObjectPtr);
				if (item->ObjectNumber == ID_LARA)
					continue;
			}

			return std::pair(losInstance.Position, losInstance.RoomNumber);
		}

		return std::nullopt;
	}

	std::optional<ItemSphereLosData> GetItemSphereLosIntersect(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool ignorePlayer)
	{
		auto losInstances = GetLosInstances(origin, roomNumber, dir, dist, false, false, true);
		for (const auto& losInstance : losInstances)
		{
			// 1) Check for sphere.
			if (losInstance.SphereID == NO_VALUE)
				continue;

			// 2) Check for object.
			if (!losInstance.ObjectPtr.has_value())
				continue;

			// 3) Check if object is item.
			if (!std::holds_alternative<ItemInfo*>(*losInstance.ObjectPtr))
				continue;

			// 4) Check if item is not player (if applicable).
			if (ignorePlayer)
			{
				const auto& item = *std::get<ItemInfo*>(*losInstance.ObjectPtr);
				if (item.ObjectNumber == ID_LARA)
					continue;
			}

			return ItemSphereLosData{ std::pair(losInstance.Position, losInstance.RoomNumber), losInstance.SphereID };
		}

		return std::nullopt;
	}

	std::optional<std::pair<Vector3, int>> GetStaticLosIntersect(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool onlySolid)
	{
		auto losInstances = GetLosInstances(origin, roomNumber, dir, dist, false);
		for (const auto& losInstance : losInstances)
		{
			// 1) FAILSAFE: Ignore sphere.
			if (losInstance.SphereID != NO_VALUE)
				continue;

			// 2) Check for object.
			if (!losInstance.ObjectPtr.has_value())
				continue;

			// 3) Check if object is static.
			if (!std::holds_alternative<MESH_INFO*>(*losInstance.ObjectPtr))
				continue;

			// 4) Check if static is solid (if applicable).
			const auto& staticObject = *std::get<MESH_INFO*>(*losInstance.ObjectPtr);
			if (onlySolid && !(staticObject.flags & StaticMeshFlags::SM_SOLID))
				continue;

			return std::pair(losInstance.Position, losInstance.RoomNumber);
		}

		return std::nullopt;
	}

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos)
	{
		auto pos = g_Renderer.GetRay(screenPos);

		auto origin = GameVector(pos.first, Camera.RoomNumber);
		auto target = GameVector(pos.second, Camera.RoomNumber);

		LOS(&origin, &target);
		return std::pair<GameVector, GameVector>(origin, target);
	}
}
