#include "framework.h"
#include "control/control.h"
#include "input.h"
#include "level.h"
#include "lara.h"
#include "generic_switch.h"
#include "animation.h"
#include "collide.h"
#include "items.h"

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

		item->flags |= 0x3E00;

		if (!TriggerActive(item) && !(item->flags & ONESHOT))
		{
			if (item->objectNumber == ID_JUMP_SWITCH)
			{
				item->goalAnimState = SWITCH_OFF;
				item->timer = 0;
				AnimateItem(item);
			}
			else
			{
				item->goalAnimState = SWITCH_ON;
				item->timer = 0;
			}
		}

		AnimateItem(item);
	}

	void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		if (TrInput & IN_ACTION
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& !Lara.gunStatus
			&& item->status == ITEM_NOT_ACTIVE
			&& !(item->flags & 0x100)
			&& item->triggerFlags >= 0
			|| Lara.isMoving && Lara.interactedItem == itemNum)
		{
			BOUNDING_BOX* bounds = GetBoundsAccurate(item);

			if (item->triggerFlags == 3 && item->currentAnimState == SWITCH_ON
				|| item->triggerFlags >= 5 && item->triggerFlags <= 7 
					&& item->currentAnimState == SWITCH_OFF)
				return;

			SwitchBounds.boundingBox.X1 = bounds->X1 - 256;
			SwitchBounds.boundingBox.X2 = bounds->X2 + 256;

			if (item->triggerFlags)
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 512;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 512;

				if (item->triggerFlags == 3)
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
					if (item->currentAnimState == SWITCH_ON) /* Switch down */
					{
						if (item->triggerFlags)
						{
							l->animNumber = LA_HOLESWITCH_ACTIVATE;
							l->currentAnimState = LS_HOLE;
						}
						else
						{
							l->currentAnimState = LS_SWITCH_UP;
							l->animNumber = LA_WALLSWITCH_DOWN;
						}
						
						item->goalAnimState = SWITCH_OFF;
					}
					else /* Switch up */
					{
						if (item->triggerFlags)
						{
							if (item->triggerFlags == 3)
							{
								l->animNumber = LA_BUTTON_LARGE_PUSH;
							}
							else
							{
								l->animNumber = LA_HOLESWITCH_ACTIVATE;
								l->currentAnimState = LS_HOLE;
							}
						}
						else
						{
							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_WALLSWITCH_UP;
						}

						item->goalAnimState = SWITCH_ON;
					}

					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;

					AddActiveItem(itemNum);
					item->status = ITEM_ACTIVE;
					AnimateItem(item);
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
			}
			else if (Lara.isMoving && Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}

			return;
		}

		if (l->currentAnimState != LS_SWITCH_DOWN && l->currentAnimState != LS_SWITCH_UP)
			ObjectCollision(itemNum, l, coll);
	}
}