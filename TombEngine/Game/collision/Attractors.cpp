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
	Attractor::Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber)
	{
		Type = type;
		Points = points;
		RoomNumber = roomNumber;

		if (points.size() > 1)
		{
			for (int i = 0; i < (points.size() - 1); i++)
			{
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				Length += Vector3::Distance(origin, target);
			}
		}
	}

	AttractorType Attractor::GetType() const
	{
		return Type;
	}

	std::vector<Vector3> Attractor::GetPoints() const
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
		static const auto ATTRAC_COLL_DEFAULT = AttractorCollisionData{};

		if (Points.empty())
		{
			TENLog(std::string("GetAttractorCollision(): attractor undefined."), LogLevel::Warning);
			return ATTRAC_COLL_DEFAULT;
		}

		// Get attractor proximity data.
		auto attracProx = GetProximityData(refPoint);

		// Calculate angles.
		auto attracOrient = (Points.size() == 1) ?
			orient : Geometry::GetOrientToPoint(Points[attracProx.SegmentIndex], Points[attracProx.SegmentIndex + 1]);
		short headingAngle = attracOrient.y - ANGLE(90.0f);
		short slopeAngle = attracOrient.x;

		// Determine inquiries.
		bool isIntersected = (attracProx.Distance <= range);
		bool isInFront = Geometry::IsPointInFront(basePos, attracProx.ClosestPoint, orient);

		// Create new attractor collision.
		auto attracColl = ATTRAC_COLL_DEFAULT;

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
		static const auto ATTRAC_PROX_DEFAULT = AttractorProximityData{};

		// Attractor has no points; return default attractor proximity data.
		if (Points.empty())
		{
			TENLog(std::string("GetTargetData(): attractor points undefined."), LogLevel::Warning);
			return ATTRAC_PROX_DEFAULT;
		}

		// Attractor is single point; return simple attractor proximity data.
		if (Points.size() == 1)
			return AttractorProximityData{ Points[0], Vector3::Distance(refPoint, Points[0]), 0 };

		auto closestPoint = Points[0];
		float closestDist = INFINITY;
		unsigned int segmentIndex = 0;

		// Find closest point on attractor.
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			auto closestPointPerp = Geometry::GetClosestPointOnLinePerp(refPoint, origin, target);
			float distance = Vector3::Distance(refPoint, closestPointPerp);

			if (distance < closestDist)
			{
				closestPoint = closestPointPerp;
				closestDist = distance;
				segmentIndex = i;
			}
		}

		// Return attractor proximity data.
		float distFromStart = GetDistanceAtPoint(closestPoint, segmentIndex);
		return AttractorProximityData{ closestPoint, closestDist, distFromStart, segmentIndex };
	}

	Vector3 Attractor::GetPointAtDistance(float distAlongLine) const
	{
		// Attractor has no points; return world origin.
		if (Points.empty())
		{
			TENLog(std::string("GetPointAtDistance(): attractor points undefined."), LogLevel::Warning);
			return Vector3::Zero;
		}

		// Attractor is single point; return it.
		if (Points.size() == 1)
			return Points[0];

		// Clamp point according to attractor length.
		if (distAlongLine <= 0.0f)
		{
			return Points[0];
		}
		else if (distAlongLine >= Length)
		{
			return Points.back();
		}

		// Find point along attractor line at distance from start.
		float distTravelled = 0.0f;
		for (int i = 0; i < (Points.size() - 1); i++)
		{
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			float segmentLength = Vector3::Distance(origin, target);
			float remainingDist = distAlongLine - distTravelled;

			if (remainingDist <= segmentLength)
			{
				float alpha = remainingDist / segmentLength;
				auto pointOnLine = Vector3::Lerp(origin, target, alpha);
				return pointOnLine;
			}

			distTravelled += segmentLength;
		}

		return Points.back();
	}

	float Attractor::GetDistanceAtPoint(const Vector3& pointOnLine, unsigned int segmentIndex) const 
	{
		// Segment index out of range; return attractor length.
		if (segmentIndex >= Points.size())
		{
			TENLog(std::string("GetDistanceAtPoint(): attractor segment index out of range."), LogLevel::Warning);
			return Length;
		}

		// Calculate distance along attractor to point.
		float distAlongLine = 0.0f;
		for (int i = 0; i <= segmentIndex; i++)
		{
			const auto& origin = Points[i];
			const auto& target = Points[i + 1];

			if (i != segmentIndex)
			{
				distAlongLine += Vector3::Distance(origin, target);
				continue;
			}

			float pointToAttractorThreshold = Geometry::GetDistanceToLine(pointOnLine, origin, target);
			if (pointToAttractorThreshold > SQRT_2)
				TENLog(std::string("GetDistanceAtPoint(): point beyond attractor."), LogLevel::Warning);

			distAlongLine += Vector3::Distance(origin, pointOnLine);
		}

		return distAlongLine;
	}

	bool Attractor::IsEdge() const
	{
		return (Type == AttractorType::Edge);
	}

	void Attractor::Update(const std::vector<Vector3>& points, int roomNumber)
	{
		Points = points;
		RoomNumber = roomNumber;

		if (points.size() > 1)
		{
			for (int i = 0; i < (points.size() - 1); i++)
			{
				const auto& origin = Points[i];
				const auto& target = Points[i + 1];

				Length += Vector3::Distance(origin, target);
			}
		}
		else
		{
			Length = 0.0f;
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

		// Draw attractor debug elements.
		if (Points.size() > 1)
		{
			for (int i = 0; i < (Points.size() - 1); i++)
			{
				auto orient = Geometry::GetOrientToPoint(Points[i], Points[i + 1]);
				orient.y += ANGLE(90.0f);
				auto direction = orient.ToDirection();

				auto labelPos = ((Points[i] + Points[i + 1]) / 2) + LABEL_OFFSET;
				auto labelPos2D = g_Renderer.GetScreenSpacePosition(labelPos);

				// Draw main line.
				g_Renderer.AddLine3D(Points[i], Points[i + 1], COLOR_YELLOW);

				// Draw indicator lines.
				g_Renderer.AddLine3D(Points[i], Points[i] + (direction * INDICATOR_LINE_LENGTH), COLOR_GREEN);
				g_Renderer.AddLine3D(Points[i + 1], Points[i + 1] + (direction * INDICATOR_LINE_LENGTH), COLOR_GREEN);
				g_Renderer.AddLine3D(Points[0], Points[0] + (-Vector3::UnitY * INDICATOR_LINE_LENGTH), COLOR_GREEN);
				g_Renderer.AddLine3D(Points.back(), Points.back() + (-Vector3::UnitY * INDICATOR_LINE_LENGTH), COLOR_GREEN);

				// Draw attractor label.
				g_Renderer.AddString(labelString, labelPos2D, Color(PRINTSTRING_COLOR_WHITE), LABEL_SCALE, 0);
			}
		}
		else if (Points.size() == 1)
		{
			auto labelPos2D = g_Renderer.GetScreenSpacePosition(Points[0]);

			// Draw sphere and label.
			g_Renderer.AddSphere(Points[0], SPHERE_SCALE, COLOR_YELLOW);
			g_Renderer.AddString(labelString, labelPos2D, Color(PRINTSTRING_COLOR_WHITE), LABEL_SCALE, 0);
		}
	}

	static std::vector<const Attractor*> GetNearbyAttractorPtrs(const Vector3& refPoint, int roomNumber, float range)
	{
		constexpr auto COUNT_MAX = 32;

		auto nearbyAttracPtrMap = std::multimap<float, const Attractor*>{};

		// Get attractors in current room.
		const auto& room = g_Level.Rooms[roomNumber];
		for (const auto& attrac : room.Attractors)
		{
			auto attracProximity = attrac.GetProximityData(refPoint);
			if (attracProximity.Distance <= range)
				nearbyAttracPtrMap.insert({ attracProximity.Distance, &attrac });
		}

		// Get attractors in neighboring rooms (search depth of 2).
		for (const int& subRoomNumber : room.neighbors)
		{
			const auto& subRoom = g_Level.Rooms[subRoomNumber];
			for (const auto& attrac : subRoom.Attractors)
			{
				auto attracPointData = attrac.GetProximityData(refPoint);
				if (attracPointData.Distance <= range)
					nearbyAttracPtrMap.insert({ attracPointData.Distance, &attrac });
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

		// Move pointers into vector.
		auto it = nearbyAttracPtrMap.begin();
		for (int i = 0; i < COUNT_MAX && it != nearbyAttracPtrMap.end(); i++, it++)
			nearbyAttracPtrs.push_back(std::move(it->second));

		return nearbyAttracPtrs;
	}

	// Fake version.
	std::vector<const Attractor*> GetDebugAttractorPtrs(const ItemInfo& item)
	{
		constexpr auto RANGE	 = BLOCK(5);
		constexpr auto COUNT_MAX = 32;

		auto& player = GetLaraInfo(item);

		auto nearbyAttracPtrs = std::vector<const Attractor*>{};
		for (auto& attrac : player.Context.HandsAttractor.SectorAttractors)
		{
			assertion(nearbyAttracPtrs.size() <= COUNT_MAX, "Nearby attractor pointer collection overflow.");
			if (nearbyAttracPtrs.size() == COUNT_MAX)
				return nearbyAttracPtrs;

			nearbyAttracPtrs.push_back(&attrac);
			attrac.DrawDebug();
		}
		
		nearbyAttracPtrs.push_back(&player.Context.HandsAttractor.DebugAttractor0);
		nearbyAttracPtrs.push_back(&player.Context.HandsAttractor.DebugAttractor1);
		nearbyAttracPtrs.push_back(&player.Context.HandsAttractor.DebugAttractor2);
		return nearbyAttracPtrs;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& basePos, int roomNumber, const EulerAngles& orient,
															   const Vector3& refPoint, float range)
	{
		auto attracPtrs = GetNearbyAttractorPtrs(refPoint, roomNumber, range);

		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(attracPtrs.size());

		for (const auto* attrac : attracPtrs)
		{
			auto attracColl = attrac->GetCollision(basePos, orient, refPoint, range);
			attracColls.push_back(attracColl);
		}

		return attracColls;
	}
	
	std::vector<AttractorCollisionData> GetAttractorCollisions(const ItemInfo& item, const Vector3& refPoint, float range)
	{
		// TODO: This call will do actual probing. For now, using debug attractors.
		//return GetAttractorCollisions(item.Pose.Position.ToVector3(), item.RoomNumber, item.Pose.Orientation, refPoint, range);
		
		// Get debug attractor pointers.
		auto attracPtrs = GetDebugAttractorPtrs(item);

		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(attracPtrs.size());

		for (const auto* attrac : attracPtrs)
		{
			auto attracColl = attrac->GetCollision(item.Pose.Position.ToVector3(), item.Pose.Orientation, refPoint, range);
			attracColls.push_back(attracColl);
		}

		return attracColls;
	}
	
	// Fake version.
	std::vector<AttractorCollisionData> GetAttractorCollisions(const std::vector<const Attractor*>& attracPtrs,
															   const ItemInfo& item, const Vector3& refPoint, float range)
	{
		auto attracColls = std::vector<AttractorCollisionData>{};
		attracColls.reserve(attracPtrs.size());

		for (const auto* attrac : attracPtrs)
		{
			auto attracColl = attrac->GetCollision(item.Pose.Position.ToVector3(), item.Pose.Orientation, refPoint, range);
			attracColls.push_back(attracColl);
		}

		return attracColls;
	}

	Attractor GenerateAttractorFromPoints(std::vector<Vector3> points, int roomNumber, AttractorType type, bool isClosedLoop)
	{
		static const auto DEFAULT_ATTRACTOR = Attractor{};

		// No points; return default.
		if (points.empty())
			return DEFAULT_ATTRACTOR;

		// Add point to create loop (if applicable).
		if (isClosedLoop)
			points.push_back(points[0]);

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
