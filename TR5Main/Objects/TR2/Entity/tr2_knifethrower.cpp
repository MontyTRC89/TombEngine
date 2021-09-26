#include "framework.h"
#include "tr2_knifethrower.h"
#include "items.h"
#include "effects/effects.h"
#include "control/box.h"
#include "people.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "Sound/sound.h"
#include "itemdata/creature_info.h"
#include "floordata.h"
#include "collide.h"
#include "missile.h"
BITE_INFO knifeLeft = { 0, 0, 0, 5 };
BITE_INFO knifeRight = { 0, 0, 0, 8 };

void KnifeControl(short fxNum)
{
	ITEM_INFO* fx;
	int speed;
	short roomNumber;
	FLOOR_INFO* floor;

	fx = &g_Level.Items[fxNum];
	FX_INFO* fxInfo = fx->data;
	if (fxInfo->counter <= 0)
	{
		KillEffect(fxNum);
		return;
	}
	else
	{
		fxInfo->counter--;
	}

	speed = fx->speed * phd_cos(fx->pos.xRot);
	fx->pos.zPos += speed * phd_cos(fx->pos.yRot);
	fx->pos.xPos += speed * phd_sin(fx->pos.yRot);
	fx->pos.yPos += fx->speed * phd_sin(-fx->pos.xRot);

	roomNumber = fx->roomNumber;
	floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);

	if (fx->pos.yPos >= GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos) || fx->pos.yPos <= GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos))
	{
		KillEffect(fxNum);
		return;
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fxNum, roomNumber);

	fx->pos.zRot += ANGLE(30);

	if (ItemNearLara(&fx->pos, 200))
	{
		LaraItem->hitPoints -= 50;
		LaraItem->hitStatus = true;

		fx->pos.yRot = LaraItem->pos.yRot;
		fx->speed = LaraItem->speed;
		fx->frameNumber = fxInfo->counter = 0;

		SoundEffect(SFX_TR2_CRUNCH2, &fx->pos, 0);
		DoBloodSplat(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, 80, fx->pos.yRot, fx->roomNumber);
		KillEffect(fxNum);
	}
}

static short ThrowKnife(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	PHD_3DPOS pos{ x,y,z };
	short fx_number = CreateNewEffect(roomNumber,ID_KNIFETHROWER_KNIFE, pos);
	if (fx_number != NO_ITEM) {
		ITEM_INFO* fx = &g_Level.Items[fx_number];
		FX_INFO* fxInfo = fx->data;
		fxInfo->counter = 200;
		fx->speed = speed + 200;
		ShootAtLara(fx);
	}
	// TODO: add fx parameters

	return fx_number;
}

void KnifethrowerControl(short itemNum)
{
	ITEM_INFO* item;
	CREATURE_INFO* knife;
	AI_INFO info;
	short angle, torso, head, tilt;

	item = &g_Level.Items[itemNum];
	knife = (CREATURE_INFO*)item->data;
	angle = torso = head = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 10)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 23;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 10;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, knife->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			knife->maximumTurn = 0;

			if (info.ahead)
				head = info.angle;

			if (knife->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				item->goalAnimState = 8;
			}
			else if (knife->mood == BORED_MOOD)
			{
				if (!info.ahead || info.distance > SQUARE(WALL_SIZE*6))
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE*4))
			{
				item->goalAnimState = 2;
			}
			else
			{
				item->goalAnimState = 3;
			}
			break;

		case 2:
			knife->maximumTurn = ANGLE(3);

			if (info.ahead)
				head = info.angle;

			if (knife->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE((WALL_SIZE*5)/2) || info.zoneNumber != info.enemyZone)
					item->goalAnimState = 1;
				else if (GetRandomControl() < 0x4000)
					item->goalAnimState = 4;
				else
					item->goalAnimState = 6;
			}
			else if (knife->mood == BORED_MOOD)
			{
				if (info.ahead && info.distance < SQUARE(WALL_SIZE*6))
					item->goalAnimState = 1;
			}
			else if (!info.ahead || info.distance > SQUARE(WALL_SIZE*4))
			{
				item->goalAnimState = 3;
			}
			break;

		case 3:
			knife->maximumTurn = ANGLE(6);
			tilt = (angle / 3);

			if (info.ahead)
				head = info.angle;

			if (Targetable(item, &info))
			{
				item->goalAnimState = 2;
			}
			else if (knife->mood == BORED_MOOD)
			{
				if (info.ahead && info.distance < SQUARE(WALL_SIZE*6))
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE*4))
			{
				item->goalAnimState = 2;
			}
			break;

		case 4:
			knife->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (Targetable(item, &info))
				item->goalAnimState = 5;
			else
				item->goalAnimState = 2;
			break;

		case 6:
			knife->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (Targetable(item, &info))
				item->goalAnimState = 7;
			else
				item->goalAnimState = 2;
			break;

		case 8:
			knife->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (Targetable(item, &info))
				item->goalAnimState = 9;
			else
				item->goalAnimState = 1;
			break;

		case 5:
			if (info.ahead)
				torso = info.angle;

			if (!knife->flags)
			{
				CreatureEffect(item, &knifeLeft, ThrowKnife);
				knife->flags = 1;
			}
			break;

		case 7:
			if (info.ahead)
				torso = info.angle;

			if (!knife->flags)
			{
				CreatureEffect(item, &knifeRight, ThrowKnife);
				knife->flags = 1;
			}
			break;

		case 9:
			if (info.ahead)
				torso = info.angle;

			if (!knife->flags)
			{
				CreatureEffect(item, &knifeLeft, ThrowKnife);
				CreatureEffect(item, &knifeRight, ThrowKnife);
				knife->flags = 1;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, head);
	CreatureAnimation(itemNum, angle, tilt);
}