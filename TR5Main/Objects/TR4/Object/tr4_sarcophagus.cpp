#include "framework.h"
#include "tr4_sarcophagus.h"
#include "level.h"
#include "input.h"
#include "lara.h"
#include "items.h"
#include "pickup.h"
#include "setup.h"

static short SarcophagusBounds[12] =
{
	0xFE00, 512, 0xFF9C, 100, 0xFE00, 0,
	0xF8E4, 1820, 0xEAAC, 5460, 0, 0
};
static PHD_VECTOR SarcophagusPosition(0, 0, -300);

void InitialiseSarcophagus(short itemNum)
{

}

void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TrInput & IN_ACTION &&
		item->status != ITEM_ACTIVE &&
		l->currentAnimState == STATE_LARA_STOP &&
		l->animNumber == ANIMATION_LARA_STAY_IDLE &&
		Lara.gunStatus == LG_NO_ARMS ||
		Lara.isMoving && (short)Lara.generalPtr == itemNum)
	{
		if (TestLaraPosition(SarcophagusBounds, item, l))
		{
			if (MoveLaraPosition(&SarcophagusPosition, item, l))
			{
				l->animNumber = ANIMATION_LARA_PICKUP_SARCOPHAGUS;
				l->currentAnimState = STATE_LARA_MISC_CONTROL;
				l->frameNumber = Anims[l->animNumber].frameBase;
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
				Lara.generalPtr = (void*)itemNum;
			}
		}
		else if (Lara.isMoving)
		{
			if ((short)Lara.generalPtr == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}
	}
	else if (l->animNumber != ANIMATION_LARA_PICKUP_SARCOPHAGUS || l->frameNumber != Anims[ANIMATION_LARA_PICKUP_SARCOPHAGUS].frameBase + 113)
	{
		ObjectCollision(itemNum, l, coll);
	}
	else
	{
		short linknum;
		for (linknum = Items[Rooms[item->roomNumber].itemNumber].nextItem; linknum != NO_ITEM; linknum = Items[linknum].nextItem)
		{
			ITEM_INFO* currentItem = &Items[linknum];

			if (linknum != itemNum && currentItem->pos.xPos == item->pos.xPos && currentItem->pos.zPos == item->pos.zPos)
			{
				if (Objects[currentItem->objectNumber].isPickup)
				{
					PickedUpObject(currentItem->objectNumber);
					currentItem->status = ITEM_ACTIVE;
					currentItem->itemFlags[3] = 1;
				}
			}
		}
	}
}