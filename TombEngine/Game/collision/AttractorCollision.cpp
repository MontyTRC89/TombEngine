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
	AttractorCollisionData::AttractorCollisionData(Attractor& attrac, const Vector3& pos, short headingAngle)
	{
		auto refOrient = EulerAngles(0, headingAngle, 0);

		// Set attractor pointer and fill proximity data.
		AttracPtr = &attrac;
		Proximity = GetProximity(pos, refOrient);

		// Calculate segment orientation.
		const auto& points = AttracPtr->GetPoints();
		auto segmentOrient = (points.size() == 1) ?
			refOrient :
			Geometry::GetOrientToPoint(points[Proximity.SegmentID], points[Proximity.SegmentID + 1]);

		// Fill remaining collision data.
		HeadingAngle = segmentOrient.y + HEADING_ANGLE_OFFSET;
		SlopeAngle = segmentOrient.x;
		IsFacingForward = (abs(Geometry::GetShortestAngle(HeadingAngle, refOrient.y)) <= FACING_FORWARD_ANGLE_THRESHOLD);
		IsInFront = Geometry::IsPointInFront(pos, Proximity.Intersection, refOrient);
	}

	AttractorCollisionData::ProximityData AttractorCollisionData::GetProximity(const Vector3& pos, const EulerAngles& refOrient) const
	{
		// NOTE: Weights sum must be 1.
		constexpr auto WEIGHT_2D_DIST	  = 0.3f;
		constexpr auto WEIGHT_3D_DIST	  = 0.2f;
		constexpr auto WEIGHT_DELTA_ANGLE = 0.4f;

		const auto& points = AttracPtr->GetPoints();

		// Single point exists; return simple proximity data.
		if (points.size() == 1)
		{
			float dist2D = Vector2::Distance(Vector2(pos.x, pos.z), Vector2(points.front().x, points.front().z));
			float dist3D = Vector3::Distance(pos, points.front());
			return ProximityData{ points.front(), dist2D, dist3D, 0.0f, 0 };
		}

		auto attracProx = ProximityData{ points.front(), INFINITY, INFINITY, 0.0f, 0 };
		float bestWeightScore = INFINITY;
		float chainDistTraveled = 0.0f;

		short deltaAngle2 = 0;

		// Find closest intersection along attractor.
		for (int i = 0; i < (points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = points[i];
			const auto& target = points[i + 1];

			// Calculate Y-perpendicular intersection and 2D distance.
			auto intersect = Geometry::GetClosestPointOnLinePerp(pos, origin, target);
			float dist2DSqr = Vector2::DistanceSquared(Vector2(pos.x, pos.z), Vector2(intersect.x, intersect.z));
			float dist3DSqr = Vector3::DistanceSquared(pos, intersect);

			// TODO: Reverse
			// Calculate delta angle.
			auto segmentOrient = Geometry::GetOrientToPoint(origin, target);
			short deltaAngle0 = abs(Geometry::GetShortestAngle(refOrient.y, segmentOrient.y + HEADING_ANGLE_OFFSET));
			short deltaAngle1 = abs(Geometry::GetShortestAngle(refOrient.y, segmentOrient.y + HEADING_ANGLE_OFFSET + ANGLE(180.0f)));
			
			// Calculate criterion weight score.
			float weightScore = (dist2DSqr * WEIGHT_2D_DIST) +
								(dist3DSqr * WEIGHT_3D_DIST) +
								std::min((deltaAngle0 * WEIGHT_DELTA_ANGLE), (deltaAngle1 * WEIGHT_DELTA_ANGLE));

			// Found new best intersection by criterion weight score; update proximity data.
			if (weightScore < bestWeightScore)
			{
				bestWeightScore = weightScore;
				chainDistTraveled += Vector3::Distance(origin, intersect);

				attracProx.Intersection = intersect;
				attracProx.Distance2D = dist2DSqr;
				attracProx.Distance3D = dist3DSqr;
				attracProx.ChainDistance += chainDistTraveled;
				attracProx.SegmentID = i;

				// Closest possible intersection found; exit loop.
				if (attracProx.Distance2D == 0.0f && attracProx.Distance3D == 0.0f)
					break;

				// Restart accumulation of distance traveled along attractor.
				chainDistTraveled = Vector3::Distance(intersect, target);
				continue;
			}

			// Accumulate distance traveled along attractor since last closest point.
			float segmentLength = Vector3::Distance(origin, target);
			chainDistTraveled += segmentLength;
		}

		// Root final distances.
		attracProx.Distance2D = sqrt(attracProx.Distance2D);
		attracProx.Distance3D = sqrt(attracProx.Distance3D);

		// Return proximity data.
		return attracProx;
	}

	AttractorCollisionData GetAttractorCollision(Attractor& attrac, const Vector3& pos, short headingAngle)
	{
		return AttractorCollisionData(attrac, pos, headingAngle);
	}

	// Debug
	static std::vector<Attractor*> GetDebugAttractorPtrs()
	{
		auto& player = GetLaraInfo(*LaraItem);

		auto debugAttracPtrs = std::vector<Attractor*>{};
		debugAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac0);
		debugAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac1);
		debugAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac2);

		return debugAttracPtrs;
	}

	// TODO: Spacial partitioning may be ideal here. Would require a general collision refactor. -- Sezz 2023.07.30
	static std::vector<Attractor*> GetNearbyAttractorPtrs(const Vector3& pos, int roomNumber, float detectRadius)
	{
		constexpr auto SECTOR_SEARCH_DEPTH = 2;

		auto sphere = BoundingSphere(pos, detectRadius);
		auto nearbyAttracPtrs = std::vector<Attractor*>{};

		// Draw debug sphere.
		g_Renderer.AddDebugSphere(sphere.Center, sphere.Radius, Vector4::One, RendererDebugPage::CollisionStats);

		// TEMP
		// Collect debug attractors.
		auto debugAttracPtrs = GetDebugAttractorPtrs();
		for (auto* attracPtr : debugAttracPtrs)
		{
			if (sphere.Intersects(attracPtr->GetBox()))
				nearbyAttracPtrs.push_back(attracPtr);
		}

		// Collect room attractors in neighbor rooms.
		auto& room = g_Level.Rooms[roomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
			for (auto& attrac : neighborRoom.Attractors)
			{
				if (sphere.Intersects(attrac.GetBox()))
					nearbyAttracPtrs.push_back(&attrac);
			}
		}

		auto bridgeItemNumbers = std::set<int>{};

		// Collect bridge item numbers in neighbor sectors.
		auto sectorPtrs = GetNeighborSectorPtrs(Vector3i(pos), roomNumber, SECTOR_SEARCH_DEPTH);
		for (auto* sectorPtr : sectorPtrs)
		{
			for (int bridgeItemNumber : sectorPtr->BridgeItemNumbers)
				bridgeItemNumbers.insert(bridgeItemNumber);
		}

		// Collect bridge attractors.
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

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle, float detectRadius)
	{
		constexpr auto COLL_COUNT_MAX = 64;

		// Get pointers to approximately nearby attractors.
		auto nearbyAttracPtrs = GetNearbyAttractorPtrs(pos, roomNumber, detectRadius);

		// Collect attractor collisions.
		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(nearbyAttracPtrs.size());
		for (auto* attracPtr : nearbyAttracPtrs)
		{
			auto attracColl = GetAttractorCollision(*attracPtr, pos, headingAngle);

			// Filter out non-intersections.
			if (attracColl.Proximity.Distance3D > detectRadius)
				continue;

			attracColls.push_back(std::move(attracColl));
		}

		// Sort collisions by 2D then 3D distance.
		std::sort(
			attracColls.begin(), attracColls.end(),
			[](const auto& attracColl0, const auto& attracColl1)
			{
				if (attracColl0.Proximity.Distance2D == attracColl1.Proximity.Distance2D)
					return (attracColl0.Proximity.Distance3D < attracColl1.Proximity.Distance3D);
				
				return (attracColl0.Proximity.Distance2D < attracColl1.Proximity.Distance2D);
			});

		// Trim collection.
		if (attracColls.size() > COLL_COUNT_MAX)
			attracColls.resize(COLL_COUNT_MAX);

		// Return attractor collisions in capped vector sorted by 2D then 3D distance.
		return attracColls;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float detectRadius)
	{
		auto relOffset = Vector3(right, down, forward);
		auto rotMatrix = Matrix::CreateRotationY(TO_RAD(headingAngle));
		auto probePoint = pos + Vector3::Transform(relOffset, rotMatrix);

		return GetAttractorCollisions(probePoint, roomNumber, headingAngle, detectRadius);
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, float forward, float down, float right, float detectRadius)
	{
		return GetAttractorCollisions(
			item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation.y,
			forward, down, right, detectRadius);
	}
}
