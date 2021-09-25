#include "framework.h"
#include "rail_switch.h"
#include "input.h"
#include "lara.h"
#include "generic_switch.h"
#include "level.h"
#include "collide.h"
#include "animation.h"
#include "items.h"

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
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE
			|| Lara.gunStatus)
			&& (!Lara.isMoving
				|| Lara.interactedItem != itemNum))
		{
			ObjectCollision(itemNum, l, coll);
		}
		else if (item->currentAnimState)
		{
			if (item->currentAnimState == SWITCH_ON)
			{
				l->pos.yRot ^= (short)ANGLE(180);

				if (TestLaraPosition(&RailSwitchBounds2, item, l))
				{
					if (MoveLaraPosition(&RailSwitchPos2, item, l))
					{
						item->goalAnimState = SWITCH_OFF;
						flag = 1;
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

				l->pos.yRot ^= (short)ANGLE(180);

				if (flag)
				{
					l->animNumber = LA_LEVER_PUSH;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					l->goalAnimState = LS_LEVERSWITCH_PUSH;
					l->currentAnimState = LS_LEVERSWITCH_PUSH;
					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;

					item->status = ITEM_ACTIVE;
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
					item->goalAnimState = SWITCH_ON;
					l->animNumber = LA_LEVER_PUSH;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					l->goalAnimState = LS_LEVERSWITCH_PUSH;
					l->currentAnimState = LS_LEVERSWITCH_PUSH;
					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;

					item->status = ITEM_ACTIVE;
					AddActiveItem(itemNum);
					AnimateItem(item);
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
			}
			else if (Lara.isMoving)
			{
				if (Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
			}

			ObjectCollision(itemNum, l, coll);
		}
	}
}