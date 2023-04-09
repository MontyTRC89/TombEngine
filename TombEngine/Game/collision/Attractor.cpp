#include "framework.h"
#include "Game/collision/Attractors.h"

#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Specific/Input/Input.h"

using namespace TEN::Math;

// Debug
#include <ois/OISKeyboard.h>
#include "Renderer/Renderer11.h"
#include "lara_helpers.h"
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

		// Set points.
		if (KeyMap[OIS::KeyCode::KC_Q])
			player.Attractor.DebugAttractor.SetPoint0(LaraItem->Pose.Position.ToVector3());
		if (KeyMap[OIS::KeyCode::KC_W])
			player.Attractor.DebugAttractor.SetPoint1(LaraItem->Pose.Position.ToVector3());

		// Show attractor as white line.
		g_Renderer.AddLine3D(player.Attractor.DebugAttractor.GetPoint0(), player.Attractor.DebugAttractor.GetPoint1(), Vector4::One);

		// Show tether line. 
		auto frontPos = Geometry::TranslatePoint(LaraItem->Pose.Position, LaraItem->Pose.Orientation.y, LARA_RADIUS, -LARA_HEIGHT);
		auto closestPoint = Geometry::GetClosestPointOnLine(
			LaraItem->Pose.Position.ToVector3(),
			player.Attractor.DebugAttractor.GetPoint0(),
			player.Attractor.DebugAttractor.GetPoint1());

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

	std::vector<Attractor> ConstructedAttractors = {};

	void GetNearbyAttractorData(std::vector<AttractorData>& attractors, const Vector3& pos, const EulerAngles& orient, float range)
	{
		attractors.clear();

		for (const auto& attractor : ConstructedAttractors)
		{
			// Get attractor point data.
			auto point0 = attractor.GetPoint0();
			auto point1 = attractor.GetPoint1();
			auto closestPoint = Geometry::GetClosestPointOnLine(pos, point0, point1);

			// Test if distance is within rance.
			float distance = Vector3::Distance(pos, closestPoint);
			if (distance > range)
				continue;

			// Probe attractor data.

			auto data = AttractorData{};

			data.AttractorPtr = &attractor;
			data.ClosestPoint = closestPoint;
			data.Distance = distance;
			data.SlopeAngle = Geometry::GetOrientToPoint(point0, point1).x;
			data.IsInFront = Geometry::IsPointInFront(pos, closestPoint, orient);
			data.IsIntersected = false; // How?

			attractors.push_back(data);
		}
	}
}
