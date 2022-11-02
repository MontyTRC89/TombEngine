#include "framework.h"
#include "Objects/Generic/Switches/rail_switch.h"
#include "Specific/Input/Input.h"
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
	const auto RailSwitchPos = Vector3i(0, 0, -550);
	const InteractionBasis RailSwitchBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			0, 0,
			-SECTOR(0.75f), -SECTOR(0.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};

	const auto RailSwitchPos2 = Vector3i(0, 0, 550);
	const InteractionBasis RailSwitchBounds2 =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			0, 0,
			SECTOR(0.5f), SECTOR(0.75f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
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

				if (TestPlayerEntityInteract(switchItem, laraItem, RailSwitchBounds2))
				{
					if (AlignPlayerToEntity(switchItem, laraItem, RailSwitchPos2))
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
			if (TestPlayerEntityInteract(switchItem, laraItem, RailSwitchBounds))
			{
				if (AlignPlayerToEntity(switchItem, laraItem, RailSwitchPos))
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
