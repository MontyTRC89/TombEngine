#include "framework.h"
#include "Game/collision/Attractor.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/collision/Point.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/trutils.h"
#include "Renderer/Renderer.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Attractor
{
	AttractorObject::AttractorObject(AttractorType type, const Vector3& pos, const Quaternion& orient, int roomNumber, const std::vector<Vector3>& points)
	{
		TENAssert(!points.empty(), "Attempted to initialize attractor with 0 points.");

		_type = type;
		_position = pos;
		_orientation = orient;
		_roomNumber = roomNumber;
		_path.Points = points;

		if (_path.Points.size() == 1)
		{
			_path.SegmentLengths.push_back(0.0f);
			_path.Length = 0.0f;
		}
		else
		{
			for (int i = 0; i < GetSegmentCount(); i++)
			{
				const auto& origin = _path.Points[i];
				const auto& target = _path.Points[i + 1];

				float segmentLength = Vector3::Distance(origin, target);

				_path.SegmentLengths.push_back(segmentLength);
				_path.Length += segmentLength;
			}
		}

		_aabb = Geometry::GetBoundingBox(points);
	}

	AttractorObject::~AttractorObject()
	{
		DetachAllPlayers();
	}

	int AttractorObject::GetRoomNumber() const
	{
		return _roomNumber;
	}

	AttractorType AttractorObject::GetType() const
	{
		return _type;
	}

	float AttractorObject::GetLength() const
	{
		return _path.Length;
	}

	const BoundingBox& AttractorObject::GetLocalAabb() const
	{
		return _aabb;
	}

	BoundingOrientedBox AttractorObject::GetWorldObb() const
	{
		return BoundingOrientedBox(_position + _aabb.Center, _aabb.Extents, _orientation);
	}

	unsigned int AttractorObject::GetSegmentCount() const
	{
		return std::max<unsigned int>((int)_path.Points.size() - 1, 1);
	}

	unsigned int AttractorObject::GetSegmentIDAtPathDistance(float pathDist) const
	{
		// Single segment exists; return segment ID 0.
		if (GetSegmentCount() == 1)
			return 0;

		// Normalize distance along attractor.
		pathDist = NormalizePathDistance(pathDist);

		// Path distance is on attractor edge; return clamped segment ID.
		if (pathDist <= 0.0f)
		{
			return 0;
		}
		else if (pathDist >= _path.Length)
		{
			return (GetSegmentCount() - 1);
		}

		// Find segment at distance along attractor.
		float pathDistTraveled = 0.0f;
		for (int i = 0; i < GetSegmentCount(); i++)
		{
			// Accumulate distance traveled along attractor.
			pathDistTraveled += _path.SegmentLengths[i];

			// Segment found; return segment ID.
			if (pathDistTraveled >= pathDist)
				return i;
		}

		// FAILSAFE: Return end segment ID.
		return (GetSegmentCount() - 1);
	}

	Vector3 AttractorObject::GetIntersectionAtPathDistance(float pathDist) const
	{
		auto transformMatrix = GetTransformMatrix();

		// Single point exists; return simple intersection.
		if (_path.Points.size() == 1)
			return Vector3::Transform(_path.Points.front(), transformMatrix);

		// Normalize distance along attractor.
		pathDist = NormalizePathDistance(pathDist);

		// Line distance is outside attractor; return clamped intersection.
		if (pathDist <= 0.0f)
		{
			return Vector3::Transform(_path.Points.front(), transformMatrix);
		}
		else if (pathDist >= _path.Length)
		{
			return Vector3::Transform(_path.Points.back(), transformMatrix);
		}

		// Find intersection at distance along attractor.
		float pathDistTraveled = 0.0f;
		for (int i = 0; i < GetSegmentCount(); i++)
		{
			float segmentLength = _path.SegmentLengths[i];
			float remainingPathDist = pathDist - pathDistTraveled;

			// Found segment of distance along attractor; return intersection.
			if (remainingPathDist <= segmentLength)
			{
				// Get segment points.
				const auto& origin = _path.Points[i];
				const auto& target = _path.Points[i + 1];

				float alpha = remainingPathDist / segmentLength;
				return Vector3::Transform(Vector3::Lerp(origin, target, alpha), transformMatrix);
			}

			// Accumulate distance traveled along attractor.
			pathDistTraveled += segmentLength;
		}

		// FAILSAFE: Return end point.
		return Vector3::Transform(_path.Points.back(), transformMatrix);
	}

	std::optional<AttractorCollisionData> AttractorObject::GetCollision(const BoundingSphere& sphere, short headingAngle, unsigned int segmentID, const Vector3& axis)
	{
		constexpr auto HEADING_ANGLE_OFFSET = ANGLE(-90.0);

		// FAILSAFE: Handle out-of-bounds segment ID.
		if (segmentID >= GetSegmentCount())
		{
			TENLog("Attempted to get attractor collision for invalid segment ID " + std::to_string(segmentID) + ".", LogLevel::Warning);
			segmentID = 0;
		}

		auto transformMatrix = GetTransformMatrix();
		auto localSphere = BoundingSphere(Vector3::Transform(sphere.Center, transformMatrix.Invert()), sphere.Radius);

		// Determine if attractor is path or single point.
		bool isPath = (_path.Points.size() > 1);

		// Test sphere-segment intersection.
		float dist = isPath ?
			Geometry::GetDistanceToLine(localSphere.Center, _path.Points[segmentID], _path.Points[segmentID + 1]) :
			Vector3::Distance(localSphere.Center, _path.Points[segmentID]);
		if (dist > localSphere.Radius)
			return std::nullopt;

		// Calculate intersection.
		auto intersect = isPath ?
			Geometry::GetClosestPointOnLinePerp(localSphere.Center, _path.Points[segmentID], _path.Points[segmentID + 1], axis) :
			_path.Points[segmentID];

		// Calculate path distance.
		float pathDist = 0.0f;
		if (isPath)
		{
			// Accumulate distance traveled along attractor toward intersection.
			for (unsigned int i = 0; i < segmentID; i++)
			{
				float segmentLength = _path.SegmentLengths[i];
				pathDist += segmentLength;
			}

			// Accumulate final distance traveled along attractor toward intersection.
			pathDist += Vector3::Distance(_path.Points[segmentID], intersect);
		}

		// TODO: Check.
		// TODO: Consider axis.
		// Calculate orientations.
		auto refOrient = EulerAngles(0, headingAngle, 0) - EulerAngles(_orientation);
		auto segmentOrient = isPath ?
			Geometry::GetOrientToPoint(_path.Points[segmentID], _path.Points[segmentID + 1]) + EulerAngles(_orientation) :
			refOrient;

		// Create attractor collision.
		auto attracColl = AttractorCollisionData{};
		attracColl.Attractor = this;
		attracColl.Intersection = Vector3::Transform(intersect, transformMatrix);
		attracColl.Distance2D = Vector2::Distance(Vector2(localSphere.Center.x, localSphere.Center.z), Vector2(intersect.x, intersect.z));
		attracColl.Distance3D = Vector3::Distance(localSphere.Center, intersect);
		attracColl.PathDistance = pathDist;
		attracColl.SegmentID = segmentID;
		attracColl.HeadingAngle = segmentOrient.y + HEADING_ANGLE_OFFSET;
		attracColl.SlopeAngle = segmentOrient.x;
		attracColl.IsInFront = Geometry::IsPointInFront(localSphere.Center, intersect, refOrient);
		return attracColl;
	}

	AttractorCollisionData AttractorObject::GetCollision(float pathDist, short headingAngle, const Vector3& axis)
	{
		constexpr auto SPHERE_RADIUS = 1.0f;

		auto sphere = BoundingSphere(GetIntersectionAtPathDistance(pathDist), SPHERE_RADIUS);
		unsigned int segmentID = GetSegmentIDAtPathDistance(pathDist);
		return *GetCollision(sphere, headingAngle, segmentID, axis);
	}
	
	void AttractorObject::SetPosition(const Vector3& pos)
	{
		_position = pos;
	}

	void AttractorObject::SetOrientation(const Quaternion& orient)
	{
		_orientation = orient;
	}

	bool AttractorObject::IsLoop() const
	{
		// Single segment exists; loop not possible.
		if (GetSegmentCount() == 1)
			return false;

		// Test if start and end points occupy same approximate position.
		float distSqr = Vector3::DistanceSquared(_path.Points.front(), _path.Points.back());
		return (distSqr <= EPSILON);
	}

	void AttractorObject::AttachPlayer(ItemInfo& playerItem)
	{
		if (!playerItem.IsLara())
		{
			TENLog("Attempted to attach non-player item to attractor.", LogLevel::Warning);
			return;
		}

		_playerItemNumbers.insert(playerItem.Index);
	}

	void AttractorObject::DetachPlayer(ItemInfo& playerItem)
	{
		_playerItemNumbers.erase(playerItem.Index);
	}

	void AttractorObject::DetachAllPlayers()
	{
		// TODO: Crashes trying to loop?

		for (int itemNumber : _playerItemNumbers)
		{
			auto& playerItem = g_Level.Items[itemNumber];
			auto& player = GetLaraInfo(playerItem);

			if (player.Context.Attractor.Attractor != nullptr)
				player.Context.Attractor.Detach(playerItem);
		}
	}

	void AttractorObject::DrawDebug(unsigned int segmentID) const
	{
		constexpr auto LABEL_OFFSET				= Vector3(0.0f, -CLICK(0.5f), 0.0f);
		constexpr auto INDICATOR_LINE_LENGTH	= BLOCK(1 / 20.0f);
		constexpr auto SPHERE_RADIUS			= BLOCK(1 / 52.0f);
		constexpr auto COLOR_YELLOW_OPAQUE		= Color(1.0f, 1.0f, 0.4f);
		constexpr auto COLOR_YELLOW_TRANSLUCENT = Color(1.0f, 1.0f, 0.4f, 0.3f);
		constexpr auto COLOR_GREEN				= Color(0.4f, 1.0f, 0.4f);
		constexpr auto BOX_COLOR				= Color(1.0f, 1.0f, 1.0f, 0.1f);

		auto getLabelScale = [](const Vector3& cameraPos, const Vector3& labelPos)
		{
			constexpr auto RANGE		   = BLOCK(5);
			constexpr auto LABEL_SCALE_MAX = 0.8f;
			constexpr auto LABEL_SCALE_MIN = 0.4f;

			float cameraDist = Vector3::Distance(cameraPos, labelPos);
			float alpha = cameraDist / RANGE;
			return Lerp(LABEL_SCALE_MAX, LABEL_SCALE_MIN, alpha);
		};

		auto transformMatrix = GetTransformMatrix();

		// Determine label string.
		auto labelString = std::string();
		switch (_type)
		{
		case AttractorType::Edge:
			labelString = "Edge";
			break;

		case AttractorType::WallEdge:
			labelString = "Wall Edge";
			break;

		default:
			labelString = "Unknown attractor";
			break;
		}

		// Draw debug elements.
		if (_path.Points.size() == 1)
		{
			// Get segment position.
			auto pos = Vector3::Transform(_path.Points.front(), transformMatrix);

			// Draw sphere.
			DrawDebugSphere(pos, SPHERE_RADIUS, COLOR_YELLOW_TRANSLUCENT, RendererDebugPage::AttractorStats, false);

			// Determine label parameters.
			auto labelPos2D = Get2DPosition(pos);
			float labelScale = getLabelScale(Camera.pos.ToVector3(), pos);

			// Draw label.
			if (labelPos2D.has_value())
				DrawDebugString(labelString, *labelPos2D, COLOR_YELLOW_OPAQUE, labelScale, RendererDebugPage::AttractorStats);
		}
		else
		{
			// Get segment points.
			const auto& origin = Vector3::Transform(_path.Points[segmentID], transformMatrix);
			const auto& target = Vector3::Transform(_path.Points[segmentID + 1], transformMatrix);

			DrawDebugLine(origin, LaraItem->Pose.Position.ToVector3(), Color());

			// Draw main line.
			DrawDebugLine(origin, target, COLOR_YELLOW_OPAQUE, RendererDebugPage::AttractorStats);

			auto orient = EulerAngles(0, Geometry::GetOrientToPoint(origin, target).y + ANGLE(90.0f), 0);
			auto dir = orient.ToDirection();

			// Draw segment heading indicator lines.
			DrawDebugLine(origin, Geometry::TranslatePoint(origin, dir, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);
			DrawDebugLine(target, Geometry::TranslatePoint(target, dir, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);

			// Determine label parameters.
			auto labelPos = ((origin + target) / 2) + LABEL_OFFSET;
			auto labelPos2D = Get2DPosition(labelPos);
			float labelScale = getLabelScale(Camera.pos.ToVector3(), labelPos);

			// Draw label.
			if (labelPos2D.has_value())
				DrawDebugString(labelString, *labelPos2D, COLOR_YELLOW_OPAQUE, labelScale, RendererDebugPage::AttractorStats);

			// Draw start indicator line.
			if (segmentID == 0)
				DrawDebugLine(origin, Geometry::TranslatePoint(origin, -Vector3::UnitY, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);
			
			// Draw end indicator line.
			if (segmentID == (GetSegmentCount() - 1))
				DrawDebugLine(target, Geometry::TranslatePoint(target, -Vector3::UnitY, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);

			// Draw local AABB.
			/*auto obb = BoundingOrientedBox(_position + _aabb.Center, _aabb.Extents, _orientation);
			DrawDebugBox(obb, BOX_COLOR, RendererDebugPage::AttractorStats);*/

			// Draw local segment AABBs.
			/*for (const auto& aabb : _path.SegmentAabbs)
			{
				auto obb = BoundingOrientedBox(_position + _aabb.Center, _aabb.Extents, _orientation);
				DrawDebugBox(obb, BOX_COLOR, RendererDebugPage::AttractorStats);
			}*/
		}
	}

	void AttractorObject::DrawDebug() const
	{
		for (int i = 0; i < GetSegmentCount(); i++)
			DrawDebug(i);
	}

	float AttractorObject::NormalizePathDistance(float pathDist) const
	{
		// Distance along attractor within bounds; return it.
		if (pathDist >= 0.0f && pathDist <= _path.Length)
			return pathDist;

		// Wrap or clamp distance along attractor.
		return (IsLoop() ? fmod(pathDist + _path.Length, _path.Length) : std::clamp(pathDist, 0.0f, _path.Length));
	}

	Matrix AttractorObject::GetTransformMatrix() const
	{
		auto translationMatrix = Matrix::CreateTranslation(_position);
		auto rotMatrix = Matrix::CreateFromQuaternion(_orientation);
		return (rotMatrix * translationMatrix);
	}

	static std::vector<AttractorObject*> GetBoundedAttractors(const BoundingSphere& sphere, int roomNumber)
	{
		// Debug
		auto getDebugAttractors = []()
		{
			auto& player = GetLaraInfo(*LaraItem);

			auto debugAttracs = std::vector<AttractorObject*>{};
			debugAttracs.push_back(&*player.Context.DebugAttracs.Attrac0);
			debugAttracs.push_back(&*player.Context.DebugAttracs.Attrac1);
			debugAttracs.push_back(&*player.Context.DebugAttracs.Attrac2);

			for (auto& attrac : player.Context.DebugAttracs.Attracs)
				debugAttracs.push_back(&attrac);

			return debugAttracs;
		};

		DrawDebugSphere(sphere.Center, sphere.Radius, Vector4::One, RendererDebugPage::AttractorStats);

		auto boundedAttracs = std::vector<AttractorObject*>{};

		// TEMP
		// Collect debug attractors.
		auto debugAttracs = getDebugAttractors();
		for (auto* attrac : debugAttracs)
		{
			if (sphere.Intersects(attrac->GetWorldObb()))
				boundedAttracs.push_back(attrac);
		}

		// 1) Collect bounded room attractors.
		auto& room = g_Level.Rooms[roomNumber];
		for (int neighborRoomNumber : room.NeighborRoomNumbers)
		{
			// Check if room is active.
			auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
			if (!neighborRoom.Active())
				continue;

			auto attracs = neighborRoom.Attractors.GetBoundedAttractors(sphere);
			boundedAttracs.insert(boundedAttracs.end(), attracs.begin(), attracs.end());
		}

		auto visitedBridgeItemNumbers = std::set<int>{};

		// 2) Collect bounded bridge attractors.
		unsigned int sectorSearchDepth = (int)ceil(sphere.Radius / BLOCK(1));
		auto sectors = GetNeighborSectors((Vector3)sphere.Center, roomNumber, sectorSearchDepth);
		for (const auto* sector : sectors)
		{
			// Test if sphere intersects sector.
			if (!sphere.Intersects(sector->Box))
				continue;

			// Run through bridges in sector.
			for (int bridgeItemNumber : sector->BridgeItemNumbers)
			{
				// Test if bridge was already visited.
				if (Contains(visitedBridgeItemNumbers, bridgeItemNumber))
					continue;
				visitedBridgeItemNumbers.insert(bridgeItemNumber);

				auto& bridgeItem = g_Level.Items[bridgeItemNumber];
				auto& bridge = GetBridgeObject(bridgeItem);

				// Test if sphere intersects bridge attractor OBB.
				if (!sphere.Intersects(bridge.GetAttractor().GetWorldObb()))
					continue;

				boundedAttracs.push_back(&bridge.GetAttractor());
			}
		}

		// 3) Return bounded attractors.
		return boundedAttracs;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle, float radius,
															   const Vector3& axis)
	{
		constexpr auto COLL_COUNT_MAX = 64;

		// 1) Get bounded attractors.
		auto sphere = BoundingSphere(pos, radius);
		auto attracs = GetBoundedAttractors(sphere, roomNumber);

		// 2) Collect attractor collisions.
		auto attracColls = std::vector<AttractorCollisionData>{};
		for (auto* attrac : attracs)
		{
			// Run through segments.
			for (int i = 0; i < attrac->GetSegmentCount(); i++)
			{
				auto attracColl = attrac->GetCollision(sphere, headingAngle, i);
				if (attracColl.has_value())
					attracColls.push_back(std::move(*attracColl));
			}
		}

		// 3) Sort collisions by 2D then 3D distance.
		std::sort(
			attracColls.begin(), attracColls.end(),
			[](const auto& attracColl0, const auto& attracColl1)
			{
				if (attracColl0.Distance2D == attracColl1.Distance2D)
					return (attracColl0.Distance3D < attracColl1.Distance3D);
				
				return (attracColl0.Distance2D < attracColl1.Distance2D);
			});

		// 4) Trim collection.
		if (attracColls.size() > COLL_COUNT_MAX)
			attracColls.resize(COLL_COUNT_MAX);

		// 5) Return attractor collisions in capped vector sorted by 2D then 3D distance.
		return attracColls;
	}

	std::vector<AttractorCollisionData> GetAttractorCollisions(const Vector3& pos, int roomNumber, short headingAngle,
															   float forward, float down, float right, float radius,
															   const Vector3& axis)
	{
		auto relOffset = Vector3(right, down, forward);
		auto rotMatrix = AxisAngle(axis, headingAngle).ToRotationMatrix();
		auto probePos = pos + Vector3::Transform(relOffset, rotMatrix);
		int probeRoomNumber = GetPointCollision(pos, roomNumber, headingAngle, forward, down, right).GetRoomNumber();

		return GetAttractorCollisions(probePos, probeRoomNumber, headingAngle, radius);
	}
	
	void DrawNearbyAttractors(const Vector3& pos, int roomNumber, short headingAngle)
	{
		constexpr auto RADIUS = BLOCK(5);

		//auto& room = g_Level.Rooms[roomNumber];
		//const auto& attracs = room.Attractors.GetAttractors();
		//for (const auto& attrac : attracs)
		//	attrac.DrawDebug();

		auto uniqueAttracs = std::set<AttractorObject*>{};

		auto attracColls = GetAttractorCollisions(pos, roomNumber, headingAngle, 0.0f, 0.0f, 0.0f, RADIUS);
		for (const auto& attracColl : attracColls)
		{
			uniqueAttracs.insert(attracColl.Attractor);
			attracColl.Attractor->DrawDebug(attracColl.SegmentID);
		}

		if (g_Renderer.GetDebugPage() == RendererDebugPage::AttractorStats)
		{
			PrintDebugMessage("Nearby attractors: %d", (int)uniqueAttracs.size());
			PrintDebugMessage("Nearby attractor segments: %d", (int)attracColls.size());
		}
	}
}
