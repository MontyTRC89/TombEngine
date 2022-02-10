#include "framework.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS SwitchBounds = 
	{
		0, 0,
		0, 0,
		0, 0,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR SwitchPos = { 0, 0, 0 };

	void SwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->Flags |= 0x3E00;

		if (!TriggerActive(item) && !(item->Flags & ONESHOT))
		{
			if (item->ObjectNumber == ID_JUMP_SWITCH)
			{
				item->TargetState = SWITCH_OFF;
				item->Timer = 0;
				AnimateItem(item);
			}
			else
			{
				item->TargetState = SWITCH_ON;
				item->Timer = 0;
			}
		}

		AnimateItem(item);
	}

	void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		if (TrInput & IN_ACTION
			&& l->ActiveState == LS_IDLE
			&& l->AnimNumber == LA_STAND_IDLE
			&& Lara.Control.HandStatus == HandStatus::Free
			&& item->Status == ITEM_NOT_ACTIVE
			&& !(item->Flags & 0x100)
			&& item->TriggerFlags >= 0
			|| Lara.Control.IsMoving && Lara.interactedItem == itemNum)
		{
			BOUNDING_BOX* bounds = GetBoundsAccurate(item);

			if (item->TriggerFlags == 3 && item->ActiveState == SWITCH_ON
				|| item->TriggerFlags >= 5 && item->TriggerFlags <= 7 
					&& item->ActiveState == SWITCH_OFF)
				return;

			SwitchBounds.boundingBox.X1 = bounds->X1 - 256;
			SwitchBounds.boundingBox.X2 = bounds->X2 + 256;

			if (item->TriggerFlags)
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 512;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 512;

				if (item->TriggerFlags == 3)
				{
					SwitchPos.z = bounds->Z1 - 256;
				}
				else
				{
					SwitchPos.z = bounds->Z1 - 128;
				}
			}
			else
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 200;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 200;
				SwitchPos.z = bounds->Z1 - 64;
			}

			if (TestLaraPosition(&SwitchBounds, item, l))
			{
				if (MoveLaraPosition(&SwitchPos, item, l))
				{
					if (item->ActiveState == SWITCH_ON) /* Switch down */
					{
						if (item->TriggerFlags)
						{
							l->AnimNumber = LA_HOLESWITCH_ACTIVATE;
							l->ActiveState = LS_HOLE;
						}
						else
						{
							l->ActiveState = LS_SWITCH_UP;
							l->AnimNumber = LA_WALLSWITCH_DOWN;
						}
						
						item->TargetState = SWITCH_OFF;
					}
					else /* Switch up */
					{
						if (item->TriggerFlags)
						{
							if (item->TriggerFlags == 3)
							{
								l->AnimNumber = LA_BUTTON_LARGE_PUSH;
							}
							else
							{
								l->AnimNumber = LA_HOLESWITCH_ACTIVATE;
								l->ActiveState = LS_HOLE;
							}
						}
						else
						{
							l->ActiveState = LS_SWITCH_DOWN;
							l->AnimNumber = LA_WALLSWITCH_UP;
						}

						item->TargetState = SWITCH_ON;
					}

					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					Lara.Control.IsMoving = false;
					Lara.Control.HandStatus = HandStatus::Busy;
					ResetLaraFlex(l);

					AddActiveItem(itemNum);
					item->Status = ITEM_ACTIVE;
					AnimateItem(item);
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
			}
			else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
			{
				Lara.Control.IsMoving = false;
				Lara.Control.HandStatus = HandStatus::Free;
			}

			return;
		}

		if (l->ActiveState != LS_SWITCH_DOWN && l->ActiveState != LS_SWITCH_UP)
			ObjectCollision(itemNum, l, coll);
	}
}