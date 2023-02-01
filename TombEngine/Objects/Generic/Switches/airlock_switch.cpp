#include "framework.h"
#include "Game/control/control.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	ObjectCollisionBounds LockSwitchBounds =
	{
		GameBoundingBox::Zero,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};

	auto AirLockSwitchPos = Vector3i::Zero;

	void AirLockSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			switchItem->Animation.ActiveState == 0 &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->Status == ITEM_NOT_ACTIVE ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			auto bounds = GameBoundingBox(switchItem);

			LockSwitchBounds.BoundingBox.X1 = bounds.X1 - 256;
			LockSwitchBounds.BoundingBox.X2 = bounds.X2 + 256;
			LockSwitchBounds.BoundingBox.Z1 = bounds.Z1 - 512;
			LockSwitchBounds.BoundingBox.Z2 = bounds.Z2 + 512;
			AirLockSwitchPos.z = bounds.Z1 - 112;

			if (TestLaraPosition(LockSwitchBounds, switchItem, laraItem))
			{
				if (MoveLaraPosition(AirLockSwitchPos, switchItem, laraItem))
				{
					if (switchItem->Animation.ActiveState == 0)
					{
						SetAnimation(laraItem, LaraAnim::LA_VALVE_TURN);
						switchItem->Animation.TargetState = 1;
					}

					ResetLaraFlex(laraItem);
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;

					AddActiveItem(itemNumber);
					switchItem->Status = ITEM_ACTIVE;
					AnimateItem(switchItem);
				}
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}

			return;
		}

		if (laraItem->Animation.ActiveState != LS_SWITCH_DOWN)
			ObjectCollision(itemNumber, laraItem, coll);
	}
}
