#include "framework.h"
#include "newobjects.h"
#include "global.h"
#include "box.h"
#include "items.h"
#include "lot.h"
#include "control.h"
#include "effect2.h"
#include "Lara.h"
#include "effect.h"
#include "draw.h"
#include "collide.h"
#include "sphere.h"
#include "switch.h"
#include "tomb4fx.h"
#include "pickup.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "sound.h"

extern DRIP_STRUCT Drips[MAX_DRIPS];

short ScalesBounds[12] = {
	0xFA80, 0xFA80, 0x0000, 0x0000, 0xFE00, 0x0200,
	0xF8E4, 0x071C, 0xEAAC, 0x1554, 0xF8E4, 0x071C
};

void ScalesControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->frameNumber != Anims[item->animNumber].frameEnd)
	{
		AnimateItem(item);
		return;
	}

	if (item->currentAnimState == 1 || item->itemFlags[1])
	{
		if (Objects[item->objectNumber].animIndex)
		{
			RemoveActiveItem(itemNum);
			item->status = ITEM_INACTIVE;
			item->itemFlags[1] = 0;

			AnimateItem(item);
			return;
		}

		if (RespawnAhmet((short)Lara.generalPtr))
		{
			short itemNos[8];
			int sw = GetSwitchTrigger(item, itemNos, 0);

			if (sw > 0)
			{
				while (Items[itemNos[sw]].objectNumber == ID_FLAME_EMITTER2)
				{
					if (--sw <= 0)
						break;
				}
				Items[itemNos[sw]].flags = 1024;
			}

			item->goalAnimState = 1;
		}

		AnimateItem(item);
	}

	int flags = 0;

	if (item->currentAnimState == 2)
	{
		flags = -512;
		RemoveActiveItem(itemNum);
		item->status = ITEM_INACTIVE;
	}
	else
	{
		flags = -1024;
		item->itemFlags[1] = 1;
	}

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	TestTriggers(TriggerIndex, 1, flags);

	AnimateItem(item);
}

void ScalesCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TestBoundsCollide(item, l, 100))
	{
		if (l->animNumber != 400 && l->animNumber != 402 || item->currentAnimState != 1)
		{
			GlobalCollisionBounds.X1 = 640;
			GlobalCollisionBounds.X2 = 1280;
			GlobalCollisionBounds.Y1 = -1280;
			GlobalCollisionBounds.Y2 = 0;
			GlobalCollisionBounds.Z1 = -256;
			GlobalCollisionBounds.Z2 = 384;

			ItemPushLara(item, l, coll, 0, 2);

			GlobalCollisionBounds.X1 = -256;
			GlobalCollisionBounds.X2 = 256;

			ItemPushLara(item, l, coll, 0, 2);

			GlobalCollisionBounds.X1 = -1280;
			GlobalCollisionBounds.X2 = -640;

			ItemPushLara(item, l, coll, 0, 2);
		}
		else
		{
			short rotY = item->pos.yRot;
			item->pos.yRot = (short)(l->pos.yRot + ANGLE(45)) & 0xC000;

			ScalesBounds[0] = -1408;
			ScalesBounds[1] = -640;
			ScalesBounds[4] = -512;
			ScalesBounds[5] = 0;

			if (TestLaraPosition(ScalesBounds, item, l))
			{
				l->animNumber = 402;
				l->frameNumber = Anims[item->animNumber].frameBase;
				item->pos.yRot = rotY;
			}
			else if (l->frameNumber == Anims[402].frameBase + 51)
			{
				SoundEffect(SFX_TR4_POUR, &l->pos, 0);
				item->pos.yRot = rotY;
			}
			else if (l->frameNumber == Anims[402].frameBase + 74)
			{
				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;

				if (l->itemFlags[3] < item->triggerFlags)
				{
					item->goalAnimState = 4;
					item->pos.yRot = rotY;
				}
				else if (l->itemFlags[3] == item->triggerFlags)
				{
					item->goalAnimState = 2;
					item->pos.yRot = rotY;
				}
				else
				{
					item->goalAnimState = 3;
				}
			}
			else
			{
				item->pos.yRot = rotY;
			}
		}
	}

	if (l->frameNumber >= Anims[400].frameBase + 44 && 
		l->frameNumber <= Anims[400].frameBase + 72 || 
		l->frameNumber >= Anims[402].frameBase + 51 &&
		l->frameNumber <= Anims[402].frameBase + 74)
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		GetLaraJointPosition(&pos, LM_LHAND);

		DRIP_STRUCT* drip = &Drips[GetFreeDrip()];
		drip->x = pos.x;
		drip->y = pos.y;
		drip->z = pos.z;
		drip->on = 1;
		drip->r = (GetRandomControl() & 0xF) + 24;
		drip->g = (GetRandomControl() & 0xF) + 44;
		drip->b = (GetRandomControl() & 0xF) + 56;
		drip->yVel = (GetRandomControl() & 0x1F) + 32;
		drip->gravity = (GetRandomControl() & 0x1F) + 32;
		drip->life = (GetRandomControl() & 0x1F) + 16;
		drip->roomNumber = l->roomNumber;
	}
}

int RespawnAhmet(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->currentAnimState != 7 || item->frameNumber != Anims[item->animNumber].frameEnd)
		return false;

	FlashFadeR = 255;
	FlashFadeG = 64;
	FlashFadeB = 0;
	FlashFader = 32;

	item->pos.xPos = (item->itemFlags[0] << 10) + 512;
	item->pos.yPos = (item->itemFlags[1] << 8);
	item->pos.zPos = (item->itemFlags[2] << 10) + 512;

	IsRoomOutside(item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (item->roomNumber != IsRoomOutsideNo)
		ItemNewRoom(itemNum, IsRoomOutsideNo);

	item->animNumber = Objects[ID_AHMET].animIndex;
	item->goalAnimState = 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
	item->hitPoints = Objects[ID_AHMET].hitPoints;

	AddActiveItem(itemNum);

	item->flags &= 0xFE;
	item->afterDeath = 0;
	item->status = ITEM_ACTIVE;
	item->collidable = true;

	EnableBaddieAI(itemNum, 1);

	item->triggerFlags = 1;

	return true;
}

void InitialiseLaraDouble(short itemNum)
{
	ClearItem(itemNum);
}

void LaraDoubleControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP, &item->pos, 0);

	if (CreatureActive(itemNum))
	{
		if (item->hitStatus)
		{
			LaraItem->hitPoints += item->hitPoints - 1000;
		}

		item->hitPoints = 1000;

		AnimateItem(item);
	}
}

short SarcophagusBounds[12] = {
	0xFE00, 0x0200, 0xFF9C, 0x0064, 0xFE00, 0x0000,
	0xF8E4, 0x071C, 0xEAAC, 0x1554, 0x0000, 0x0000
};
PHD_VECTOR SarcophagusPosition = { 0x00000000, 0x00000000, 0xFFFFFED4 };

void InitialiseSarcophagus(short itemNum)
{

}

void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TrInput & IN_ACTION &&
		item->status != ITEM_ACTIVE &&
		l->currentAnimState == 2 &&
		l->animNumber == 103 &&
		!Lara.gunStatus ||
		Lara.isMoving && (short)Lara.generalPtr == itemNum)
	{
		if (TestLaraPosition(SarcophagusBounds, item, l))
		{
			if (MoveLaraPosition(&SarcophagusPosition, item, l))
			{
				l->animNumber = 439;
				l->currentAnimState = 89;
				l->frameNumber = Anims[l->animNumber].frameBase;
				item->flags |= 0x3E00;

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
	else if (l->animNumber != 439 || l->frameNumber != Anims[439].frameBase + 113)
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
				if (Objects[currentItem->objectNumber].collision == PickupCollision)
				{
					PickedUpObject(currentItem->objectNumber);
					currentItem->status = ITEM_ACTIVE;
					currentItem->itemFlags[3] = 1;
				}
			}
		}
	}
}