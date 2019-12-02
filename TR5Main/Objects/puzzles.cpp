#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effect2.h"
#include "..\Game\Lara.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\collide.h"
#include "..\Game\sphere.h"
#include "..\Game\switch.h"

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
				SoundEffect(SFX_TR4_POUR_ID97, &l->pos, 0);
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

		GetLaraJointPosition(&pos, LJ_LHAND);

		DRIP_STRUCT* drip = &Drips[GetFreeDrip()];
		drip->x = pos.x;
		drip->y = pos.y;
		drip->z = pos.z;
		drip->On = 1;
		drip->R = (GetRandomControl() & 0xF) + 24;
		drip->G = (GetRandomControl() & 0xF) + 44;
		drip->B = (GetRandomControl() & 0xF) + 56;
		drip->Yvel = (GetRandomControl() & 0x1F) + 32;
		drip->Gravity = (GetRandomControl() & 0x1F) + 32;
		drip->Life = (GetRandomControl() & 0x1F) + 16;
		drip->RoomNumber = l->roomNumber;
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