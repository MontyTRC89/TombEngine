#include "framework.h"
#include "Game/collision/Attractors.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"

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
		constexpr auto COLOR = Vector4(1.0f, 1.0f, 0.0f, 1.0f);

		g_Renderer.AddLine3D(GetPoint0(), GetPoint1(), COLOR);
	}

	bool Attractor::operator ==(const Attractor& attrac) const
	{
		if (Type == attrac.Type &&
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
	std::vector<const Attractor*> GetNearbyAttractorPtrs(const ItemInfo& item)
	{
		constexpr auto COUNT_MAX = 64;

		auto& player = GetLaraInfo(item);

		auto attracPtrs = std::vector<const Attractor*>{};
		for (auto& attrac : player.Context.Attractor.SectorAttractors)
		{
			assertion(attracPtrs.size() <= COUNT_MAX, "Nearby attractor pointer collection overflow.");
			if (attracPtrs.size() == COUNT_MAX)
				return attracPtrs;

			attracPtrs.push_back(&attrac);
			attrac.DrawDebug();
		}

		attracPtrs.push_back(&player.Context.Attractor.DebugAttractor0);
		attracPtrs.push_back(&player.Context.Attractor.DebugAttractor1);
		attracPtrs.push_back(&player.Context.Attractor.DebugAttractor2);
		attracPtrs.push_back(&player.Context.Attractor.DebugAttractor3);
		attracPtrs.push_back(&player.Context.Attractor.DebugAttractor4);
		return attracPtrs;
	}

	static AttractorCollisionData GetAttractorCollision(const ItemInfo& item, const Attractor& attrac, const Vector3& refPoint, float range)
	{
		// Get points.
		auto point0 = attrac.GetPoint0();
		auto point1 = attrac.GetPoint1();
		auto targetPoint = attrac.IsEdge() ?
			Geometry::GetClosestPointOnLinePerp(refPoint, point0, point1) :
			Geometry::GetClosestPointOnLine(refPoint, point0, point1);

		// Calculate distances.
		float dist = Vector3::Distance(refPoint, targetPoint);
		float distFromEnd = std::min(Vector3::Distance(targetPoint, point0), Vector3::Distance(targetPoint, point1));

		// Calculate angles.
		auto orient = Geometry::GetOrientToPoint(point0, point1);
		short headingAngle = orient.y - ANGLE(90.0f);
		short slopeAngle = orient.x;

		// Determine inquiries.
		bool isIntersected = (dist <= range);
		bool isInFront = Geometry::IsPointInFront(item.Pose, targetPoint);

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

	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const std::vector<const Attractor*>& attracPtrs,
															   const Vector3& refPoint, float range)
	{
		auto attracColls = std::vector<AttractorCollisionData>{};
		for (const auto* attrac : attracPtrs)
		{
			auto attracColl = GetAttractorCollision(item, *attrac, refPoint, range);
			attracColls.push_back(attracColl);
		}

		return attracColls;
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
