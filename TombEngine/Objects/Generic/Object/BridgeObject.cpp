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
		_attractor = AttractorObject(AttractorType::Edge, item.Pose.Position.ToVector3(), item.Pose.Orientation.ToQuaternion(), item.RoomNumber, points);
	}

	void BridgeObject::Update(const ItemInfo& item)
	{
		_attractor.SetPosition(item.Pose.Position.ToVector3());
		_attractor.SetOrientation(item.Pose.Orientation.ToQuaternion());
	}

	std::vector<Vector3> BridgeObject::GetAttractorPoints(const ItemInfo& item) const
	{
		constexpr auto TILT_STEP = CLICK(1);

		// Determine tilt offset.
		auto offset = Vector3::Zero;
		switch (item.ObjectNumber)
		{
		default:
		case ID_BRIDGE_FLAT:
			break;

		case ID_BRIDGE_TILT1:
			offset = Vector3(0.0f, TILT_STEP, 0.0f);
			break;

		case ID_BRIDGE_TILT2:
			offset = Vector3(0.0f, TILT_STEP * 2, 0.0f);
			break;

		case ID_BRIDGE_TILT3:
			offset = Vector3(0.0f, TILT_STEP * 3, 0.0f);
			break;

		case ID_BRIDGE_TILT4:
			offset = Vector3(0.0f, TILT_STEP * 4, 0.0f);
			break;
		}

		// Get AABB corners.
		auto bounds = GameBoundingBox(&item);
		auto aabb = BoundingBox(bounds.GetCenter(), bounds.GetExtents());
		auto corners = std::array<Vector3, BoundingBox::CORNER_COUNT>{};
		aabb.GetCorners(corners.data());
		
		// Collect and return attractor points. NOTE: Traces only top plane of bridge.
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
