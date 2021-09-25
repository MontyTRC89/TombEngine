#include "framework.h"
#include "fullblock_switch.h"
#include "input.h"
#include "lara.h"
#include "generic_switch.h"
#include "setup.h"
#include "collide.h"
#include "level.h"
#include "animation.h"
#include "items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS FullBlockSwitchBounds = 
	{
		-384, 384,
		0, 256,
		0, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR FullBlockSwitchPos = { 0, 0, 0 };

	byte SequenceUsed[6];
	byte SequenceResults[3][3][3];
	byte Sequences[3];
	byte CurrentSequence;

	void FullBlockSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if ((!(TrInput & IN_ACTION)
			|| item->status
			|| item->flags & 0x100
			|| CurrentSequence >= 3
			|| Lara.gunStatus
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE)
			&& (!Lara.isMoving || Lara.interactedItem !=itemNum))
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		if (TestLaraPosition(&FullBlockSwitchBounds, item, l))
		{
			if (MoveLaraPosition(&FullBlockSwitchPos, item, l))
			{
				if (item->currentAnimState == 1)
				{
					l->currentAnimState = LS_SWITCH_DOWN;
					l->animNumber = LA_BUTTON_GIANT_PUSH;
					item->goalAnimState = 0;
				}
				l->goalAnimState = LS_STOP;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				item->status = ITEM_ACTIVE;

				AddActiveItem(itemNum);
				AnimateItem(item);

				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
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
	}

	void FullBlockSwitchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->animNumber != Objects[item->objectNumber].animIndex + 2
			|| CurrentSequence >= 3
			|| item->itemFlags[0])
		{
			if (CurrentSequence >= 4)
			{
				item->itemFlags[0] = 0;
				item->goalAnimState = SWITCH_ON;
				item->status = ITEM_NOT_ACTIVE;
				if (++CurrentSequence >= 7)
					CurrentSequence = 0;
			}
		}
		else
		{
			item->itemFlags[0] = 1;
			Sequences[CurrentSequence++] = item->triggerFlags;
		}

		AnimateItem(item);
	}
}