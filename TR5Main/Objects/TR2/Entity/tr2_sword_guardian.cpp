#include "framework.h"
#include "Objects/TR2/Entity/tr2_sword_guardian.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"

BITE_INFO swordBite = { 0, 37, 550, 15 };

void InitialiseSwordGuardian(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	item = &g_Level.Items[itemNum];

	ClearItem(itemNum);
}

static void SwordGuardianFly(ITEM_INFO* item)
{
	PHD_VECTOR pos;

	pos.x = (GetRandomControl() * 256 / 32768) + item->pos.xPos - 128;
	pos.y = (GetRandomControl() * 256 / 32768) + item->pos.yPos - 256;
	pos.z = (GetRandomControl() * 256 / 32768) + item->pos.zPos - 128;

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

	item = &g_Level.Items[itemNum];
	sword = (CREATURE_INFO*)item->data;
	angle = head = torso = 0;
	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 12)
		{
			//item->meshBits >>= 1;
			SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 0);
			SoundEffect(SFX_TR4_EXPLOSION2, &LaraItem->pos, 0);
			//item->meshBits = 0xFFFFFFFF;
			//item->objectNumber = ID_SAS;
			ExplodingDeath(itemNum, -1, 256);
			//item->objectNumber = ID_SWAT;
			DisableBaddieAI(itemNum);
			KillItem(itemNum);
			//item->status = ITEM_DESACTIVATED;
			//item->flags |= ONESHOT;
			item->currentAnimState = 12;
		}
		return;
	}
	else
	{
		sword->LOT.step = STEP_SIZE;
		sword->LOT.drop = -STEP_SIZE;
		sword->LOT.fly = NO_FLYING;
		sword->LOT.zone = ZONE_BASIC;
		CreatureAIInfo(item, &info);

		if (item->currentAnimState == 8)
		{
			if (info.zoneNumber != info.enemyZone)
			{
				sword->LOT.step = WALL_SIZE * 20;
				sword->LOT.drop = -WALL_SIZE * 20;
				sword->LOT.fly = STEP_SIZE / 4;
				sword->LOT.zone = ZONE_FLYER;
				CreatureAIInfo(item, &info);
			}
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, sword->maximumTurn);

		if (item->currentAnimState != 9)
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