#include "framework.h"
#include "Game/collision/AttractorCollision.h"

#include "Game/collision/Attractor.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Attractor
{
	static AttractorProximityData GetAttractorProximity(const Attractor& attrac, const Vector3& probePoint)
	{
		const auto& points = attrac.GetPoints();

		// 1 point exists; return simple attractor proximity data.
		if (points.size() == 1)
		{
			return AttractorProximityData
			{
				points.front(),
				Vector3::Distance(probePoint, points.front()),
				0.0f,
				0
			};
		}

		auto attracProx = AttractorProximityData{ points.front(), INFINITY, 0.0f, 0 };
		float chainDistTravelled = 0.0f;

		// Find closest point along attractor.
		for (int i = 0; i < (points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = points[i];
			const auto& target = points[i + 1];

			auto closestPoint = Geometry::GetClosestPointOnLinePerp(probePoint, origin, target);
			float dist = Vector3::Distance(probePoint, closestPoint);

			// Found new closest point; update proximity data.
			if (dist < attracProx.Distance)
			{
				chainDistTravelled += Vector3::Distance(origin, closestPoint);

				attracProx.Intersection = closestPoint;
				attracProx.Distance = dist;
				attracProx.ChainDistance += chainDistTravelled;
				attracProx.SegmentID = i;

				// Restart accumulation of distance travelled along attractor.
				chainDistTravelled = Vector3::Distance(closestPoint, target);
				continue;
			}

			// Accumulate distance travelled along attractor since last closest point.
			float segmentLength = Vector3::Distance(origin, target);
			chainDistTravelled += segmentLength;
		}

		// Return proximity data.
		return attracProx;
	}

	AttractorCollisionData GetAttractorCollision(Attractor& attrac, const Vector3& basePos, const EulerAngles& orient, const Vector3& probePoint)
	{
		constexpr auto HEADING_ANGLE_OFFSET			  = ANGLE(-90.0f);
		constexpr auto FACING_FORWARD_ANGLE_THRESHOLD = ANGLE(90.0f);

		const auto& points = attrac.GetPoints();

		// Create attractor collision.
		auto attracColl = AttractorCollisionData(attrac);

		// Fill attractor proximity.
		attracColl.Proximity = GetAttractorProximity(attrac, probePoint);

		// Calculate segment orientation.
		const auto& origin = points[attracColl.Proximity.SegmentID];
		const auto& target = points[attracColl.Proximity.SegmentID + 1];
		auto attracOrient = (points.size() == 1) ? orient : Geometry::GetOrientToPoint(origin, target);

		// Fill remaining collision data.
		attracColl.HeadingAngle = attracOrient.y + HEADING_ANGLE_OFFSET;
		attracColl.SlopeAngle = attracOrient.x;
		attracColl.IsFacingForward = (abs(Geometry::GetShortestAngle(attracColl.HeadingAngle, orient.y)) <= FACING_FORWARD_ANGLE_THRESHOLD);
		attracColl.IsInFront = Geometry::IsPointInFront(basePos, attracColl.Proximity.Intersection, orient);

		// Return attractor collision.
		return attracColl;
	}

	// Debug
	static std::vector<Attractor*> GetDebugAttractorPtrs(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto debugAttracPtrs = std::vector<Attractor*>{};
		debugAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac0);
		debugAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac1);
		debugAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac2);

		for (auto& attrac : player.Context.DebugAttracs.Attracs)
			debugAttracPtrs.push_back(&attrac);

		return debugAttracPtrs;
	}

	// TODO: Spacial partitioning may be ideal here. Would require a general collision refactor. -- Sezz 2023.07.30
	static std::vector<Attractor*> GetNearbyAttractorPtrs(const Vector3& probePoint, int roomNumber, float detectRadius)
	{
		auto sphere = BoundingSphere(probePoint, detectRadius);
		auto nearbyAttracPtrs = std::vector<Attractor*>{};

		// Get attractors in neighboring rooms.
		auto& room = g_Level.Rooms[roomNumber];
		for (int roomNumber : room.neighbors)
		{
			auto& neightborRoom = g_Level.Rooms[roomNumber];
			for (auto& attrac : neightborRoom.Attractors)
			{
				if (sphere.Intersects(attrac.GetBox()))
					nearbyAttracPtrs.push_back(&attrac);
			}
		}

		// TODO: Way of dealing with dynamic attractors.
		// Get bridge attractors.
		for (auto& [itemNumber, attrac] : g_Level.BridgeAttractors)
		{
			if (sphere.Intersects(attrac.GetBox()))
				nearbyAttracPtrs.push_back(&attrac);
		}

		// TEMP
		// Get debug attractors.
		auto debugAttracPtrs = GetDebugAttractorPtrs(*LaraItem);
		for (auto* attracPtr : debugAttracPtrs)
		{
			if (sphere.Intersects(attracPtr->GetBox()))
				nearbyAttracPtrs.push_back(attracPtr);
		}

		// Draw debug sphere.
		g_Renderer.AddDebugSphere(sphere.Center, sphere.Radius, Vector4::One, RendererDebugPage::CollisionStats);

		// Return pointers to approximately nearby attractors from sphere-AABB tests.
		return nearbyAttracPtrs;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& probePoint, float detectRadius)
	{
		constexpr auto COLL_COUNT_MAX = 64;

		// Get pointers to approximately nearby attractors from sphere-AABB tests.
		auto nearbyAttracPtrs = GetNearbyAttractorPtrs(probePoint, roomNumber, detectRadius);

		// Get attractor collisions sorted by distance.
		auto attracCollMap = std::multimap<float, AttractorCollisionData>{};
		for (auto* attracPtr : nearbyAttracPtrs)
		{
			auto attracColl = GetAttractorCollision(*attracPtr, basePos, orient, probePoint);

			// Filter out non-intersections.
			if (attracColl.Proximity.Distance > detectRadius)
				continue;

			attracCollMap.insert({ attracColl.Proximity.Distance, std::move(attracColl) });
		}

		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(std::min((int)attracCollMap.size(), COLL_COUNT_MAX));

		// Move attractor collisions from map to capped vector.
		int count = 0;
		for (auto& [dist, attracColl] : attracCollMap)
		{
			attracColls.push_back(std::move(attracColl));

			count++;
			if (count >= COLL_COUNT_MAX)
				break;
		}

		// Return attractor collisions.
		return attracColls;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& probePoint, float detectRadius)
	{
		return GetAttractorCollisions(item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation, probePoint, detectRadius);
	}
}
