#include "../newobjects.h"
#include "../../Game/Box.h"
#include "../../Game/effects.h"
#include "../../Game/lot.h"
#include "../../Game/effect2.h"
#include "../../Game/items.h"
#include "../../Game/tomb4fx.h"
#include "..\..\Specific\level.h"
#include "../../Game/lara.h"

BITE_INFO swordBite = { 0, 37, 550, 15 };

void InitialiseSwordGuardian(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	item = &Items[itemNum];

	ClearItem(itemNum);

	//item->status = ITEM_INACTIVE;
	//item->meshBits = 0;
}

void SwordGuardianFly(ITEM_INFO* item)
{
	PHD_VECTOR pos;

	pos.x = (GetRandomControl() << 8 >> 15) + item->pos.xPos - 128;
	pos.y = (GetRandomControl() << 8 >> 15) + item->pos.yPos - 256;
	pos.z = (GetRandomControl() << 8 >> 15) + item->pos.zPos - 128;

	TriggerGunSmoke(pos.x, pos.y, pos.z, 1, 1, 1, 1, WEAPON_GRENADE_LAUNCHER, 32);
	SoundEffect(SFX_TR2_WARRIOR_HOVER, &item->pos, 0);
}

void SwordGuardianControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* sword;
	AI_INFO info;
	short angle, head, torso;
	bool lara_alive;

	item = &Items[itemNum];
	sword = (CREATURE_INFO*)item->data;
	angle = head = torso = 0;
	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 12)
		{
			//item->meshBits >>= 1;
			SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0);
			SoundEffect(SFX_EXPLOSION2, &LaraItem->pos, 0);
			//item->meshBits = 0xFFFFFFFF;
			//item->objectNumber = ID_SAS;
			ExplodingDeath(itemNum, -1, 256);
			//item->objectNumber = ID_SWAT;
			DisableBaddieAI(itemNum);
			KillItem(itemNum);
			//item->status = ITEM_DEACTIVATED;
			//item->flags |= ONESHOT;
			item->currentAnimState = 12;

			/*
			if (!item->meshBits)
			{
				SoundEffect(105, NULL, 0);
				item->meshBits = 0xFFFFFFFF;
				item->objectNumber = ID_SAS;
				ExplodingDeath(itemNum, -1, 256);
				item->objectNumber = ID_SWAT;
				DisableBaddieAI(itemNum);
				KillItem(itemNum);
				item->status = ITEM_DEACTIVATED;
				item->flags |= ONESHOT;
			}
			*/
		}
		return;
	}
	else
	{
		/* Get ground based information */
		sword->LOT.step = STEP_SIZE;
		sword->LOT.drop = -STEP_SIZE;
		sword->LOT.fly = NO_FLYING;
		sword->LOT.zone = 1;
		CreatureAIInfo(item, &info);

		if (item->currentAnimState == 8)
		{
			/* If flying and not in same zone, then use fly zone */
			if (info.zoneNumber != info.enemyZone)
			{
				sword->LOT.step = WALL_SIZE * 20;
				sword->LOT.drop = -WALL_SIZE * 20;
				sword->LOT.fly = STEP_SIZE / 4;
				sword->LOT.zone = 4;
				CreatureAIInfo(item, &info);
			}
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, sword->maximumTurn);

		if (item->currentAnimState != 9) // for reload
			item->meshBits = 0xFFFFFFFF;

		switch (item->currentAnimState)
		{
		case 9:
			sword->maximumTurn = 0;

			if (!sword->flags)
			{
				item->meshBits = (item->meshBits << 1) + 1;
				sword->flags = 3;
			}
			else
			{
				sword->flags--;
			}
			break;

		case 1:
			sword->maximumTurn = 0;

			if (info.ahead)
				head = info.angle;

			if (lara_alive)
			{
				if (info.bite && info.distance < 0x100000)
				{
					if (GetRandomControl() >= 0x4000)
						item->goalAnimState = 5;
					else
						item->goalAnimState = 3;
				}
				else
				{
					if (info.zoneNumber == info.enemyZone)
						item->goalAnimState = 2;
					else
						item->goalAnimState = 8;
				}
			}
			else
			{
				item->goalAnimState = 7;
			}
			break;

		case 2:
			sword->maximumTurn = ANGLE(9);

			if (info.ahead)
				head = info.angle;

			if (lara_alive)
			{
				if (info.bite && info.distance < 0x400000)
					item->goalAnimState = 10;
				else if (info.zoneNumber != info.enemyZone)
					item->goalAnimState = 1;
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;

		case 3:
			sword->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (!info.bite || info.distance > 0x100000)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 4;
			break;

		case 5:
			sword->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (!info.bite || info.distance > 0x100000)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 6;
			break;

		case 10:
			sword->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (!info.bite || info.distance > 0x400000)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 11;
			break;

		case 8:
			sword->maximumTurn = ANGLE(7);

			if (info.ahead)
				head = info.angle;

			SwordGuardianFly(item);

			if (!sword->LOT.fly)
				item->goalAnimState = 1;
			break;

		case 4:
		case 6:
		case 11:
			if (info.ahead)
				torso = info.angle;

			if (!sword->flags && (item->touchBits & 0xC000))
			{
				LaraItem->hitPoints -= 300;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &swordBite, DoBloodSplat);
				sword->flags = 1;
			}
			break;
		}
	}

	if (item->hitPoints > 0)
	{
		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNum, angle, 0);
	}
}