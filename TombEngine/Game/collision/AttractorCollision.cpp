#include "framework.h"
#include "Game/collision/AttractorCollision.h"

#include "Game/collision/Attractor.h"
#include "Game/collision/floordata.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Attractor
{
	AttractorCollisionData::AttractorCollisionData(Attractor& attrac, const Vector3& basePos, const EulerAngles& orient, const Vector3& probePoint) :
		Attrac(attrac)
	{
		constexpr auto HEADING_ANGLE_OFFSET			  = ANGLE(-90.0f);
		constexpr auto FACING_FORWARD_ANGLE_THRESHOLD = ANGLE(90.0f);

		// Fill proximity data.
		Proximity = GetProximity(probePoint);

		// Calculate segment orientation.
		const auto& points = Attrac.GetPoints();
		const auto& origin = points[Proximity.SegmentID];
		const auto& target = points[Proximity.SegmentID + 1];
		auto attracOrient = (points.size() == 1) ? orient : Geometry::GetOrientToPoint(origin, target);

		// Fill remaining collision data.
		HeadingAngle = attracOrient.y + HEADING_ANGLE_OFFSET;
		SlopeAngle = attracOrient.x;
		IsFacingForward = (abs(Geometry::GetShortestAngle(HeadingAngle, orient.y)) <= FACING_FORWARD_ANGLE_THRESHOLD);
		IsInFront = Geometry::IsPointInFront(basePos, Proximity.Intersection, orient);
	}

	AttractorCollisionData::ProximityData AttractorCollisionData::GetProximity(const Vector3& probePoint) const
	{
		const auto& points = Attrac.GetPoints();

		// 1 point exists; return simple proximity data.
		if (points.size() == 1)
		{
			float dist = Vector3::Distance(probePoint, points.front());
			return ProximityData{ points.front(), dist, 0.0f, 0 };
		}

		auto attracProx = ProximityData{ points.front(), INFINITY, 0.0f, 0 };
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
		return AttractorCollisionData(attrac, basePos, orient, probePoint);
	}

	// Debug
	static std::vector<Attractor*> GetDebugAttractorPtrs()
	{
		auto& player = GetLaraInfo(*LaraItem);

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
		constexpr auto SECTOR_SEARCH_DEPTH = 1;

		auto sphere = BoundingSphere(probePoint, detectRadius);
		auto nearbyAttracPtrs = std::vector<Attractor*>{};

		// Draw debug sphere.
		g_Renderer.AddDebugSphere(sphere.Center, sphere.Radius, Vector4::One, RendererDebugPage::CollisionStats);

		// TEMP
		// Get debug attractors.
		auto debugAttracPtrs = GetDebugAttractorPtrs();
		for (auto* attracPtr : debugAttracPtrs)
		{
			if (sphere.Intersects(attracPtr->GetBox()))
				nearbyAttracPtrs.push_back(attracPtr);
		}

		// TODO: Way of dealing with dynamic bridge attractors.
		// 
		// O(m * l * k) + relatively cheap arithmetic overhead:
		// (m = avg. subset room count, l = avg. subset sector count, k = avg. bridge count)
		// 1) Get room grid coords in 3x3 vicinity, derive sector IDs.
		// 2) Collect unique bridge item numbers from sectors into std::set.
		// 3) Get bridge ItemData variant BridgeObject (TODO).
		// 4) Get attractor contained in BridgeObject.
		// 
		// Bridge construction/destruction ends up simple.
		// - Initialize() generates bridge attractor.
		// - Control() updates attractor if Pose has changed.
		// - Destructor cleans up by detaching players.

		auto bridgeItemNumbers = std::set<int>{};

		// Run through neighbor rooms.
		auto& room = g_Level.Rooms[roomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			// Get room attractors.
			auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
			for (auto& attrac : neighborRoom.Attractors)
			{
				if (sphere.Intersects(attrac.GetBox()))
					nearbyAttracPtrs.push_back(&attrac);
			}

			// Get bridge item numbers.
			auto roomGridCoords = GetNeighborRoomGridCoords(Vector3i(probePoint), neighborRoomNumber, SECTOR_SEARCH_DEPTH);
			for (const auto& roomGridCoord : roomGridCoords)
			{
				const auto& sector = GetFloor(neighborRoomNumber, roomGridCoord);
				for (int bridgeItemNumber : sector.BridgeItemNumbers)
					bridgeItemNumbers.insert(bridgeItemNumber);
			}
		}

		// Get bridge attractors.
		for (int bridgeItemNumber : bridgeItemNumbers)
		{
			auto& bridgeItem = g_Level.Items[bridgeItemNumber];
			//auto& bridge = GetBridgeObject(bridgeItem);

			auto& attrac = *bridgeItem.Attractor;//bridge.Attractor;
			if (sphere.Intersects(attrac.GetBox()))
				nearbyAttracPtrs.push_back(&attrac);
		}

		// Return pointers to approximately nearby attractors from sphere-AABB tests.
		return nearbyAttracPtrs;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& probePoint, float detectRadius)
	{
		constexpr auto COLL_COUNT_MAX = 64;

		// Get pointers to approximately nearby attractors.
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
		unsigned int count = 0;
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
