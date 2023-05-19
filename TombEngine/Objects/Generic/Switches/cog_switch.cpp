#include "framework.h"
#include "Objects/Generic/Switches/cog_switch.h"
#include "Game/control/control.h"
#include "Specific/Input/Input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/itemdata/door_data.h"
#include "Game/control/box.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/items.h"

using namespace TEN::Input;
using namespace TEN::Entities::Doors;

namespace TEN::Entities::Switches
{
	const ObjectCollisionBounds CogSwitchBounds =
	{
		GameBoundingBox(
			-SECTOR(0.5f), SECTOR(0.5f),
			0, 0,
			-SECTOR(1.5f), -SECTOR(0.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};
	const auto CogSwitchPos = Vector3i(0, 0, -856);

	void CogSwitchCollision(short itemNum, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNum];
		auto* triggerIndex = GetTriggerIndex(switchItem);

		int targetItemNum;
		ItemInfo* target = nullptr;
		DOOR_DATA* door   = nullptr;

		// Try to find first item in a trigger list, and if it is a door,
		// attach it to cog. If no object found or object is not door,
		// bypass further processing and do ordinary object collision.

		if (triggerIndex)
		{
			short* trigger = triggerIndex;
			targetItemNum = trigger[3] & VALUE_BITS;

			if (targetItemNum < g_Level.Items.size())
			{
				target = &g_Level.Items[targetItemNum];
				if (target->Data.is<DOOR_DATA>())
					door = (DOOR_DATA*)target->Data;
			}
		}

		// Door was not found, do ordinary collision and exit.

		if ((door == nullptr) && (!switchItem->TriggerFlags))
		{
			ObjectCollision(itemNum, laraItem, coll);
			return;
		}

		// Door is found, attach to it.

		if (switchItem->Status == ITEM_NOT_ACTIVE)
		{
			if (!(switchItem->Flags & IFLAG_INVISIBLE) &&
				(TrInput & IN_ACTION &&
					laraItem->Animation.ActiveState == LS_IDLE &&
					laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
					lara->Control.HandStatus == HandStatus::Free &&
					!switchItem->Animation.IsAirborne ||
					lara->Control.IsMoving &&
					lara->Context.InteractedItem == itemNum))
			{
				if (TestLaraPosition(CogSwitchBounds, switchItem, laraItem))
				{
					if (MoveLaraPosition(CogSwitchPos, switchItem, laraItem))
					{
						ResetPlayerFlex(laraItem);
						laraItem->Animation.AnimNumber = LA_COGWHEEL_GRAB;
						laraItem->Animation.TargetState = LS_COGWHEEL;
						laraItem->Animation.ActiveState = LS_COGWHEEL;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						lara->Control.IsMoving = false;
						lara->Control.HandStatus = HandStatus::Busy;
						lara->Context.InteractedItem = targetItemNum;

						AddActiveItem(itemNum);
						switchItem->Animation.TargetState = SWITCH_ON;
						switchItem->Status = ITEM_ACTIVE;

						if ((door != nullptr) && (!switchItem->TriggerFlags))
						{
							if (!door->opened)
							{
								AddActiveItem((target - g_Level.Items.data()));
								target->Status = ITEM_ACTIVE;
							}
						}
					}
					else
						lara->Context.InteractedItem = itemNum;

					return;
				}
				else if (lara->Control.IsMoving && lara->Context.InteractedItem == itemNum)
				{
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Free;
				}
			}

			ObjectCollision(itemNum, laraItem, coll);
		}
	}

	void CogSwitchControl(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		AnimateItem(switchItem);

		if (switchItem->Animation.ActiveState == SWITCH_ON)
		{
			if (switchItem->Animation.TargetState == SWITCH_ON && !(TrInput & IN_ACTION))
			{
				LaraItem->Animation.TargetState = LS_COGWHEEL_UNGRAB;
				switchItem->Animation.TargetState = SWITCH_OFF;
			}

			if (LaraItem->Animation.AnimNumber == LA_COGWHEEL_PULL)
			{
				if (LaraItem->Animation.FrameNumber == g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase + 10)
				{
					if (!switchItem->TriggerFlags)
					{
						auto* doorItem = &g_Level.Items[Lara.Context.InteractedItem];
						doorItem->ItemFlags[0] = COG_DOOR_TURN;
					}
				}
			}
		}
		else
		{
			if ((switchItem->Animation.FrameNumber == g_Level.Anims[switchItem->Animation.AnimNumber].frameEnd)
				&& (LaraItem->Animation.AnimNumber == LA_COGWHEEL_RELEASE))
			{
				switchItem->Animation.ActiveState = SWITCH_OFF;
				switchItem->Status = ITEM_NOT_ACTIVE;

				RemoveActiveItem(itemNumber);

				Lara.Control.HandStatus = HandStatus::Free;
			}
			else
			{
				//If Lara is repeating the PULL animation (because player dropped after the frame check).
				//do the wheel animation again.
				switchItem->Animation.TargetState = SWITCH_ON;
			}
		}
	}
}
