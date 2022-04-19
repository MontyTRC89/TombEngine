#include "framework.h"
#include "Objects/TR2/Entity/tr2_mercenary.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

BITE_INFO mercUziBite = { 0, 150, 19, 17 };
BITE_INFO mercAutoPistolBite = { 0, 230, 9, 17 };

void MercenaryUziControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* mc1;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	mc1 = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 13)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 14;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 13;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);
		angle = CreatureTurn(item, mc1->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			mc1->maximumTurn = 0;

			if (mc1->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance > 0x400000)
				{
					item->goalAnimState = 2;
				}

				if (GetRandomControl() >= 0x2000)
				{
					if (GetRandomControl() >= 0x4000)
						item->goalAnimState = 11;
					else
						item->goalAnimState = 7;
				}
				else
				{
					item->goalAnimState = 5;
				}
			}
			else
			{
				if (mc1->mood == ATTACK_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (!info.ahead)
				{
					item->goalAnimState = 2;
				}
				else
				{
					item->goalAnimState = 1;
				}
			}
			break;
		case 2:
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			mc1->maximumTurn = ANGLE(7.0f);

			if (mc1->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance <= 0x400000 || info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = 1;
				}
				else
				{
					item->goalAnimState = 12;
				}
			}
			else if (mc1->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			else
			{
				if (info.ahead)
				{
					item->goalAnimState = 2;
				}
				else
				{
					item->goalAnimState = 1;
				}
			}
			break;
		case 3:
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			mc1->maximumTurn = ANGLE(10.0f);
			tilt = (angle / 3);

			if (mc1->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = 1;
				}
				else if (mc1->mood == BORED_MOOD)
				{
					item->goalAnimState = 2;
				}
			}
			break;

		case 5:
		case 7:
		case 8:
		case 9:
			mc1->maximumTurn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!ShotLara(item, &info, &mercUziBite, torso_y, 8))
				item->goalAnimState = 1;

			if (info.distance < 0x400000)
				item->goalAnimState = 1;
			break;

		case 10:
		case 14:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!ShotLara(item, &info, &mercUziBite, torso_y, 8))
				item->goalAnimState = 1;

			if (info.distance < 0x400000)
				item->goalAnimState = 2;
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}

void MercenaryAutoPistolControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* mc2;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	mc2 = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 9;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 11;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, mc2->maximumTurn);

		switch (item->currentAnimState)
		{
		case 2:
			mc2->maximumTurn = 0;
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (mc2->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance <= 0x400000)
				{
					if (GetRandomControl() >= 0x2000)
					{
						if (GetRandomControl() >= 0x4000)
							item->goalAnimState = 5;
						else
							item->goalAnimState = 8;
					}
					else
					{
						item->goalAnimState = 7;
					}
				}
				else
				{
					item->goalAnimState = 3;
				}
			}
			else
			{
				if (mc2->mood == ATTACK_MOOD)
					item->goalAnimState = 4;
				if (!info.ahead || GetRandomControl() < 0x100)
					item->goalAnimState = 3;
			}
			break;
		case 3:
			mc2->maximumTurn = ANGLE(7);
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (mc2->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x400000 || info.zoneNumber == info.enemyZone || GetRandomControl() < 1024)
					item->goalAnimState = 2;
				else
					item->goalAnimState = 1;
			}
			else if (mc2->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
			}
			else if (info.ahead && GetRandomControl() < 1024)
			{
				item->goalAnimState = 2;
			}
			break;
		case 4:
			mc2->maximumTurn = ANGLE(10);
			tilt = (angle / 3);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (mc2->mood != ESCAPE_MOOD && (mc2->mood == ESCAPE_MOOD || Targetable(item, &info)))
				item->goalAnimState = 2;
			break;
		case 1:
		case 5:
		case 6:
			mc2->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			break;
		case 7:
		case 8:
		case 13:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;

				if (!mc2->flags)
				{
					if (GetRandomControl() < 0x2000)
						item->goalAnimState = 2;

					ShotLara(item, &info, &mercAutoPistolBite, torso_y, 50);
					mc2->flags = 1;
				}
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 9:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;

				if (info.distance < 0x400000)
					item->goalAnimState = 3;

				if (mc2->flags != 1)
				{
					if (!ShotLara(item, &info, &mercAutoPistolBite, torso_y, 50))
						item->goalAnimState = 3;
					mc2->flags = 1;
				}
			}
			else
			{
				item->goalAnimState = 3;
			}
			break;
		case 12:
			mc2->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 13;
			else
				item->goalAnimState = 2;
			break;
		case 10:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;

				if (info.distance < 0x400000)
					item->goalAnimState = 3;

				if (mc2->flags != 2)
				{
					if (!ShotLara(item, &info, &mercAutoPistolBite, torso_y, 50))
						item->goalAnimState = 3;
					mc2->flags = 2;
				}
			}
			else
			{
				item->goalAnimState = 3;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}