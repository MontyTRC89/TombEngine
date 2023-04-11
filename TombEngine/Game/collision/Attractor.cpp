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
		Length = Vector3::Distance(Point0, Point1);
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

	float Attractor::GetLength() const
	{
		return Length;
	}

	int Attractor::GetRoomNumber() const
	{
		return RoomNumber;
	}

	void Attractor::SetPoint0(const Vector3& point)
	{
		Point0 = point;
		Length = Vector3::Distance(Point0, Point1);
	}

	void Attractor::SetPoint1(const Vector3& point)
	{
		Point1 = point;
		Length = Vector3::Distance(Point0, Point1);
	}

	bool Attractor::IsEdge()
	{
		return (Type == AttractorType::Edge);
	}

	void Attractor::DrawDebug(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();

		// Set points.
		if (KeyMap[OIS::KeyCode::KC_Q])
		{
			auto pos = LaraItem->Pose.Position.ToVector3() +
				Vector3(0.0f, -LaraCollision.Setup.Height, 0.0f) +
				Vector3::Transform(Vector3(0.0f, box.Extents.z, 0.0f), rotMatrix);
			player.Control.Attractor.DebugAttractor.SetPoint0(pos);
		}

		if (KeyMap[OIS::KeyCode::KC_W])
		{
			auto pos = LaraItem->Pose.Position.ToVector3() +
				Vector3(0.0f, -LaraCollision.Setup.Height, 0.0f) +
				Vector3::Transform(Vector3(0.0f, box.Extents.z, 0.0f), rotMatrix);
			player.Control.Attractor.DebugAttractor.SetPoint1(pos);
		}

		// Show attractor as white line.
		g_Renderer.AddLine3D(player.Control.Attractor.DebugAttractor.GetPoint0(), player.Control.Attractor.DebugAttractor.GetPoint1(), Vector4::One);

		// Show tether line. 
		auto frontPos = Geometry::TranslatePoint(LaraItem->Pose.Position, LaraItem->Pose.Orientation.y, LARA_RADIUS, -LARA_HEIGHT);
		auto closestPoint = Geometry::GetClosestPointOnLine(
			LaraItem->Pose.Position.ToVector3(),
			player.Control.Attractor.DebugAttractor.GetPoint0(),
			player.Control.Attractor.DebugAttractor.GetPoint1());

		// Draw tether lines. Magenta when in front, white when behind.
		if (Geometry::IsPointInFront(LaraItem->Pose, closestPoint))
		{
			g_Renderer.AddLine3D(frontPos.ToVector3(), closestPoint, Vector4(1, 0, 1, 1));
		}
		else
		{
			g_Renderer.AddLine3D(frontPos.ToVector3(), closestPoint, Vector4::One);
		}
	}

	std::vector<Attractor> GetAttractorsFromPoints(const std::vector<Vector3>& points, int roomNumber)
	{
		// Prepare container.
		auto attracs = std::vector<Attractor>{};
		attracs.resize(points.size());

		// Generate attractors between points.
		for (int i = 0; i < points.size(); i++)
		{
			auto point0 = points[i];
			auto point1 = points[(i < (points.size() - 1)) ? (i + 1) : 0];
			auto attrac = Attractor(AttractorType::Edge, point0, point1, roomNumber);

			attracs.push_back(attrac);
		}

		// Return attractors.
		return attracs;
	}

	std::vector<Attractor> GetSectorAttractors(const CollisionResult& pointColl)
	{
		auto points = pointColl.BottomBlock->GetSurfaceVertices(pointColl.Coordinates.x, pointColl.Coordinates.z, true);
		return GetAttractorsFromPoints(points, pointColl.RoomNumber);
	}

	std::vector<Attractor> GetBridgeAttractors(const ItemInfo& item)
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
		return GetAttractorsFromPoints(points, item.RoomNumber);
	}

	Vector3 ProjectOnPlane(const Vector3& vector, const Vector3& planeNormal)
	{
		float dot = vector.Dot(planeNormal);
		return vector - planeNormal * dot;
	}

	AttractorData GetAttractorData(const ItemInfo& item, const CollisionInfo& coll, const Attractor& attrac, const Vector3& refPoint)
	{
		bool getPerpPoint = (attrac.GetType() == AttractorType::Edge);

		// Get attractor point data.
		auto point0 = attrac.GetPoint0();
		auto point1 = attrac.GetPoint1();
		auto closestPoint = getPerpPoint ?
			Geometry::GetPerpendicularPointOnLine(refPoint, point0, point1) :
			Geometry::GetClosestPointOnLine(refPoint, point0, point1);

		// debug
		g_Renderer.AddLine3D(refPoint, closestPoint, Vector4(1, 0, 1, 1));

		// Calculate distances.
		float dist = Geometry::GetDistanceToLine(refPoint, point0, point1);
		float distFromEnd = std::min(Vector3::Distance(closestPoint, point0), Vector3::Distance(closestPoint, point1));

		// Determine enquiries.
		float range = OFFSET_RADIUS(coll.Setup.Radius);
		bool isIntersected = (dist <= range);
		bool isInFront = Geometry::IsPointInFront(item.Pose, closestPoint);
		auto pointOnLeft = Geometry::IsPointOnLeft(item.Pose, point0) ? AttractorPoint::Point0 : AttractorPoint::Point1;

		// Calculate angles.
		auto orient = Geometry::GetOrientToPoint(point0, point1);
		short facingAngle = orient.y - ANGLE(90.0f);
		short slopeAngle = orient.x;

		// Create new attractor data.
		auto attracData = AttractorData{};

		attracData.AttractorPtr = &attrac;
		attracData.ClosestPoint = closestPoint;
		attracData.IsIntersected = isIntersected;
		attracData.IsInFront = isInFront;
		attracData.Distance = dist;
		attracData.DistanceFromEnd = distFromEnd;
		attracData.FacingAngle = facingAngle;
		attracData.SlopeAngle = slopeAngle;
		attracData.PointOnLeft = pointOnLeft;
		return attracData;
	}

	// TODO
	// Store nearby Attractor object pointers.
	// Then, store vectors of AttractorData objects for all three player points.
	void GetPlayerNearbyAttractorData(ItemInfo& item, const CollisionInfo& coll)
	{
		auto& player = GetLaraInfo(item);

		player.Control.Attractor.NearbyData.clear();
		player.Control.Attractor.NearbyData = std::vector<AttractorData>{};

		// TODO: Need different positions.
		auto refPoint = item.Pose.Position.ToVector3() + Vector3(0.0f, coll.Setup.Height, 0.0f);

		auto attracData = GetAttractorData(item, coll, player.Control.Attractor.DebugAttractor, refPoint);
		Lara.Control.Attractor.NearbyData.push_back(attracData);

		for (auto& attrac : player.Control.Attractor.BridgeAttractors)
		{
			if (player.Control.Attractor.NearbyData.size() >= PLAYER_NEARBY_ATTRACTOR_COUNT_MAX)
				return;

			attracData = GetAttractorData(item, coll, attrac, refPoint);
			player.Control.Attractor.NearbyData.emplace_back(attracData);
		}

		for (auto& attrac : player.Control.Attractor.SectorAttractors)
		{
			if (player.Control.Attractor.NearbyData.size() >= PLAYER_NEARBY_ATTRACTOR_COUNT_MAX)
				return;

			attracData = GetAttractorData(item, coll, attrac, refPoint);
			player.Control.Attractor.NearbyData.emplace_back(attracData);
		}
	}
}
