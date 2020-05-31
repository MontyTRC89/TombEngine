#include "framework.h"
#include "tr5_cogswitch.h"
#include "level.h"
#include "control.h"
#include "input.h"
#include "lara.h"
#include "items.h"
#include "sound.h"
#include "door.h"

static PHD_VECTOR CogSwitchPos(0, 0, -856);
static short CogSwitchBounds[12] =
{
	0xFE00, 0x0200, 0x0000, 0x0000, 0xFA00, 0xFE00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};

void CogSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (!TriggerIndex)
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}

	short* trigger = TriggerIndex;
	for (int i = *TriggerIndex; (i & 0x1F) != 4; trigger++)
	{
		if (i < 0)
			break;
		i = trigger[1];
	}

	ITEM_INFO* target = &Items[trigger[3] & 0x3FF];
	DOOR_DATA* door = (DOOR_DATA*)target->data;

	if (item->status == ITEM_NOT_ACTIVE)
	{
		if (!(item->flags & 0x100)
			&& (TrInput & IN_ACTION
				&& !Lara.gunStatus
				&& !(item->status && item->gravityStatus)
				&& l->currentAnimState == STATE_LARA_STOP
				&& l->animNumber == ANIMATION_LARA_STAY_IDLE
				|| Lara.isMoving && Lara.generalPtr == (void*)itemNum))
		{
			if (TestLaraPosition(CogSwitchBounds, item, l))
			{
				if (MoveLaraPosition(&CogSwitchPos, item, l))
				{
					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.generalPtr = target;
					l->animNumber = ANIMATION_LARA_COGWHEEL_GRAB;
					l->goalAnimState = STATE_LARA_COGWHEEL;
					l->currentAnimState = STATE_LARA_COGWHEEL;
					l->frameNumber = Anims[l->animNumber].frameBase;

					AddActiveItem(itemNum);

					item->goalAnimState = 1;
					item->status = ITEM_ACTIVE;
					if (!door->opened)
					{
						AddActiveItem((target - Items));
						target->itemFlags[2] = target->pos.yPos;
						target->status = ITEM_ACTIVE;
					}
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
				return;
			}
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}

		ObjectCollision(itemNum, l, coll);
	}
}

void CogSwitchControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	AnimateItem(item);

	if (item->currentAnimState == 1)
	{
		if (item->goalAnimState == 1 && !(TrInput & IN_ACTION))
		{
			LaraItem->goalAnimState = STATE_LARA_STOP;
			item->goalAnimState = 0;
		}

		if (LaraItem->animNumber == ANIMATION_LARA_COGWHEEL_PULL)
		{
			if (LaraItem->frameNumber == (Anims[ANIMATION_LARA_COGWHEEL_PULL].frameBase + 10))
			{
				ITEM_INFO* it = (ITEM_INFO*)Lara.generalPtr;
				it->itemFlags[0] = 40;
				Lara.generalPtr = it;
				SoundEffect(SFX_STONE_SCRAPE_FAST, &it->pos, 0);
			}
		}
	}
	else
	{
		if (item->frameNumber == Anims[item->animNumber].frameEnd)
		{
			item->currentAnimState = 0;
			item->status = ITEM_NOT_ACTIVE;
			RemoveActiveItem(itemNum);
			LaraItem->animNumber = ANIMATION_LARA_STAY_SOLID;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = STATE_LARA_STOP;
			LaraItem->currentAnimState = STATE_LARA_STOP;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
}