#include "framework.h"
#include "Objects/Generic/Switches/cog_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/itemdata/door_data.h"
#include "Game/control/box.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/items.h"

using namespace TEN::Entities::Doors;

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS CogSwitchBounds =
	{
		-512, 512,
		0, 0,
		-1536, -512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};
	PHD_VECTOR CogSwitchPos(0, 0, -856);

	void CogSwitchCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNum];
		auto* triggerIndex = GetTriggerIndex(switchItem);

		int targetItemNum;
		ITEM_INFO* target = nullptr;
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

		if (door == nullptr)
		{
			ObjectCollision(itemNum, laraItem, coll);
			return;
		}

		// Door is found, attach to it.

		if (switchItem->Status == ITEM_NOT_ACTIVE)
		{
			if (!(switchItem->Flags & ONESHOT) &&
				(TrInput & IN_ACTION &&
					laraItem->ActiveState == LS_IDLE &&
					laraItem->AnimNumber == LA_STAND_IDLE &&
					laraInfo->Control.HandStatus == HandStatus::Free &&
					!switchItem->Airborne ||
					laraInfo->Control.IsMoving &&
					laraInfo->InteractedItem == itemNum))
			{
				if (TestLaraPosition(&CogSwitchBounds, switchItem, laraItem))
				{
					if (MoveLaraPosition(&CogSwitchPos, switchItem, laraItem))
					{
						ResetLaraFlex(laraItem);
						laraItem->AnimNumber = LA_COGWHEEL_GRAB;
						laraItem->TargetState = LS_COGWHEEL;
						laraItem->ActiveState = LS_COGWHEEL;
						laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
						laraInfo->Control.IsMoving = false;
						laraInfo->Control.HandStatus = HandStatus::Busy;
						laraInfo->InteractedItem = targetItemNum;

						AddActiveItem(itemNum);
						switchItem->TargetState = SWITCH_ON;
						switchItem->Status = ITEM_ACTIVE;

						if (door != NULL)
						{
							if (!door->opened)
							{
								AddActiveItem((target - g_Level.Items.data()));
								target->Status = ITEM_ACTIVE;
							}
						}
					}
					else
						laraInfo->InteractedItem = itemNum;

					return;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNum)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}
			}

			ObjectCollision(itemNum, laraItem, coll);
		}
	}

	void CogSwitchControl(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		AnimateItem(switchItem);

		if (switchItem->ActiveState == SWITCH_ON)
		{
			if (switchItem->TargetState == SWITCH_ON && !(TrInput & IN_ACTION))
			{
				LaraItem->TargetState = LS_IDLE;
				switchItem->TargetState = SWITCH_OFF;
			}

			if (LaraItem->AnimNumber == LA_COGWHEEL_PULL)
			{
				if (LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameBase + 10)
				{
					auto* doorItem = &g_Level.Items[Lara.InteractedItem];
					doorItem->ItemFlags[0] = COG_DOOR_TURN;
				}
			}
		}
		else
		{
			if (switchItem->FrameNumber == g_Level.Anims[switchItem->AnimNumber].frameEnd)
			{
				switchItem->ActiveState = SWITCH_OFF;
				switchItem->Status = ITEM_NOT_ACTIVE;

				RemoveActiveItem(itemNumber);

				LaraItem->AnimNumber = LA_STAND_SOLID;
				LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
				LaraItem->TargetState = LS_IDLE;
				LaraItem->ActiveState = LS_IDLE;
				Lara.Control.HandStatus = HandStatus::Free;
			}
		}
	}
}
