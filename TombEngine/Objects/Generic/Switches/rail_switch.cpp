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

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	Vector3Int RailSwitchPos = { 0, 0, -550 };

	OBJECT_COLLISION_BOUNDS RailSwitchBounds =
	{
		-256, 256,
		0, 0,
		-768, -512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	Vector3Int RailSwitchPos2 = { 0, 0, 550 }; 

	OBJECT_COLLISION_BOUNDS RailSwitchBounds2 =
	{
		-256, 256,
		0, 0,
		512, 768,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	void RailSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		int flag = 0;

		if ((!(TrInput & IN_ACTION) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			lara->Control.HandStatus != HandStatus::Free) &&
			(!lara->Control.IsMoving ||
				lara->InteractedItem != itemNumber))
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
		else if (switchItem->Animation.ActiveState)
		{
			if (switchItem->Animation.ActiveState == SWITCH_ON)
			{
				laraItem->Pose.Orientation.y ^= (short)ANGLE(180.0f);

				if (TestLaraPosition(&RailSwitchBounds2, switchItem, laraItem))
				{
					if (MoveLaraPosition(&RailSwitchPos2, switchItem, laraItem))
					{
						switchItem->Animation.TargetState = SWITCH_OFF;
						flag = 1;
					}
					else
						lara->InteractedItem = itemNumber;
				}
				else if (lara->Control.IsMoving && lara->InteractedItem == itemNumber)
				{
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Free;
				}

				laraItem->Pose.Orientation.y ^= (short)ANGLE(180.0f);

				if (flag)
				{
					ResetLaraFlex(laraItem);
					laraItem->Animation.AnimNumber = LA_LEVER_PUSH;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraItem->Animation.TargetState = LS_LEVERSWITCH_PUSH;
					laraItem->Animation.ActiveState = LS_LEVERSWITCH_PUSH;
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Busy;
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
					laraItem->Animation.AnimNumber = LA_LEVER_PUSH;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraItem->Animation.TargetState = LS_LEVERSWITCH_PUSH;
					laraItem->Animation.ActiveState = LS_LEVERSWITCH_PUSH;
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Busy;
					switchItem->Animation.TargetState = SWITCH_ON;
					switchItem->Status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);
					AnimateItem(switchItem);
				}
				else
					lara->InteractedItem = itemNumber;
			}
			else if (lara->Control.IsMoving)
			{
				if (lara->InteractedItem == itemNumber)
				{
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Free;
				}
			}

			ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}
