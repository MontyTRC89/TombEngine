#include "framework.h"
#include "tr4_sarcophagus.h"
#include "level.h"
#include "input.h"
#include "lara.h"
#include "items.h"
#include "pickup.h"
#include "setup.h"
#include "health.h"

static PHD_VECTOR SarcophagusPosition(0, 0, -300);
OBJECT_COLLISION_BOUNDS SarcophagusBounds =
{ -512, 512, -100, 100, -512, 0, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

void InitialiseSarcophagus(short itemNum)
{

}

void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (TrInput & IN_ACTION &&
		item->status != ITEM_ACTIVE &&
		l->currentAnimState == LS_STOP &&
		l->animNumber == LA_STAND_IDLE &&
		Lara.gunStatus == LG_NO_ARMS ||
		Lara.isMoving && Lara.interactedItem == itemNum)
	{
		if (TestLaraPosition(&SarcophagusBounds, item, l))
		{
			if (MoveLaraPosition(&SarcophagusPosition, item, l))
			{
				l->animNumber = LA_PICKUP_SARCOPHAGUS;
				l->currentAnimState = LS_MISC_CONTROL;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				item->flags |= IFLAG_ACTIVATION_MASK;

				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;

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
		else if (Lara.isMoving)
		{
			if (Lara.interactedItem == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}
	}
	else if (l->animNumber != LA_PICKUP_SARCOPHAGUS || l->frameNumber != g_Level.Anims[LA_PICKUP_SARCOPHAGUS].frameBase + 113)
	{
		ObjectCollision(itemNum, l, coll);
	}
	else
	{
		short linknum;
		for (linknum = g_Level.Items[g_Level.Rooms[item->roomNumber].itemNumber].nextItem; linknum != NO_ITEM; linknum = g_Level.Items[linknum].nextItem)
		{
			ITEM_INFO* currentItem = &g_Level.Items[linknum];

			if (linknum != itemNum && currentItem->pos.xPos == item->pos.xPos && currentItem->pos.zPos == item->pos.zPos)
			{
				if (Objects[currentItem->objectNumber].isPickup)
				{
					PickedUpObject(static_cast<GAME_OBJECT_ID>(currentItem->objectNumber), 0);
					currentItem->status = ITEM_ACTIVE;
					currentItem->itemFlags[3] = 1;
					AddDisplayPickup(currentItem->objectNumber);
				}
			}
		}
	}
}