#include "framework.h"
#include "Game/collision/Attractors.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

//------debug
#include "Game/Lara/lara.h"
//------

namespace TEN::Collision::Attractors
{
	Attractor::Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber)
	{
		assertion(!points.empty(), "Attempted to initialize invalid attractor.");

		Type = type;
		Points = points;
		RoomNumber = roomNumber;

		// Cache length if at least 2 points exist.
		if (points.size() >= 2)
		{
			for (int i = 0; i < (points.size() - 1); i++)
			{
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				Length += Vector3::Distance(origin, target);
			}
		}
	}

	Attractor::~Attractor()
	{
		// TODO: Parenting.
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

	AttractorCollisionData Attractor::GetCollision(const Vector3& basePos, const EulerAngles& orient, const Vector3& refPoint, float range) const
	{
		// Get proximity data.
		auto attracProx = GetProximityData(refPoint);

		// Derive segment origin and target.
		const auto& origin = Points[attracProx.SegmentIndex];
		const auto& target = Points[attracProx.SegmentIndex + 1];

		// Calculate angles.
		auto attracOrient = (Points.size() == 1) ? orient : Geometry::GetOrientToPoint(origin, target);
		short headingAngle = attracOrient.y - ANGLE(90.0f);
		short slopeAngle = attracOrient.x;

		// Determine inquiries.
		bool isIntersected = (attracProx.Distance <= range);
		bool isInFront = Geometry::IsPointInFront(basePos, attracProx.Point, orient);

		// Create and return attractor collision data.
		auto attracColl = AttractorCollisionData{};
		attracColl.AttractorPtr = this;
		attracColl.Proximity = attracProx;
		attracColl.HeadingAngle = headingAngle;
		attracColl.SlopeAngle = slopeAngle;
		attracColl.IsIntersected = isIntersected;
		attracColl.IsInFront = isInFront;
		return attracColl;
	}

	AttractorProximityData Attractor::GetProximityData(const Vector3& refPoint) const
	{
		auto attracProx = AttractorProximityData{ Points.front(), INFINITY, 0.0f, 0 };
		float lineDistFromLastClosestPoint = 0.0f;

		// Single point exists; return simple proximity data.
		if (Points.size() == 1)
		{
			attracProx.Point = Points.front();
			attracProx.Distance = Vector3::Distance(refPoint, Points.front());
			attracProx.LineDistance = 0.0f;
			attracProx.SegmentIndex = 0;
			return attracProx;
		}

		// Find closest point along attractor.
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			auto closestPoint = Geometry::GetClosestPointOnLinePerp(refPoint, origin, target);
			float dist = Vector3::Distance(refPoint, closestPoint);

			// Found new closest point; update proximity data.
			if (dist < attracProx.Distance)
			{
				lineDistFromLastClosestPoint += Vector3::Distance(origin, closestPoint);

				attracProx.Point = closestPoint;
				attracProx.Distance = dist;
				attracProx.LineDistance += lineDistFromLastClosestPoint;
				attracProx.SegmentIndex = i;

				// Restart line distance accumulation since last closest point.
				lineDistFromLastClosestPoint = Vector3::Distance(closestPoint, target);
				continue;
			}

			// Accumulate line distance since last closest point.
			lineDistFromLastClosestPoint += Vector3::Distance(origin, target);
		}

		// Return proximity data.
		return attracProx;
	}

	Vector3 Attractor::GetPointAtLineDistance(float lineDist) const
	{
		// Single point exists; return it.
		if (Points.size() == 1)
			return Points.front();
		
		// Normalize distance along attractor.
		lineDist = NormalizeLineDistance(lineDist);

		// Clamp point.
		if (lineDist <= 0.0f)
		{
			return Points.front();
		}
		else if (lineDist >= Length)
		{
			return Points.back();
		}
		
		// Find point at distance along attractor.
		float lineDistTravelled = 0.0f;
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			float segmentLength = Vector3::Distance(origin, target);
			float remainingLineDist = lineDist - lineDistTravelled;

			// Found correct segment.
			if (remainingLineDist <= segmentLength)
			{
				float alpha = remainingLineDist / segmentLength;
				return Vector3::Lerp(origin, target, alpha);
			}

			lineDistTravelled += segmentLength;
		}

		// FAILSAFE: Return end point.
		return Points.back();
	}

	unsigned int Attractor::GetSegmentIndexAtDistance(float lineDist) const
	{
		// Single segment exists; return index 0.
		if (Points.size() <= 2)
			return 0;

		// Normalize distance along attractor.
		lineDist = NormalizeLineDistance(lineDist);

		// Clamp segment index.
		if (lineDist <= 0.0f)
		{
			return 0;
		}
		else if (lineDist >= Length)
		{
			return (Points.size() - 1);
		}

		// Find segment at distance along attractor.
		float distTravelled = 0.0f;
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			distTravelled += Vector3::Distance(origin, target);
			if (distTravelled >= lineDist)
				return i;
		}

		// FAILSAFE: Return last segment index.
		return (Points.size() - 1);
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

		// Test if start and end points occupy roughly the same position.
		return (Vector3::Distance(Points.front(), Points.back()) <= EPSILON);
	}

	void Attractor::Update(const std::vector<Vector3>& points, int roomNumber)
	{
		assertion(!points.empty(), "Attempted to update invalid attractor.");

		Points = points;
		RoomNumber = roomNumber;

		// Cache length if at least 2 points exist.
		Length = 0.0f;
		if (points.size() >= 2)
		{
			for (int i = 0; i < (points.size() - 1); i++)
			{
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				Length += Vector3::Distance(origin, target);
			}
		}
	}

	void Attractor::DrawDebug() const
	{
		constexpr auto LABEL_SCALE			 = 0.75f;
		constexpr auto LABEL_OFFSET			 = Vector3(0.0f, -CLICK(0.25f), 0.0f);
		constexpr auto INDICATOR_LINE_LENGTH = 50.0f;
		constexpr auto SPHERE_SCALE			 = 15.0f;
		constexpr auto COLOR_GREEN			 = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr auto COLOR_YELLOW			 = Vector4(1.0f, 1.0f, 0.0f, 1.0f);

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
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				// Draw main line.
				g_Renderer.AddLine3D(origin, target, COLOR_YELLOW);

				auto orient = Geometry::GetOrientToPoint(origin, target);
				orient.y += ANGLE(90.0f);
				auto direction = orient.ToDirection();

				// Draw indicator lines.
				g_Renderer.AddLine3D(origin, Points[i] + (direction * INDICATOR_LINE_LENGTH), COLOR_GREEN);
				g_Renderer.AddLine3D(target, target + (direction * INDICATOR_LINE_LENGTH), COLOR_GREEN);

				auto labelPos = ((origin + target) / 2) + LABEL_OFFSET;
				auto labelPos2D = g_Renderer.GetScreenSpacePosition(labelPos);

				// Draw label.
				g_Renderer.AddString(labelString, labelPos2D, Color(PRINTSTRING_COLOR_WHITE), LABEL_SCALE, PRINTSTRING_OUTLINE);
			}

			g_Renderer.AddLine3D(Points.front(), Points.front() + (-Vector3::UnitY * INDICATOR_LINE_LENGTH), COLOR_GREEN);
			g_Renderer.AddLine3D(Points.back(), Points.back() + (-Vector3::UnitY * INDICATOR_LINE_LENGTH), COLOR_GREEN);
		}
		else if (Points.size() == 1)
		{
			auto labelPos2D = g_Renderer.GetScreenSpacePosition(Points.front());

			// Draw sphere and label.
			g_Renderer.AddSphere(Points.front(), SPHERE_SCALE, COLOR_YELLOW);
			g_Renderer.AddString(labelString, labelPos2D, Color(PRINTSTRING_COLOR_WHITE), LABEL_SCALE, PRINTSTRING_OUTLINE);
		}
	}

	float Attractor::NormalizeLineDistance(float lineDist) const
	{
		// Line distance within bounds; return unmodified distance along attractor.
		if (lineDist >= 0.0f && lineDist <= Length)
			return lineDist;

		// Attractor looped; wrap distance along attractor.
		if (IsLooped())
		{
			int sign = -std::copysign(1, lineDist);
			return (lineDist + (Length * sign));
		}
		
		// Attractor not looped; clamp distance along attractor.
		return std::clamp(lineDist, 0.0f, Length);
	}

	// Fake version.
	std::vector<const Attractor*> GetDebugAttractorPtrs(const ItemInfo& item)
	{
		constexpr auto RANGE	 = BLOCK(5);
		constexpr auto COUNT_MAX = 32;

		auto& player = GetLaraInfo(item);

		auto nearbyAttracPtrs = std::vector<const Attractor*>{};
		for (auto& attrac : player.Context.DebugAttrac.SectorAttractors)
		{
			if (nearbyAttracPtrs.size() >= COUNT_MAX)
				return nearbyAttracPtrs;

			nearbyAttracPtrs.push_back(&attrac);
			attrac.DrawDebug();
		}

		nearbyAttracPtrs.push_back(&player.Context.DebugAttrac.DebugAttractor0);
		nearbyAttracPtrs.push_back(&player.Context.DebugAttrac.DebugAttractor1);
		nearbyAttracPtrs.push_back(&player.Context.DebugAttrac.DebugAttractor2);
		return nearbyAttracPtrs;
	}

	static std::vector<const Attractor*> GetNearbyAttractorPtrs(const Vector3& refPoint, int roomNumber, float range)
	{
		constexpr auto COUNT_MAX = 32;

		auto nearbyAttracPtrMap = std::multimap<float, const Attractor*>{};

		// --------------debug
		// Get debug attractors.
		auto debugAttracPtrs = GetDebugAttractorPtrs(*LaraItem);
		for (const auto* attracPtr : debugAttracPtrs)
		{
			float dist = attracPtr->GetProximityData(refPoint).Distance;
			if (dist <= range)
				nearbyAttracPtrMap.insert({ dist, attracPtr });
		}
		// --------------

		// Get attractors in current room.
		const auto& room = g_Level.Rooms[roomNumber];
		for (const auto& attrac : room.Attractors)
		{
			float dist = attrac.GetProximityData(refPoint).Distance;
			if (dist <= range)
				nearbyAttracPtrMap.insert({ dist, &attrac });
		}

		// TODO: Check if it actually has search depth of 2.
		// Get attractors in neighboring rooms (search depth of 2).
		for (const int& subRoomNumber : room.neighbors)
		{
			const auto& subRoom = g_Level.Rooms[subRoomNumber];
			for (const auto& attrac : subRoom.Attractors)
			{
				float dist = attrac.GetProximityData(refPoint).Distance;
				if (dist <= range)
					nearbyAttracPtrMap.insert({ dist, &attrac });
			}
		}

		// TODO
		// Get bridge attractors.
		/*for (const auto& [bridgeItemNumber, attracs] : g_Level.BridgeAttractors)
		{
			const auto& bridgeItem = g_Level.Items[bridgeItemNumber];

			if (Vector3::Distance(refPoint, bridgeItem.Pose.Position.ToVector3()) > range)
				continue;
		}*/

		auto nearbyAttracPtrs = std::vector<const Attractor*>{};
		nearbyAttracPtrs.reserve(COUNT_MAX);

		// Move attractor pointers into capped vector.
		auto it = nearbyAttracPtrMap.begin();
		for (int i = 0; i < COUNT_MAX && it != nearbyAttracPtrMap.end(); i++, it++)
			nearbyAttracPtrs.push_back(std::move(it->second));

		// Return attractor pointers sorted by distance.
		return nearbyAttracPtrs;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& refPoint, float range)
	{
		auto attracPtrs = GetNearbyAttractorPtrs(refPoint, roomNumber, range);

		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(attracPtrs.size());

		for (const auto* attracPtr : attracPtrs)
		{
			auto attracColl = attracPtr->GetCollision(basePos, orient, refPoint, range);
			attracColls.push_back(attracColl);
		}

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

	static Attractor GenerateBridgeAttractor(const ItemInfo& bridgeItem)
	{
		constexpr auto TILT_STEP = CLICK(1);

		// Get bounding box.
		auto box = GameBoundingBox(&bridgeItem).ToBoundingOrientedBox(bridgeItem.Pose);

		// Determine tilt offset.
		int tiltOffset = 0;
		switch (bridgeItem.ObjectNumber)
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
		return GenerateAttractorFromPoints(points, bridgeItem.RoomNumber, AttractorType::Edge);
	}

	std::optional<Attractor> GenerateSectorAttractor(const CollisionResult& pointColl)
	{
		// Invalid sector; return nullopt.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return std::nullopt;

		// Generate and return bridge attractor.
		if (pointColl.Position.Bridge >= 0)
		{
			const auto& bridgeItem = g_Level.Items[pointColl.Position.Bridge];
			return GenerateBridgeAttractor(bridgeItem);
		}

		// Generate and return floor attractor.
		auto points = pointColl.BottomBlock->GetSurfaceVertices(pointColl.Coordinates.x, pointColl.Coordinates.z, true);
		return GenerateAttractorFromPoints(points, pointColl.RoomNumber, AttractorType::Edge);
	}
}
