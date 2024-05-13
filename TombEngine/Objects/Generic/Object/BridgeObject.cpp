#include "framework.h"
#include "Objects/Generic/Object/BridgeObject.h"

#include "Game/collision/Attractor.h"
#include "Game/items.h"
#include "Objects/game_object_ids.h"
#include "Objects/Generic/Object/Pushable/PushableInfo.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"

using namespace TEN::Collision::Attractor;

namespace TEN::Entities::Generic
{
	AttractorObject& BridgeObject::GetAttractor()
	{
		return _attractor;
	}

	void BridgeObject::Initialize(const ItemInfo& item)
	{
		auto points = GetAttractorPoints(item);
		_attractor = AttractorObject(AttractorType::Edge, points, item.RoomNumber);
	}

	void BridgeObject::Update(const ItemInfo& item)
	{
		auto points = GetAttractorPoints(item);
		_attractor.Update(points, item.RoomNumber);
	}

	std::vector<Vector3> BridgeObject::GetAttractorPoints(const ItemInfo& item) const
	{
		constexpr auto TILT_STEP = CLICK(1);

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

		// Get corners.
		auto corners = std::array<Vector3, BoundingOrientedBox::CORNER_COUNT>{};
		auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		box.GetCorners(corners.data());

		// NOTE: Traces only top plane.
		// Collect and return relevant points.
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

	const BridgeObject& GetBridgeObject(const ItemInfo& item)
	{
		// HACK: Pushable bridges.
		if (item.Data.is<PushableInfo>())
		{
			const auto& pushable = GetPushableInfo(item);
			return pushable.Bridge;
		}

		return (BridgeObject&)item.Data;
	}

	BridgeObject& GetBridgeObject(ItemInfo& item)
	{
		// HACK: Pushable bridges.
		if (item.Data.is<PushableInfo>())
		{
			auto& pushable = GetPushableInfo(item);
			return pushable.Bridge;
		}

		return (BridgeObject&)item.Data;
	}
}
