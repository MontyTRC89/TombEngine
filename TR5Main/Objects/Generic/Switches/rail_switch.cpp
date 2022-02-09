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
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR RailSwitchPos2 = { 0, 0, 550 }; 

	OBJECT_COLLISION_BOUNDS RailSwitchBounds2 =
	{
		-256, 256,
		0, 0,
		512, 768,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	void RailSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		int flag = 0;
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((!(TrInput & IN_ACTION)
			|| l->ActiveState != LS_IDLE
			|| l->AnimNumber != LA_STAND_IDLE
			|| Lara.gunStatus)
			&& (!Lara.Control.IsMoving
				|| Lara.interactedItem != itemNum))
		{
			ObjectCollision(itemNum, l, coll);
		}
		else if (item->ActiveState)
		{
			if (item->ActiveState == SWITCH_ON)
			{
				l->Position.yRot ^= (short)ANGLE(180);

				if (TestLaraPosition(&RailSwitchBounds2, item, l))
				{
					if (MoveLaraPosition(&RailSwitchPos2, item, l))
					{
						item->TargetState = SWITCH_OFF;
						flag = 1;
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
				}
				else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}

				l->Position.yRot ^= (short)ANGLE(180);

				if (flag)
				{
					l->AnimNumber = LA_LEVER_PUSH;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					l->TargetState = LS_LEVERSWITCH_PUSH;
					l->ActiveState = LS_LEVERSWITCH_PUSH;
					Lara.Control.IsMoving = false;
					ResetLaraFlex(l);
					Lara.gunStatus = LG_HANDS_BUSY;

					item->Status = ITEM_ACTIVE;
					AddActiveItem(itemNum);
					AnimateItem(item);

					return;
				}
			}

			ObjectCollision(itemNum, l, coll);
		}
		else
		{
			if (TestLaraPosition(&RailSwitchBounds, item, l))
			{
				if (MoveLaraPosition(&RailSwitchPos, item, l))
				{
					item->TargetState = SWITCH_ON;
					l->AnimNumber = LA_LEVER_PUSH;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					l->TargetState = LS_LEVERSWITCH_PUSH;
					l->ActiveState = LS_LEVERSWITCH_PUSH;
					Lara.Control.IsMoving = false;
					ResetLaraFlex(l);
					Lara.gunStatus = LG_HANDS_BUSY;

					item->Status = ITEM_ACTIVE;
					AddActiveItem(itemNum);
					AnimateItem(item);
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
			}
			else if (Lara.Control.IsMoving)
			{
				if (Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
			}

			ObjectCollision(itemNum, l, coll);
		}
	}
}