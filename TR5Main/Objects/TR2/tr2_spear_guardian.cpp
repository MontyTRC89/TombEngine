#include "../newobjects.h"
#include "../../Game/Box.h"
#include "../../Game/effects.h"
#include "../../Game/items.h"
#include "../../specific/setup.h"
#include "..\..\Specific\level.h"

BITE_INFO spearLeftBite = { 0, 0, 920, 11 };
BITE_INFO spearRightBite = { 0, 0, 920, 18 };

void XianDamage(ITEM_INFO* item, CREATURE_INFO* xian, int damage)
{
	if (!(xian->flags & 1) && (item->touchBits & 0x40000))
	{
		LaraItem->hitPoints -= damage;
		LaraItem->hitStatus = true;
		CreatureEffect(item, &spearRightBite, DoBloodSplat);
		xian->flags |= 1;
		SoundEffect(SFX_TR2_CRUNCH3, &item->pos, 0);
	}

	if (!(xian->flags & 2) && (item->touchBits & 0x800))
	{
		LaraItem->hitPoints -= damage;
		LaraItem->hitStatus = true;
		CreatureEffect(item, &spearLeftBite, DoBloodSplat);
		xian->flags |= 2;
		SoundEffect(SFX_TR2_CRUNCH3, &item->pos, 0);
	}
}

void InitialiseSpearGuardian(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	ClearItem(itemNum);

	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 48;

	anim = &Anims[item->animNumber];

	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;

	//item->status = ITEM_INACTIVE;
	//item->meshBits = 0;
}

void SpearGuardianControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* xian;
	short angle, head, neck, tilt;
	int random, lara_alive;
	AI_INFO info;

	item = &Items[itemNum];
	xian = (CREATURE_INFO*)item->data;
	head = neck = angle = tilt = 0;
	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		item->currentAnimState = 17;
		item->meshBits >>= 1;

		if (!item->meshBits)
		{
			SoundEffect(105, NULL, 0);
			/*
			item->meshBits = 0xffffffff;
			item->objectNumber = ID_SPEAR_GUARDIAN_STATUE; // just to fool ExplodingDeath to produce jade chunks
			ExplodingDeath(itemNum, 0xffffffff, 0);
			item->objectNumber = ID_SPEAR_GUARDIAN;
			DisableBaddieAI(itemNum);
			KillItem(itemNum);
			item->status = ITEM_DEACTIVATED;
			item->flags |= ONESHOT;
			*/
		}
		return;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, xian->maximumTurn);

		if (item->currentAnimState != 18) // for reload
			item->meshBits = 0xFFFFFFFF;

		switch (item->currentAnimState)
		{
		case 18:
			/* Make jade man come to life!! */
			if (!xian->flags)
			{
				item->meshBits = (item->meshBits << 1) + 1;
				xian->flags = 3;
			}
			else
				xian->flags--;
			break;

		case 1:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = 0;

			if (xian->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->goalAnimState = 2;
				else if (random < 0x400)
					item->goalAnimState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 5;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = 0;

			if (xian->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (xian->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->goalAnimState = 1;
				else if (random < 0x400)
					item->goalAnimState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 13;
			else
				item->goalAnimState = 3;
			break;

		case 3:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = ANGLE(3);

			if (xian->mood == ESCAPE_MOOD)
				item->goalAnimState = 4;
			else if (xian->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->goalAnimState = 1;
				else if (random < 0x400)
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				if (info.distance < SQUARE(WALL_SIZE * 3 / 2))
					item->goalAnimState = 7;
				else if (GetRandomControl() < 0x4000)
					item->goalAnimState = 9;
				else
					item->goalAnimState = 11;
			}
			else if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 3))
				item->goalAnimState = 4;
			break;

		case 4:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = ANGLE(5);

			if (xian->mood == ESCAPE_MOOD)
				break;
			else if (xian->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2)) // the only way he'll ever break out of a run is to attack Lara
				item->goalAnimState = 15;
			break;

		case 5:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE))
				item->goalAnimState = 1;
			else
				item->goalAnimState = 6;
			break;

		case 7:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 3 / 2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 8;
			break;

		case 9:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 8;
			break;

		case 11:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 8;
			break;

		case 13:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE))
				item->goalAnimState = 2;
			else
				item->goalAnimState = 14;
			break;

		case 15:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 4;
			else
				item->goalAnimState = 16;
			break;

		case 6:
			XianDamage(item, xian, 75);
			break;

		case 8:
		case 10:
		case 12:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 75);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else
				item->goalAnimState = 3;
			break;

		case 14:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 75);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 1;
			else
				item->goalAnimState = 2;
			break;

		case 16:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 120);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 4;
			break;
		}
	}

	if (lara_alive && LaraItem->hitPoints <= 0)
	{
		CreatureKill(item, 49, 19, 2); // uses EXTRA_YETIKILL slot
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNum, angle, tilt);
}