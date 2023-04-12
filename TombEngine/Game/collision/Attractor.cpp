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

namespace TEN::Collision
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

	static std::vector<Attractor> GenerateAttractorsFromPoints(const std::vector<Vector3>& points, int roomNumber, AttractorType type,
															   bool isClosedLoop = true)
	{
		// Prepare container.
		auto attracs = std::vector<Attractor>{};
		attracs.resize(points.size());

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

	// TODO: Bridge tilts.
	static std::vector<Attractor> GetBridgeAttractors(const ItemInfo& item)
	{
		// Get bridge bounding box.
		auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		// Determine relative corner points.
		auto point0 = Vector3(box.Extents.x, -box.Extents.y, box.Extents.z);
		auto point1 = Vector3(-box.Extents.x, -box.Extents.y, box.Extents.z);
		auto point2 = Vector3(-box.Extents.x, -box.Extents.y, -box.Extents.z);
		auto point3 = Vector3(box.Extents.x, -box.Extents.y, -box.Extents.z);

		// Calculate absolute corner points.
		auto rotMatrix = Matrix::CreateFromQuaternion(box.Orientation);
		auto points = std::vector<Vector3>
		{
			box.Center + Vector3::Transform(point0, rotMatrix),
			box.Center + Vector3::Transform(point1, rotMatrix),
			box.Center + Vector3::Transform(point2, rotMatrix),
			box.Center + Vector3::Transform(point3, rotMatrix)
		};

		// Return attractors generated from points.
		return GenerateAttractorsFromPoints(points, item.RoomNumber, AttractorType::Edge);
	}

	std::vector<Attractor> GetSectorAttractors(const CollisionResult& pointColl)
	{
		// Get bridge attractors.
		if (pointColl.Position.Bridge >= 0)
		{
			const auto& bridgeItem = g_Level.Items[pointColl.Position.Bridge];
			return GetBridgeAttractors(bridgeItem);
		}

		// Get room attractors.
		auto points = pointColl.BottomBlock->GetSurfaceVertices(pointColl.Coordinates.x, pointColl.Coordinates.z, true);
		return GenerateAttractorsFromPoints(points, pointColl.RoomNumber, AttractorType::Edge);
	}

	// TODO: Actually probe for attractors.
	std::vector<const Attractor*> GetNearbyAttractorPtrs(const ItemInfo& item)
	{
		constexpr auto COUNT_MAX = 32;

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

		// Determine enquiries.
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

	// TODO: Maybe return struct with this vector and the refPoint + range, just in case.
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
}
