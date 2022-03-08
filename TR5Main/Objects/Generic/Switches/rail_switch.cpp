#include "framework.h"
#include "Objects/Generic/Switches/rail_switch.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	PHD_VECTOR RailSwitchPos = { 0, 0, -550 };

	OBJECT_COLLISION_BOUNDS RailSwitchBounds =
	{
		-256, 256,
		0, 0,
		-768, -512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR RailSwitchPos2 = { 0, 0, 550 }; 

	OBJECT_COLLISION_BOUNDS RailSwitchBounds2 =
	{
		-256, 256,
		0, 0,
		512, 768,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	void RailSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		int flag = 0;

		if ((!(TrInput & IN_ACTION) ||
			laraItem->ActiveState != LS_IDLE ||
			laraItem->AnimNumber != LA_STAND_IDLE ||
			laraInfo->Control.HandStatus != HandStatus::Free) &&
			(!laraInfo->Control.IsMoving ||
				laraInfo->InteractedItem != itemNumber))
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
		else if (switchItem->ActiveState)
		{
			if (switchItem->ActiveState == SWITCH_ON)
			{
				laraItem->Position.yRot ^= (short)ANGLE(180.0f);

				if (TestLaraPosition(&RailSwitchBounds2, switchItem, laraItem))
				{
					if (MoveLaraPosition(&RailSwitchPos2, switchItem, laraItem))
					{
						switchItem->TargetState = SWITCH_OFF;
						flag = 1;
					}
					else
						laraInfo->InteractedItem = itemNumber;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				laraItem->Position.yRot ^= (short)ANGLE(180.0f);

				if (flag)
				{
					ResetLaraFlex(laraItem);
					laraItem->AnimNumber = LA_LEVER_PUSH;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					laraItem->TargetState = LS_LEVERSWITCH_PUSH;
					laraItem->ActiveState = LS_LEVERSWITCH_PUSH;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					switchItem->Status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);
					AnimateItem(switchItem);
					return;
				}
			}

			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			if (TestLaraPosition(&RailSwitchBounds, switchItem, laraItem))
			{
				if (MoveLaraPosition(&RailSwitchPos, switchItem, laraItem))
				{
					ResetLaraFlex(laraItem);
					laraItem->AnimNumber = LA_LEVER_PUSH;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					laraItem->TargetState = LS_LEVERSWITCH_PUSH;
					laraItem->ActiveState = LS_LEVERSWITCH_PUSH;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					switchItem->TargetState = SWITCH_ON;
					switchItem->Status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);
					AnimateItem(switchItem);
				}
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else if (laraInfo->Control.IsMoving)
			{
				if (laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}
			}

			ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}
