#include "framework.h"
#include "Game/collision/Attractor.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"

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
		CacheLength();
		CacheBox();
	}

	// TODO
	Attractor::~Attractor()
	{
		// Dereference current attractor held by players.
		/*for (auto& [itemNumber, itemPtr] : _attachedPlayers)
		{
			auto& player = GetLaraInfo(*itemPtr);

			if (player.Context.HandsAttractor.AttracPtr == this)
				player.Context.HandsAttractor.AttracPtr = nullptr;
		}*/
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

	float Attractor::GetLength() const
	{
		return _length;
	}

	const BoundingBox& Attractor::GetBox() const
	{
		return _box;
	}

	bool Attractor::IsEdge() const
	{
		return (_type == AttractorType::Edge);
	}

	bool Attractor::IsLooped() const
	{
		// Too few points; loop not possible.
		if (_points.size() <= 2)
			return false;

		// Test if start and end points occupy same approximate position.
		float distSqr = Vector3::DistanceSquared(_points.front(), _points.back());
		return (distSqr <= EPSILON);
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
		float chainDistTravelled = 0.0f;
		for (int i = 0; i < (_points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = _points[i];
			const auto& target = _points[i + 1];

			float segmentLength = Vector3::Distance(origin, target);
			float remainingChainDist = chainDist - chainDistTravelled;

			// Found segment of distance along attractor; return intersection.
			if (remainingChainDist <= segmentLength)
			{
				float alpha = remainingChainDist / segmentLength;
				return Vector3::Lerp(origin, target, alpha);
			}

			// Accumulate distance travelled along attractor.
			chainDistTravelled += segmentLength;
		}

		// FAILSAFE: Return end point.
		return _points.back();
	}

	unsigned int Attractor::GetSegmentIDAtChainDistance(float chainDist) const
	{
		// Single segment exists; return segment ID 0.
		if (_points.size() <= 2)
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
			return ((int)_points.size() - 1);
		}

		// Find segment at distance along attractor.
		float chainDistTravelled = 0.0f;
		for (int i = 0; i < (_points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = _points[i];
			const auto& target = _points[i + 1];

			// Accumulate distance travelled along attractor.
			chainDistTravelled += Vector3::Distance(origin, target);

			// Segment found; return segment ID.
			if (chainDistTravelled >= chainDist)
				return i;
		}

		// FAILSAFE: Return end segment ID.
		return ((int)_points.size() - 1);
	}

	void Attractor::Update(const std::vector<Vector3>& points, int roomNumber)
	{
		if (points.empty())
			TENLog("Attempted to update invalid attractor.", LogLevel::Warning);

		_points = points;
		_roomNumber = roomNumber;
		CacheLength();
		CacheBox();
	}

	// TODO
	void Attractor::AttachPlayer(ItemInfo& playerItem)
	{
		if (!playerItem.IsLara())
			return;

		//_attachedPlayers.insert({ playerItem.Index, &playerItem });
	}

	// TODO
	void Attractor::DetachPlayer(ItemInfo& playerItem)
	{
		if (!playerItem.IsLara())
			return;

		//_attachedPlayers.erase(playerItem.Index);
	}

	void Attractor::DrawDebug() const
	{
		constexpr auto LABEL_OFFSET			 = Vector3(0.0f, -CLICK(0.25f), 0.0f);
		constexpr auto INDICATOR_LINE_LENGTH = BLOCK(1 / 20.0f);
		constexpr auto SPHERE_RADIUS		 = BLOCK(1 / 52.0f);
		constexpr auto COLOR_YELLOW			 = Color(1.0f, 1.0f, 0.4f);
		constexpr auto COLOR_GREEN			 = Color(0.4f, 1.0f, 0.4f);

		auto getLabelScale = [](const Vector3& cameraPos, const Vector3& labelPos)
		{
			constexpr auto RANGE		   = BLOCK(10);
			constexpr auto LABEL_SCALE_MAX = 0.8f;
			constexpr auto LABEL_SCALE_MIN = 0.2f;

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

		default:
			labelString = "Undefined attractor";
			break;
		}

		// Draw debug elements.
		if (_points.size() == 1)
		{
			// Draw sphere.
			g_Renderer.AddDebugSphere(_points.front(), SPHERE_RADIUS, Color(COLOR_YELLOW.R(), COLOR_YELLOW.G(), COLOR_YELLOW.B(), 0.2f), RendererDebugPage::CollisionStats, false);

			// Determine label parameters.
			auto labelPos = _points.front();
			auto labelPos2D = g_Renderer.Get2DPosition(labelPos);
			float labelScale = getLabelScale(Camera.pos.ToVector3(), labelPos);

			// Draw label.
			if (labelPos2D.has_value())
				g_Renderer.AddDebugString(labelString, *labelPos2D, Color(PRINTSTRING_COLOR_WHITE), labelScale, PRINTSTRING_OUTLINE, RendererDebugPage::CollisionStats);
		}
		else
		{
			for (int i = 0; i < (_points.size() - 1); i++)
			{
				// Get segment points.
				const auto& origin = _points[i];
				const auto& target = _points[i + 1];

				// Draw main line.
				g_Renderer.AddDebugLine(origin, target, COLOR_YELLOW);

				auto orient = EulerAngles(0, Geometry::GetOrientToPoint(origin, target).y + ANGLE(90.0f), 0);
				auto dir = orient.ToDirection();

				// Draw segment heading indicator lines.
				g_Renderer.AddDebugLine(origin, Geometry::TranslatePoint(origin, dir, INDICATOR_LINE_LENGTH), COLOR_GREEN);
				g_Renderer.AddDebugLine(target, Geometry::TranslatePoint(target, dir, INDICATOR_LINE_LENGTH), COLOR_GREEN);

				// Determine label parameters.
				auto labelPos = ((origin + target) / 2) + LABEL_OFFSET;
				auto labelPos2D = g_Renderer.Get2DPosition(labelPos);
				float labelScale = getLabelScale(Camera.pos.ToVector3(), labelPos);

				// Draw label.
				if (labelPos2D.has_value())
					g_Renderer.AddDebugString(labelString, *labelPos2D, Color(PRINTSTRING_COLOR_WHITE), labelScale, 0, RendererDebugPage::CollisionStats);
			}

			// Draw start and end indicator lines.
			g_Renderer.AddDebugLine(_points.front(), Geometry::TranslatePoint(_points.front(), -Vector3::UnitY, INDICATOR_LINE_LENGTH), COLOR_GREEN);
			g_Renderer.AddDebugLine(_points.back(), Geometry::TranslatePoint(_points.back(), -Vector3::UnitY, INDICATOR_LINE_LENGTH), COLOR_GREEN);

			// Draw AABB.
			//g_Renderer.AddDebugBox(_box, Vector4::One, RendererDebugPage::CollisionStats);
		}
	}

	float Attractor::NormalizeChainDistance(float chainDist) const
	{
		// Distance along attractor is within bounds; return it.
		if (chainDist >= 0.0f && chainDist <= _length)
			return chainDist;

		// Looped; wrap distance along attractor.
		if (IsLooped())
		{
			int sign = -std::copysign(1, chainDist);
			return (chainDist + (_length * sign));
		}
		
		// Not looped; clamp distance along attractor.
		return std::clamp(chainDist, 0.0f, _length);
	}

	void Attractor::CacheLength()
	{
		// Single point exists; set 0 length.
		if (_points.size() == 1)
			_length = 0.0f;

		float length = 0.0f;
		for (int i = 0; i < (_points.size() - 1); i++)
		{
			// Get segment points.
			const auto& origin = _points[i];
			const auto& target = _points[i + 1];

			// Accumulate length.
			length += Vector3::Distance(origin, target);
		}

		_length = length;
	}

	void Attractor::CacheBox()
	{
		_box = Geometry::GetBoundingBox(_points);
	}

	Attractor GenerateBridgeAttractor(const ItemInfo& bridgeItem)
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
		auto points = std::vector<Vector3>
		{
			corners[0],
			corners[4],
			corners[5] + offset,
			corners[1] + offset,
			corners[0]
		};

		// Return bridge attractor.
		return Attractor(AttractorType::Edge, points, bridgeItem.RoomNumber);
	}
}