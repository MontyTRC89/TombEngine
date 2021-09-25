#include "framework.h"
#include "tr5_crowdove_switch.h"
#include "control/control.h"
#include "input.h"
#include "level.h"
#include "lara.h"
#include "generic_switch.h"
#include "Sound/sound.h"
#include "animation.h"
#include "items.h"

using namespace TEN::Entities::Switches;

namespace TEN::Entities::TR5
{
	OBJECT_COLLISION_BOUNDS CrowDoveBounds =
	{
		-256, 256,
		0, 0,
		-512, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR CrowDovePos = { 0, 0, -400 }; 

	void InitialiseCrowDoveSwitch(short itemNumber)
	{
		g_Level.Items[itemNumber].meshBits = 3;
	}

	void CrowDoveSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->flags & ONESHOT
			|| !(item->meshBits & 4)
			|| (!(TrInput & IN_ACTION)
				|| Lara.gunStatus
				|| l->currentAnimState != LS_STOP
				|| l->animNumber != LA_STAND_IDLE
				|| l->gravityStatus)
			&& (!Lara.isMoving || Lara.interactedItem != itemNum))
		{
			if (l->currentAnimState != LS_DOVESWITCH)
				ObjectCollision(itemNum, l, coll);
		}
		else
		{
			int oldYrot = item->pos.yRot;
			item->pos.yRot = l->pos.yRot;
			if (TestLaraPosition(&CrowDoveBounds, item, l))
			{
				if (MoveLaraPosition(&CrowDovePos, item, l))
				{
					l->animNumber = LA_DOVESWITCH_TURN;
					l->currentAnimState = LS_DOVESWITCH;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;

					AddActiveItem(itemNum);

					// NOTE: In original TR5 the switch was used together with heavy switches.
					// This little fix make it usable normaly and less hardcoded.
					item->itemFlags[0] = 0;

					item->status = ITEM_ACTIVE;
					item->pos.yRot = oldYrot;
					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.interactedItem = itemNum;
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
				item->pos.yRot = oldYrot;
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				item->pos.yRot = oldYrot;
			}
		}
	}

	void CrowDoveSwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->meshBits & 2) 
		{
			ExplodeItemNode(item, 1, 0, 256); 
			SoundEffect(SFX_TR5_RAVENSWITCH_EXP, &item->pos, 0);
			item->meshBits = 5;	
			RemoveActiveItem(itemNumber);

			// NOTE: In original TR5 the switch was used together with heavy switches.
			// This little fix make it usable normaly and less hardcoded.
			item->itemFlags[0] = 1; 
		}
		else if (item->itemFlags[0] == 0)
		{
			if (item->currentAnimState == SWITCH_OFF)
				item->goalAnimState = SWITCH_ON;

			AnimateItem(item);

			if (item->currentAnimState == SWITCH_OFF)
				item->pos.yRot += ANGLE(90);
		}
	}
}