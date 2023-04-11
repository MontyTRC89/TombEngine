#include "framework.h"
#include "Game/collision/Attractors.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"

using namespace TEN::Math;

// Debug
#include <ois/OISKeyboard.h>
#include "Renderer/Renderer11.h"
#include "lara_helpers.h"
#include "Specific/Input/Input.h"
using namespace TEN::Input;
using TEN::Renderer::g_Renderer;
// ---

namespace TEN::Collision
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
		g_Renderer.AddLine3D(GetPoint0(), GetPoint1(), Vector4::One);
	}

	static std::vector<Attractor> GenerateAttractorsFromPoints(const std::vector<Vector3>& points, int roomNumber, bool isClosedLoop = true)
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
			auto attrac = Attractor(AttractorType::Edge, linePoint0, linePoint1, roomNumber);

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
		return GenerateAttractorsFromPoints(points, item.RoomNumber);
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
		return GenerateAttractorsFromPoints(points, pointColl.RoomNumber);
	}

	static AttractorCollision GetAttractorCollision(const ItemInfo& item, const CollisionInfo& coll, const Attractor& attrac,
													const Vector3& refPoint, float range)
	{
		// Get points.
		auto point0 = attrac.GetPoint0();
		auto point1 = attrac.GetPoint1();
		auto closestPoint = attrac.IsEdge() ?
			Geometry::GetPerpendicularPointOnLine(refPoint, point0, point1) :
			Geometry::GetClosestPointOnLine(refPoint, point0, point1);

		// Calculate distances.
		float dist = Vector3::Distance(refPoint, closestPoint);
		float distFromEnd = std::min(Vector3::Distance(closestPoint, point0), Vector3::Distance(closestPoint, point1));

		// Determine enquiries.
		bool isIntersected = (dist <= range);
		bool isInFront = Geometry::IsPointInFront(item.Pose, closestPoint);
		auto pointOnLeft = Geometry::IsPointOnLeft(item.Pose, point0) ? AttractorPoint::Point0 : AttractorPoint::Point1;

		// Calculate angles.
		auto orient = Geometry::GetOrientToPoint(point0, point1);
		short facingAngle = orient.y - ANGLE(90.0f);
		short slopeAngle = orient.x;

		// Create new attractor collision.
		auto attracColl = AttractorCollision{};

		attracColl.AttractorPtr = &attrac;
		attracColl.ClosestPoint = closestPoint;
		attracColl.IsIntersected = isIntersected;
		attracColl.IsInFront = isInFront;
		attracColl.Distance = dist;
		attracColl.DistanceFromEnd = distFromEnd;
		attracColl.HeadingAngle = facingAngle;
		attracColl.SlopeAngle = slopeAngle;
		attracColl.PointOnLeft = pointOnLeft;
		return attracColl;
	}

	// TODO: Actually probe for attractors.
	std::vector<const Attractor*> GetNearbyAttractorPtrs(const ItemInfo& item)
	{
		constexpr auto COUNT_MAX = 32;

		auto& player = GetLaraInfo(item);

		auto attracPtrs = std::vector<const Attractor*>{};

		for (auto& attrac : player.Context.Attractor.SectorAttractors)
		{
			if (attracPtrs.size() >= COUNT_MAX)
				return attracPtrs;

			attracPtrs.push_back(&attrac);

			attrac.DrawDebug();
		}

		attracPtrs.push_back(&player.Context.Attractor.DebugAttractor);
		return attracPtrs;
	}

	std::vector<AttractorCollision> GetAttractorCollisions(const ItemInfo& item, const CollisionInfo& coll,
														   const std::vector<const Attractor*>& attracPtrs,
														   const Vector3& refPoint, float range)
	{
		auto attracColls = std::vector<AttractorCollision>{};
		for (const auto* attrac : attracPtrs)
		{
			auto attracColl = GetAttractorCollision(item, coll, *attrac, refPoint, range);
			attracColls.push_back(attracColl);
		}

		return attracColls;
	}
}
