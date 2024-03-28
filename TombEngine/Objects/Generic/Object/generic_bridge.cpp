#include "framework.h"
#include "Objects/Generic/Object/generic_bridge.h"

#include "Game/collision/floordata.h"
#include "Objects/game_object_ids.h"
#include "Objects/Generic/Object/BridgeObject.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

namespace TEN::Entities::Generic
{
	template <int tiltGrade>
	static std::optional<int> GetBridgeFloorHeight(const ItemInfo& item, const Vector3i& pos)
	{
		auto boxHeight = GetBridgeItemIntersect(item, pos, false);
		if (boxHeight.has_value() && tiltGrade != 0)
		{
			int height = item.Pose.Position.y + (tiltGrade * ((GetOffset(item.Pose.Orientation.y, pos.x, pos.z) / 4) + (BLOCK(1 / 8.0f))));
			return height;
		}

		return boxHeight;
	}

	template <int tiltGrade>
	static std::optional<int> GetBridgeCeilingHeight(const ItemInfo& item, const Vector3i& pos)
	{
		auto boxHeight = GetBridgeItemIntersect(item, pos, true);
		if (boxHeight.has_value() && tiltGrade != 0)
		{
			int height = item.Pose.Position.y + (tiltGrade * ((GetOffset(item.Pose.Orientation.y, pos.x, pos.z) / 4) + (BLOCK(1 / 8.0f))));
			return (height + CLICK(1));
		}

		return boxHeight;
	}

	template <int tiltGrade>
	static int GetBridgeFloorBorder(const ItemInfo& item)
	{
		return GetBridgeBorder(item, false);
	}

	template <int tiltGrade>
	static int GetBridgeCeilingBorder(const ItemInfo& item)
	{
		return GetBridgeBorder(item, true);
	}

	void InitializeBridge(short itemNumber)
	{
		auto& bridgeItem = g_Level.Items[itemNumber];
		bridgeItem.Data = BridgeObject();
		auto& bridge = GetBridgeObject(bridgeItem);

		// Initialize routines.
		switch (bridgeItem.ObjectNumber)
		{
		default:
		case ID_BRIDGE_FLAT:
			bridge.GetFloorHeight = GetBridgeFloorHeight<0>;
			bridge.GetCeilingHeight = GetBridgeCeilingHeight<0>;
			bridge.GetFloorBorder = GetBridgeFloorBorder<0>;
			bridge.GetCeilingBorder = GetBridgeCeilingBorder<0>;
			break;

		case ID_BRIDGE_TILT1:
			bridge.GetFloorHeight = GetBridgeFloorHeight<1>;
			bridge.GetCeilingHeight = GetBridgeCeilingHeight<1>;
			bridge.GetFloorBorder = GetBridgeFloorBorder<1>;
			bridge.GetCeilingBorder = GetBridgeCeilingBorder<1>;
			break;

		case ID_BRIDGE_TILT2:
			bridge.GetFloorHeight = GetBridgeFloorHeight<2>;
			bridge.GetCeilingHeight = GetBridgeCeilingHeight<2>;
			bridge.GetFloorBorder = GetBridgeFloorBorder<2>;
			bridge.GetCeilingBorder = GetBridgeCeilingBorder<2>;
			break;

		case ID_BRIDGE_TILT3:
			bridge.GetFloorHeight = GetBridgeFloorHeight<3>;
			bridge.GetCeilingHeight = GetBridgeCeilingHeight<3>;
			bridge.GetFloorBorder = GetBridgeFloorBorder<3>;
			bridge.GetCeilingBorder = GetBridgeCeilingBorder<3>;
			break;

		case ID_BRIDGE_TILT4:
			bridge.GetFloorHeight = GetBridgeFloorHeight<4>;
			bridge.GetCeilingHeight = GetBridgeCeilingHeight<4>;
			bridge.GetFloorBorder = GetBridgeFloorBorder<4>;
			bridge.GetCeilingBorder = GetBridgeCeilingBorder<4>;
			break;
		}

		UpdateBridgeItem(bridgeItem);
	}

	int GetOffset(short angle, int x, int z)
	{
		// Get rotated sector point.
		auto sectorPoint = GetSectorPoint(x, z).ToVector2();
		auto rotMatrix = Matrix::CreateRotationZ(TO_RAD(angle));
		Vector2::Transform(sectorPoint, rotMatrix, sectorPoint);

		// Return offset.
		return -sectorPoint.x;
	}
}
