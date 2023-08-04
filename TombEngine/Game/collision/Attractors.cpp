#include "framework.h"
#include "Game/collision/Attractors.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Attractors
{
	Attractor::Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber)
	{
		assertion(!points.empty(), "Attempted to initialize invalid attractor.");

		Type = type;
		Points = points;
		RoomNumber = roomNumber;
		Box = Geometry::GetBoundingBox(points);

		// Cache length.
		if (points.size() >= 2)
		{
			for (int i = 0; i < (points.size() - 1); i++)
			{
				// Get segment points.
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				Length += Vector3::Distance(origin, target);
			}
		}

		//AttachedPlayers = {};
	}

	Attractor::~Attractor()
	{
		// Dereference current attractor held by players.
		/*for (auto& [entityID, entity] : AttachedPlayers)
		{
			if (!entity.IsLara())
				continue;

			auto& player = GetLaraInfo(entity);

			if (player.Context.HandsAttractor.AttracPtr == this)
				player.Context.HandsAttractor.AttracPtr = nullptr;

			if (player.Context.FeetAttractor.AttracPtr == this)
				player.Context.FeetAttractor.AttracPtr = nullptr;
		}*/
	}

	AttractorType Attractor::GetType() const
	{
		return Type;
	}

	const std::vector<Vector3>& Attractor::GetPoints() const
	{
		return Points;
	}

	int Attractor::GetRoomNumber() const
	{
		return RoomNumber;
	}

	float Attractor::GetLength() const
	{
		return Length;
	}

	const BoundingBox& Attractor::GetBoundingBox() const
	{
		return Box;
	}

	AttractorCollisionData Attractor::GetCollision(const Vector3& basePos, const EulerAngles& orient, const Vector3& refPoint, float range) const
	{
		constexpr auto HEADING_ANGLE_OFFSET			  = ANGLE(-90.0f);
		constexpr auto FORWARD_FACING_ANGLE_THRESHOLD = ANGLE(90.0f);

		// Get attractor proximity data.
		auto attracProx = GetProximity(refPoint);

		// Get segment points.
		const auto& origin = Points[attracProx.SegmentID];
		const auto& target = Points[attracProx.SegmentID + 1];

		// Calculate angles.
		auto attracOrient = (Points.size() == 1) ? orient : Geometry::GetOrientToPoint(origin, target);
		short headingAngle = attracOrient.y + HEADING_ANGLE_OFFSET;
		short slopeAngle = attracOrient.x;

		// Determine inquiries.
		bool isIntersected = (attracProx.Distance <= range);
		bool isFacingForward = (abs(Geometry::GetShortestAngle(headingAngle, orient.y)) <= FORWARD_FACING_ANGLE_THRESHOLD);
		bool isInFront = Geometry::IsPointInFront(basePos, attracProx.IntersectPoint, orient);

		// Create attractor collision data.
		auto attracColl = AttractorCollisionData(*this);
		attracColl.Proximity = attracProx;
		attracColl.HeadingAngle = headingAngle;
		attracColl.SlopeAngle = slopeAngle;
		attracColl.IsIntersected = isIntersected;
		attracColl.IsFacingForward = isFacingForward;
		attracColl.IsInFront = isInFront;

		// Return attractor collision data.
		return attracColl;
	}

	AttractorProximityData Attractor::GetProximity(const Vector3& refPoint) const
	{
		// Single point exists; return simple attractor proximity data.
		if (Points.size() == 1)
		{
			return AttractorProximityData
			{
				Points.front(),
				Vector3::Distance(refPoint, Points.front()),
				0.0f,
				0
			};
		}

		auto attracProx = AttractorProximityData{ Points.front(), INFINITY, 0.0f, 0 };
		float chainDistTravelled = 0.0f;

		// Find closest point along attractor.
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			auto closestPoint = Geometry::GetClosestPointOnLinePerp(refPoint, origin, target);
			float dist = Vector3::Distance(refPoint, closestPoint);

			// Found new closest point; update proximity data.
			if (dist < attracProx.Distance)
			{
				chainDistTravelled += Vector3::Distance(origin, closestPoint);

				attracProx.IntersectPoint = closestPoint;
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

	Vector3 Attractor::GetPointAtChainDistance(float chainDist) const
	{
		// Single point exists; return it.
		if (Points.size() == 1)
			return Points.front();
		
		// Normalize distance along attractor.
		chainDist = NormalizeChainDistance(chainDist);

		// Line distance is outside attractor; return clamped point.
		if (chainDist <= 0.0f)
		{
			return Points.front();
		}
		else if (chainDist >= Length)
		{
			return Points.back();
		}
		
		// Find point at distance along attractor.
		float chainDistTravelled = 0.0f;
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			float segmentLength = Vector3::Distance(origin, target);
			float remainingChainDist = chainDist - chainDistTravelled;

			// Found segment of distance along attractor; return interpolated point.
			if (remainingChainDist <= segmentLength)
			{
				float alpha = remainingChainDist / segmentLength;
				return Vector3::Lerp(origin, target, alpha);
			}

			// Accumulate distance travelled along attractor.
			chainDistTravelled += segmentLength;
		}

		// FAILSAFE: Return end point.
		return Points.back();
	}

	unsigned int Attractor::GetSegmentIDAtChainDistance(float chainDist) const
	{
		// Single segment exists; return segment ID 0.
		if (Points.size() <= 2)
			return 0;

		// Normalize distance along attractor.
		chainDist = NormalizeChainDistance(chainDist);

		// Chain distance is on attractor edge; return clamped segment ID.
		if (chainDist <= 0.0f)
		{
			return 0;
		}
		else if (chainDist >= Length)
		{
			return ((int)Points.size() - 1);
		}

		// Find segment at distance along attractor.
		float chainDistTravelled = 0.0f;
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			// Accumulate distance travelled along attractor.
			chainDistTravelled += Vector3::Distance(origin, target);

			// Segment found; return segment ID.
			if (chainDistTravelled >= chainDist)
				return i;
		}

		// FAILSAFE: Return end segment ID.
		return ((int)Points.size() - 1);
	}

	bool Attractor::IsEdge() const
	{
		return (Type == AttractorType::Edge);
	}

	bool Attractor::IsLooped() const
	{
		// Too few points; loop not possible.
		if (Points.size() <= 2)
			return false;

		// Test if start and end points occupy roughly same position.
		return (Vector3::Distance(Points.front(), Points.back()) <= EPSILON);
	}

	void Attractor::AttachPlayer(ItemInfo& entity)
	{
		if (!entity.IsLara())
			return;

		//AttachedPlayers.insert({ entity.Index, entity });
	}

	void Attractor::DetachPlayer(ItemInfo& entity)
	{
		if (!entity.IsLara())
			return;

		//AttachedPlayers.erase(entity.Index);
	}

	void Attractor::Update(const std::vector<Vector3>& points, int roomNumber)
	{
		assertion(!points.empty(), "Attempted to update invalid attractor.");

		Points = points;
		RoomNumber = roomNumber;
		Box = Geometry::GetBoundingBox(points);

		// Cache length.
		Length = 0.0f;
		if (points.size() >= 2)
		{
			for (int i = 0; i < (points.size() - 1); i++)
			{
				// Get segment points.
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				Length += Vector3::Distance(origin, target);
			}
		}
	}

	void Attractor::DrawDebug() const
	{
		constexpr auto LABEL_SCALE			 = 0.8f;
		constexpr auto LABEL_OFFSET			 = Vector3(0.0f, -CLICK(0.25f), 0.0f);
		constexpr auto INDICATOR_LINE_LENGTH = 50.0f;
		constexpr auto SPHERE_SCALE			 = 15.0f;
		constexpr auto COLOR_GREEN			 = Vector4(0.4f, 1.0f, 0.4f, 1.0f);
		constexpr auto COLOR_YELLOW			 = Vector4(1.0f, 1.0f, 0.4f, 1.0f);
		constexpr auto DEBUG_PAGE			 = RendererDebugPage::CollisionStats;

		// Determine label string.
		auto labelString = std::string();
		switch (Type)
		{
		default:
			labelString = "Attractor";
			break;

		case AttractorType::Edge:
			labelString = "Edge";
			break;
		}

		// Draw debug elements.
		if (Points.size() >= 2)
		{
			for (int i = 0; i < (Points.size() - 1); i++)
			{
				// Get segment points.
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				// Draw main line.
				g_Renderer.AddLine3D(origin, target, COLOR_YELLOW);

				auto orient = Geometry::GetOrientToPoint(origin, target);
				orient.y += ANGLE(90.0f);
				auto dir = orient.ToDirection();

				// Draw segment heading indicator lines.
				g_Renderer.AddLine3D(origin, origin + (dir * INDICATOR_LINE_LENGTH), COLOR_GREEN);
				g_Renderer.AddLine3D(target, target + (dir * INDICATOR_LINE_LENGTH), COLOR_GREEN);

				auto labelPos = ((origin + target) / 2) + LABEL_OFFSET;
				auto labelPos2D = g_Renderer.Get2DPosition(labelPos);

				// Draw label.
				if (labelPos2D.has_value())
					g_Renderer.AddDebugString(labelString, *labelPos2D, Color(PRINTSTRING_COLOR_WHITE), LABEL_SCALE, 0, DEBUG_PAGE);
			}

			// Draw start and end indicator lines.
			g_Renderer.AddLine3D(Points.front(), Points.front() + (-Vector3::UnitY * INDICATOR_LINE_LENGTH), COLOR_GREEN);
			g_Renderer.AddLine3D(Points.back(), Points.back() + (-Vector3::UnitY * INDICATOR_LINE_LENGTH), COLOR_GREEN);
		}
		else if (Points.size() == 1)
		{
			// Draw sphere.
			g_Renderer.AddSphere(Points.front(), SPHERE_SCALE, COLOR_YELLOW);

			// Draw label.
			auto labelPos2D = g_Renderer.Get2DPosition(Points.front());
			if (labelPos2D.has_value())
				g_Renderer.AddString(labelString, *labelPos2D, Color(PRINTSTRING_COLOR_WHITE), LABEL_SCALE, PRINTSTRING_OUTLINE);
		}
	}

	float Attractor::NormalizeChainDistance(float chainDist) const
	{
		// Distance along attractor is within bounds; return it.
		if (chainDist >= 0.0f && chainDist <= Length)
			return chainDist;

		// Looped; wrap distance along attractor.
		if (IsLooped())
		{
			int sign = -std::copysign(1, chainDist);
			return (chainDist + (Length * sign));
		}
		
		// Not looped; clamp distance along attractor.
		return std::clamp(chainDist, 0.0f, Length);
	}

	// TEMP
	static std::vector<const Attractor*> GetDebugAttractorPtrs(const ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto nearbyAttracPtrs = std::vector<const Attractor*>{};
		nearbyAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac0);
		nearbyAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac1);
		nearbyAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac2);
		return nearbyAttracPtrs;
	}

	static void DrawDebugAttractorBounds(const BoundingSphere& sphere, std::vector<const Attractor*> attracs)
	{
		g_Renderer.AddDebugSphere(sphere.Center, sphere.Radius, Vector4::One, RendererDebugPage::CollisionStats);

		for (const auto* attracPtr : attracs)
		{
			const auto& box = attracPtr->GetBoundingBox();
			auto orientedBox = BoundingOrientedBox(box.Center, box.Extents, Quaternion::Identity);

			g_Renderer.AddDebugBox(orientedBox, Vector4::One, RendererDebugPage::CollisionStats);
		}
	}

	// TODO: TRAE method of a search algorithm incorporating an R-tree might be ideal here.
	// Would require a general collision refactor. -- Sezz 2023.07.30
	static std::vector<const Attractor*> GetNearbyAttractorPtrs(const Vector3& refPoint, int roomNumber, float range)
	{
		auto sphere = BoundingSphere(refPoint, range);
		auto nearbyAttracPtrs = std::vector<const Attractor*>{};

		// TEMP
		// Get debug attractors.
		auto debugAttracPtrs = GetDebugAttractorPtrs(*LaraItem);
		for (const auto* attracPtr : debugAttracPtrs)
		{
			if (sphere.Intersects(attracPtr->GetBoundingBox()))
				nearbyAttracPtrs.push_back(attracPtr);
		}

		// Get attractors in neighboring rooms.
		const auto& room = g_Level.Rooms[roomNumber];
		for (int roomNumber : room.neighbors)
		{
			const auto& subRoom = g_Level.Rooms[roomNumber];
			for (const auto& attrac : subRoom.Attractors)
			{
				if (sphere.Intersects(attrac.GetBoundingBox()))
					nearbyAttracPtrs.push_back(&attrac);
			}
		}

		// Get bridge attractors.
		for (const auto& [bridgeID, attrac] : g_Level.BridgeAttractors)
		{
			if (sphere.Intersects(attrac.GetBoundingBox()))
				nearbyAttracPtrs.push_back(&attrac);
		}

		DrawDebugAttractorBounds(sphere, nearbyAttracPtrs);

		// Return attractor pointers found in sphere-AABB test.
		return nearbyAttracPtrs;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& refPoint, float range)
	{
		constexpr auto COUNT_MAX = 64;

		// Get pointers to approximately nearby attractors from sphere-AABB tests.
		auto nearbyAttracPtrs = GetNearbyAttractorPtrs(refPoint, roomNumber, range);

		// Get attractor collisions sorted by distance.
		auto attracCollMap = std::multimap<float, AttractorCollisionData>{};
		for (const auto& attracPtr : nearbyAttracPtrs)
		{
			auto attracColl = attracPtr->GetCollision(basePos, orient, refPoint, range);
			attracCollMap.insert({ attracColl.Proximity.Distance, attracColl });
		}

		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(std::min((int)nearbyAttracPtrs.size(), COUNT_MAX));

		// Move attractor collisions from map to capped vector.
		auto it = attracCollMap.begin();
		for (int i = 0; i < COUNT_MAX && it != attracCollMap.end(); i++, it++)
			attracColls.push_back(std::move(it->second));

		// Return attractor collisions.
		return attracColls;
	}
	
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& refPoint, float range)
	{
		return GetAttractorCollisions(item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation, refPoint, range);
	}
	
	Attractor GenerateAttractorFromPoints(std::vector<Vector3> points, int roomNumber, AttractorType type, bool isClosedLoop)
	{
		// Add point to create loop (if applicable).
		if (isClosedLoop)
			points.push_back(points.front());

		// Generate attractor.
		return Attractor(type, points, roomNumber);
	}

	static Attractor GenerateBridgeAttractor(const ItemInfo& bridge)
	{
		constexpr auto TILT_STEP = CLICK(1);

		// Get bounding box.
		auto box = GameBoundingBox(&bridge).ToBoundingOrientedBox(bridge.Pose);

		// Determine tilt offset.
		int tiltOffset = 0;
		switch (bridge.ObjectNumber)
		{
		default:
		case ID_BRIDGE_FLAT:
			break;

		case ID_BRIDGE_TILT1:
			tiltOffset = TILT_STEP;
			break;

		case ID_BRIDGE_TILT2:
			tiltOffset = TILT_STEP * 2;
			break;

		case ID_BRIDGE_TILT3:
			tiltOffset = TILT_STEP * 3;
			break;

		case ID_BRIDGE_TILT4:
			tiltOffset = TILT_STEP * 4;
			break;
		}

		// Determine relative corner points.
		auto point0 = Vector3(box.Extents.x, -box.Extents.y + tiltOffset, box.Extents.z);
		auto point1 = Vector3(-box.Extents.x, -box.Extents.y, box.Extents.z);
		auto point2 = Vector3(-box.Extents.x, -box.Extents.y, -box.Extents.z);
		auto point3 = Vector3(box.Extents.x, -box.Extents.y + tiltOffset, -box.Extents.z);

		// Calculate absolute corner points.
		auto rotMatrix = Matrix::CreateFromQuaternion(box.Orientation);
		auto points = std::vector<Vector3>
		{
			box.Center + Vector3::Transform(point0, rotMatrix),
			box.Center + Vector3::Transform(point1, rotMatrix),
			box.Center + Vector3::Transform(point2, rotMatrix),
			box.Center + Vector3::Transform(point3, rotMatrix)
		};

		// Generate and return attractor.
		return GenerateAttractorFromPoints(points, bridge.RoomNumber, AttractorType::Edge);
	}

	// Debug
	std::optional<Attractor> GenerateSectorAttractor(const CollisionResult& pointColl)
	{
		// Invalid sector; return nullopt.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return std::nullopt;

		// Generate and return bridge attractor.
		if (pointColl.Position.Bridge >= 0)
		{
			const auto& bridge = g_Level.Items[pointColl.Position.Bridge];
			return GenerateBridgeAttractor(bridge);
		}

		// Generate and return floor attractor.
		auto points = pointColl.BottomBlock->GetSurfaceVertices(pointColl.Coordinates.x, pointColl.Coordinates.z, true);
		return GenerateAttractorFromPoints(points, pointColl.RoomNumber, AttractorType::Edge);
	}
}
