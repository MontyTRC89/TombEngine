#include "framework.h"
#include "Game/collision/Attractor.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Collision::Attractor
{
	Attractor::Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber)
	{
		assertion(!points.empty(), "Attempted to initialize invalid attractor.");
		
		_type = type;
		_points = points;
		_roomNumber = roomNumber;
		CacheSegmentLengths();
		CacheLength();
		CacheBox();
	}

	Attractor::~Attractor()
	{
		DetachAllPlayers();
	}

	AttractorType Attractor::GetType() const
	{
		return _type;
	}

	const std::vector<Vector3>& Attractor::GetPoints() const
	{
		return _points;
	}

	int Attractor::GetRoomNumber() const
	{
		return _roomNumber;
	}

	const std::vector<float>& Attractor::GetSegmentLengths() const
	{
		return _segmentLengths;
	}

	float Attractor::GetLength() const
	{
		return _length;
	}

	const BoundingBox& Attractor::GetBox() const
	{
		return _box;
	}

	bool Attractor::IsLooped() const
	{
		// Single segment exists; loop not possible.
		if (GetSegmentCount() == 1)
			return false;

		// Test if start and end points occupy same approximate position.
		float distSqr = Vector3::DistanceSquared(_points.front(), _points.back());
		return (distSqr <= EPSILON);
	}

	unsigned int Attractor::GetSegmentCount() const
	{
		return std::max<unsigned int>(_points.size() - 1, 1);
	}

	unsigned int Attractor::GetSegmentIDAtChainDistance(float chainDist) const
	{
		// Single segment exists; return segment ID 0.
		if (GetSegmentCount() == 1)
			return 0;

		// Normalize distance along attractor.
		chainDist = NormalizeChainDistance(chainDist);

		// Chain distance is on attractor edge; return clamped segment ID.
		if (chainDist <= 0.0f)
		{
			return 0;
		}
		else if (chainDist >= _length)
		{
			return (GetSegmentCount() - 1);
		}

		// Find segment at distance along attractor.
		float chainDistTraveled = 0.0f;
		for (int i = 0; i < GetSegmentCount(); i++)
		{
			// Accumulate distance traveled along attractor.
			chainDistTraveled += _segmentLengths[i];

			// Segment found; return segment ID.
			if (chainDistTraveled >= chainDist)
				return i;
		}

		// FAILSAFE: Return end segment ID.
		return (GetSegmentCount() - 1);
	}

	Vector3 Attractor::GetIntersectionAtChainDistance(float chainDist) const
	{
		// Single point exists; return simple intersection.
		if (_points.size() == 1)
			return _points.front();

		// Normalize distance along attractor.
		chainDist = NormalizeChainDistance(chainDist);

		// Line distance is outside attractor; return clamped intersection.
		if (chainDist <= 0.0f)
		{
			return _points.front();
		}
		else if (chainDist >= _length)
		{
			return _points.back();
		}

		// Find intersection at distance along attractor.
		float chainDistTraveled = 0.0f;
		for (int i = 0; i < GetSegmentCount(); i++)
		{
			float segmentLength = _segmentLengths[i];
			float remainingChainDist = chainDist - chainDistTraveled;

			// Found segment of distance along attractor; return intersection.
			if (remainingChainDist <= segmentLength)
			{
				// Get segment points.
				const auto& origin = _points[i];
				const auto& target = _points[i + 1];

				float alpha = remainingChainDist / segmentLength;
				return Vector3::Lerp(origin, target, alpha);
			}

			// Accumulate distance traveled along attractor.
			chainDistTraveled += segmentLength;
		}

		// FAILSAFE: Return end point.
		return _points.back();
	}

	void Attractor::Update(const std::vector<Vector3>& points, int roomNumber)
	{
		if (points.empty())
			TENLog("Attempted to update attractor to invalid state.", LogLevel::Warning);

		_points = points;
		_roomNumber = roomNumber;
		CacheSegmentLengths();
		CacheLength();
		CacheBox();
	}

	void Attractor::AttachPlayer(ItemInfo& playerItem)
	{
		if (!playerItem.IsLara())
		{
			TENLog("Attempted to attach non-player item to attractor.", LogLevel::Warning);
			return;
		}

		_attachedPlayerItemNumbers.insert(playerItem.Index);
	}

	void Attractor::DetachPlayer(ItemInfo& playerItem)
	{
		_attachedPlayerItemNumbers.erase(playerItem.Index);
	}

	void Attractor::DetachAllPlayers()
	{
		// TODO: Crashes trying to loop?
		return;

		for (int itemNumber : _attachedPlayerItemNumbers)
		{
			auto& playerItem = g_Level.Items[itemNumber];
			auto& player = GetLaraInfo(playerItem);

			if (player.Context.Attractor.Ptr != nullptr)
				player.Context.Attractor.Detach(playerItem);
		}
	}

	void Attractor::DrawDebug(unsigned int segmentID) const
	{
		constexpr auto LABEL_OFFSET				= Vector3(0.0f, -CLICK(0.5f), 0.0f);
		constexpr auto INDICATOR_LINE_LENGTH	= BLOCK(1 / 20.0f);
		constexpr auto SPHERE_RADIUS			= BLOCK(1 / 52.0f);
		constexpr auto COLOR_YELLOW_OPAQUE		= Color(1.0f, 1.0f, 0.4f);
		constexpr auto COLOR_YELLOW_TRANSLUCENT = Color(1.0f, 1.0f, 0.4f, 0.3f);
		constexpr auto COLOR_GREEN				= Color(0.4f, 1.0f, 0.4f);

		auto getLabelScale = [](const Vector3& cameraPos, const Vector3& labelPos)
		{
			constexpr auto RANGE		   = BLOCK(5);
			constexpr auto LABEL_SCALE_MAX = 0.8f;
			constexpr auto LABEL_SCALE_MIN = 0.4f;

			float cameraDist = Vector3::Distance(cameraPos, labelPos);
			float alpha = cameraDist / RANGE;
			return Lerp(LABEL_SCALE_MAX, LABEL_SCALE_MIN, alpha);
		};

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
		if (_points.size() == 1)
		{
			// Draw sphere.
			g_Renderer.AddDebugSphere(_points.front(), SPHERE_RADIUS, COLOR_YELLOW_TRANSLUCENT, RendererDebugPage::AttractorStats, false);

			// Determine label parameters.
			auto labelPos = _points.front();
			auto labelPos2D = g_Renderer.Get2DPosition(labelPos);
			float labelScale = getLabelScale(Camera.pos.ToVector3(), labelPos);

			// Draw label.
			if (labelPos2D.has_value())
				g_Renderer.AddDebugString(labelString, *labelPos2D, COLOR_YELLOW_OPAQUE, labelScale, (int)PrintStringFlags::Outline, RendererDebugPage::AttractorStats);
		}
		else
		{
			// Get segment points.
			const auto& origin = _points[segmentID];
			const auto& target = _points[segmentID + 1];

			// Draw main line.
			g_Renderer.AddDebugLine(origin, target, COLOR_YELLOW_OPAQUE, RendererDebugPage::AttractorStats);

			auto orient = EulerAngles(0, Geometry::GetOrientToPoint(origin, target).y + ANGLE(90.0f), 0);
			auto dir = orient.ToDirection();

			// Draw segment heading indicator lines.
			g_Renderer.AddDebugLine(origin, Geometry::TranslatePoint(origin, dir, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);
			g_Renderer.AddDebugLine(target, Geometry::TranslatePoint(target, dir, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);

			// Determine label parameters.
			auto labelPos = ((origin + target) / 2) + LABEL_OFFSET;
			auto labelPos2D = g_Renderer.Get2DPosition(labelPos);
			float labelScale = getLabelScale(Camera.pos.ToVector3(), labelPos);

			// Draw label.
			if (labelPos2D.has_value())
				g_Renderer.AddDebugString(labelString, *labelPos2D, COLOR_YELLOW_OPAQUE, labelScale, 0, RendererDebugPage::AttractorStats);

			// Draw start indicator line.
			if (segmentID == 0)
				g_Renderer.AddDebugLine(origin, Geometry::TranslatePoint(origin, -Vector3::UnitY, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);
			
			// Draw end indicator line
			if (segmentID == (GetSegmentCount() - 1))
				g_Renderer.AddDebugLine(target, Geometry::TranslatePoint(target, -Vector3::UnitY, INDICATOR_LINE_LENGTH), COLOR_GREEN, RendererDebugPage::AttractorStats);

			// Draw AABB.
			//g_Renderer.AddDebugBox(_box, Vector4::One, RendererDebugPage::AttractorStats);
		}
	}

	float Attractor::NormalizeChainDistance(float chainDist) const
	{
		// Distance along attractor is within bounds; return it.
		if (chainDist >= 0.0f && chainDist <= _length)
			return chainDist;

		// Wrap or clamp distance along attractor.
		return (IsLooped() ? fmod(chainDist + _length, _length) : std::clamp(chainDist, 0.0f, _length));
	}

	void Attractor::CacheSegmentLengths()
	{
		// Clear segment lengths.
		_segmentLengths.clear();

		// Single point exists; cache single segment length of 0.
		if (_points.size() == 1)
		{
			_segmentLengths.push_back(0.0f);
			return;
		}

		// Collect segment lengths.
		for (int i = 0; i < GetSegmentCount(); i++)
		{
			// Get segment points.
			const auto& origin = _points[i];
			const auto& target = _points[i + 1];

			// Add segment length.
			_segmentLengths.push_back(Vector3::Distance(origin, target));
		}
	}
	
	void Attractor::CacheLength()
	{
		// Accumulate length.
		float length = 0.0f;
		for (float segmentLength : _segmentLengths)
			length += segmentLength;
		
		_length = length;
	}

	void Attractor::CacheBox()
	{
		_box = Geometry::GetBoundingBox(_points);
	}

	std::vector<Vector3> GetBridgeAttractorPoints(const ItemInfo& bridgeItem)
	{
		constexpr auto TILT_STEP = CLICK(1);

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

		// Get corners.
		auto corners = std::array<Vector3, 8>{};
		auto box = GameBoundingBox(&bridgeItem).ToBoundingOrientedBox(bridgeItem.Pose);
		box.GetCorners(corners.data());

		// Collect relevant points.
		auto offset = Vector3(0.0f, tiltOffset, 0.0f);
		return std::vector<Vector3>
		{
			corners[0],
			corners[4],
			corners[5] + offset,
			corners[1] + offset,
			corners[0]
		};
	}

	Attractor GenerateBridgeAttractor(const ItemInfo& bridgeItem)
	{
		auto points = GetBridgeAttractorPoints(bridgeItem);
		return Attractor(AttractorType::Edge, points, bridgeItem.RoomNumber);
	}
}
