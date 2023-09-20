#include "framework.h"
#include "Objects/Generic/Object/Pushables/BridgeCollision.h"

#include "Game/collision/floordata.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

namespace TEN::Entities::Generic
{
	void ActivateClimbablePushableCollider(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UsesRoomCollision)
		{
			AddBridge(itemNumber);
			pushable.BridgeColliderFlag = true;
		}
	}

	void DeactivateClimbablePushableCollider(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);
		
		if (pushable.UsesRoomCollision)
		{
			RemoveBridge(itemNumber);
			pushable.BridgeColliderFlag = false;
		}
	}

	void RefreshClimbablePushableCollider(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UsesRoomCollision)
			UpdateBridgeItem(itemNumber);
	}

	std::optional<int> ClimbablePushableFloor(int itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);

		if (pushableItem.Status != ITEM_INVISIBLE && pushable.UsesRoomCollision && boxHeight.has_value() && pushableItem.Active)
		{
			int height = pushableItem.Pose.Position.y - GetPushableHeight(pushableItem);
			return std::optional{ height };
		}

		return std::nullopt;
	}

	std::optional<int> ClimbablePushableCeiling(int itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (pushableItem.Status != ITEM_INVISIBLE && pushable.UsesRoomCollision && boxHeight.has_value() && pushableItem.Active)
			return std::optional{ pushableItem.Pose.Position.y };

		return std::nullopt;
	}

	int ClimbablePushableFloorBorder(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

		auto height = pushableItem.Pose.Position.y - GetPushableHeight(pushableItem);
		return height;
	}

	int ClimbablePushableCeilingBorder(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

		return pushableItem.Pose.Position.y;
	}

	void AddBridgePushableStack(int itemNumber, bool addBridge)
	{
		auto pushableItemCopy = g_Level.Items[itemNumber];
		auto& pushableCopy = GetPushableInfo(pushableItemCopy);

		if (!pushableCopy.UsesRoomCollision)
			return; //It can't have stacked items or bridge collider.

		if (pushableCopy.BridgeColliderFlag)
			addBridge ? AddBridge(itemNumber) : RemoveBridge(itemNumber);
		
		while (pushableCopy.StackUpperItem != NO_ITEM)
		{
			pushableItemCopy = g_Level.Items[pushableCopy.StackUpperItem];
			pushableCopy = GetPushableInfo(pushableItemCopy);

			if (pushableCopy.BridgeColliderFlag)
				addBridge ? AddBridge(pushableItemCopy.Index) : RemoveBridge(pushableItemCopy.Index);
		}
	}
}
