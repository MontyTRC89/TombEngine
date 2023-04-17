#include "framework.h"
#include "Game/collision/Attractors.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Attractors
{
	Attractor::Attractor(AttractorType type, const Vector3& point0, const Vector3& point1, int roomNumber)
	{
		Type = type;
		Point0 = point0;
		Point1 = point1;
		RoomNumber = roomNumber;
	}

	AttractorType Attractor::GetType() const
	{
		return Type;
	}

	Vector3 Attractor::GetPoint0() const
	{
		return Point0;
	}

	Vector3 Attractor::GetPoint1() const
	{
		return Point1;
	}

	int Attractor::GetRoomNumber() const
	{
		return RoomNumber;
	}

	bool Attractor::IsEdge() const
	{
		return (Type == AttractorType::Edge);
	}

	void Attractor::DrawDebug() const
	{
		constexpr auto COLOR_GREEN	= Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr auto COLOR_YELLOW = Vector4(1.0f, 1.0f, 0.0f, 1.0f);

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

		auto orient = Geometry::GetOrientToPoint(Point0, Point1);
		orient.y += ANGLE(90.0f);
		auto direction = orient.ToDirection();

		auto stringPos = ((Point0 + Point1) / 2) + Vector3(0.0f, -CLICK(0.25f), 0.0f);
		auto stringPos2D = g_Renderer.GetScreenSpacePosition(stringPos);

		g_Renderer.AddLine3D(Point0, Point1, COLOR_YELLOW);
		g_Renderer.AddLine3D(Point0, Point0 + (direction * 50.0f), COLOR_GREEN);
		g_Renderer.AddLine3D(Point1, Point1 + (direction * 50.0f), COLOR_GREEN);
		g_Renderer.AddString(labelString, stringPos2D, Color(PRINTSTRING_COLOR_WHITE), 0.75f, 0);
	}

	bool Attractor::operator ==(const Attractor& attrac) const
	{
		if (Type == attrac.GetType() &&
			Point0 == attrac.GetPoint0() &&
			Point1 == attrac.GetPoint1() &&
			RoomNumber == attrac.GetRoomNumber())
		{
			return true;
		}

		return false;
	}
	
	bool Attractor::operator !=(const Attractor& attrac) const
	{
		return !(*this == attrac);
	}

	// TODO: Actually probe for attractors.
	std::vector<const Attractor*> GetNearbyAttractorPtrs(const Vector3& pos, int roomNumber, float range)
	{
		constexpr auto COUNT_MAX = 64;

		auto rangeSqr = SQUARE(range);

		auto nearbyAttracPtrs = std::vector<const Attractor*>{};
		auto subRoomNumbers = std::set<int>{ roomNumber };

		// Assess attractors in current room.
		const auto& room = g_Level.Rooms[roomNumber];
		for (const auto& attrac : room.Attractors)
		{

		}

		// Assess attractors in neighboring rooms (search depth of 2).
		for (const int& subRoomNumber : room.neighbors)
		{
			const auto& subRoom = g_Level.Rooms[subRoomNumber];
		}

		// Assess bridge attractors.
		for (const auto& [bridgeItemNumber, attracs] : g_Level.BridgeAttractors)
		{
			const auto& bridgeItem = g_Level.Items[bridgeItemNumber];

			if (!subRoomNumbers.count(bridgeItem.RoomNumber))
				continue;

			if (Vector3::DistanceSquared(pos, bridgeItem.Pose.Position.ToVector3()) > rangeSqr)
				continue;
		}

		return nearbyAttracPtrs;
	}

	std::vector<const Attractor*> GetNearbyAttractorPtrs(const ItemInfo& item)
	{
		constexpr auto RANGE	 = BLOCK(5);
		constexpr auto COUNT_MAX = 64;

		auto& player = GetLaraInfo(item);

		auto nearbyAttracPtrs = std::vector<const Attractor*>{};
		for (auto& attrac : player.Context.Attractor.SectorAttractors)
		{
			assertion(nearbyAttracPtrs.size() <= COUNT_MAX, "Nearby attractor pointer collection overflow.");
			if (nearbyAttracPtrs.size() == COUNT_MAX)
				return nearbyAttracPtrs;

			nearbyAttracPtrs.push_back(&attrac);
			attrac.DrawDebug();
		}
		
		nearbyAttracPtrs.push_back(&player.Context.Attractor.DebugAttractor0);
		nearbyAttracPtrs.push_back(&player.Context.Attractor.DebugAttractor1);
		nearbyAttracPtrs.push_back(&player.Context.Attractor.DebugAttractor2);
		nearbyAttracPtrs.push_back(&player.Context.Attractor.DebugAttractor3);
		nearbyAttracPtrs.push_back(&player.Context.Attractor.DebugAttractor4);
		return nearbyAttracPtrs;
	}

	static AttractorCollisionData GetAttractorCollision(const Attractor& attrac, const Vector3& basePos, const EulerAngles& orient,
														const Vector3& refPoint, float range)
	{
		// Get points.
		auto point0 = attrac.GetPoint0();
		auto point1 = attrac.GetPoint1();
		auto targetPoint = Geometry::GetClosestPointOnLinePerp(refPoint, point0, point1);

		// Calculate distances.
		float dist = Vector3::Distance(refPoint, targetPoint);
		float distFromEnd = std::min(Vector3::Distance(targetPoint, point0), Vector3::Distance(targetPoint, point1));

		// Calculate angles.
		auto attracOrient = Geometry::GetOrientToPoint(point0, point1);
		short headingAngle = attracOrient.y - ANGLE(90.0f);
		short slopeAngle = attracOrient.x;

		// Determine inquiries.
		bool isIntersected = (dist <= range);
		bool isInFront = Geometry::IsPointInFront(basePos, targetPoint, orient);

		// Create new attractor collision.
		auto attracColl = AttractorCollisionData{};

		attracColl.AttractorPtr = &attrac;
		attracColl.TargetPoint = targetPoint;
		attracColl.Distance = dist;
		attracColl.DistanceFromEnd = distFromEnd;
		attracColl.HeadingAngle = headingAngle;
		attracColl.SlopeAngle = slopeAngle;
		attracColl.IsIntersected = isIntersected;
		attracColl.IsInFront = isInFront;

		return attracColl;
	}
	
	std::vector<AttractorCollisionData> GetAttractorCollisions(const std::vector<const Attractor*>& attracPtrs,
															   const Vector3& basePos, const EulerAngles& orient,
															   const Vector3& refPoint, float range)
	{
		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(attracPtrs.size());

		for (const auto* attrac : attracPtrs)
		{
			auto attracColl = GetAttractorCollision(*attrac, basePos, orient, refPoint, range);
			attracColls.push_back(attracColl);
		}

		return attracColls;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const std::vector<const Attractor*>& attracPtrs,
															   const ItemInfo& item, const Vector3& refPoint, float range)
	{
		return GetAttractorCollisions(attracPtrs, item.Pose.Position.ToVector3(), item.Pose.Orientation, refPoint, range);
	}

	/*static */std::vector<Attractor> GenerateAttractorsFromPoints(const std::vector<Vector3>& points, int roomNumber, AttractorType type,
															   bool isClosedLoop)
	{
		// No points; return empty vector.
		if (points.empty())
			return {};

		// Prepare container.
		auto attracs = std::vector<Attractor>{};
		attracs.reserve(points.size());

		// Generate attractors between points.
		unsigned int count = isClosedLoop ? points.size() : (points.size() - 1);
		for (int i = 0; i < count; i++)
		{
			auto linePoint0 = points[i];
			auto linePoint1 = points[(i < (points.size() - 1)) ? (i + 1) : 0];
			auto attrac = Attractor(type, linePoint0, linePoint1, roomNumber);

			attracs.push_back(attrac);
		}

		// Return attractors.
		return attracs;
	}

	static std::vector<Attractor> GenerateBridgeAttractors(const ItemInfo& item)
	{
		constexpr auto TILT_STEP = CLICK(1);

		// Get bounding box.
		auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		// Determine tilt offset.
		int tiltOffset = 0;
		switch (item.ObjectNumber)
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

		// Generate and return attractors.
		return GenerateAttractorsFromPoints(points, item.RoomNumber, AttractorType::Edge);
	}

	std::vector<Attractor> GenerateSectorAttractors(const CollisionResult& pointColl)
	{
		// Invalid sector; return empty vector.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return {};

		// Generate and return bridge attractors.
		if (pointColl.Position.Bridge >= 0)
		{
			const auto& bridgeItem = g_Level.Items[pointColl.Position.Bridge];
			return GenerateBridgeAttractors(bridgeItem);
		}

		// Generate and return floor attractors.
		auto points = pointColl.BottomBlock->GetSurfaceVertices(pointColl.Coordinates.x, pointColl.Coordinates.z, true);
		return GenerateAttractorsFromPoints(points, pointColl.RoomNumber, AttractorType::Edge);
	}
}
