#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_dualrevolver.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO workerDualGunL = { -2, 275, 23, 6 };
BITE_INFO workerDualGunR = { 2, 275, 23, 10 };

void WorkerDualGunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* dual;
	AI_INFO info;
	short angle, head_x, head_y, torso_x, torso_y, tilt;

	item = &g_Level.Items[itemNum];
	dual = (CREATURE_INFO*)item->data;
	angle = head_x = head_y = torso_x = torso_y = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 32;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 11;
		}
	}
	else if (LaraItem->hitPoints <= 0)
	{
		item->goalAnimState = 2;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, dual->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
		case 2:
			dual->maximumTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (dual->mood == ATTACK_MOOD || LaraItem->hitPoints > 0)
			{
				if (Targetable(item, &info))
				{
					if (info.distance <= 0x900000)
						item->goalAnimState = 9;
					else
						item->goalAnimState = 3;
				}
				else
				{
					switch (dual->mood)
					{
					case ATTACK_MOOD:
						if (info.distance > 0x19000000 || !info.ahead)
							item->goalAnimState = 4;
						else
							item->goalAnimState = 3;
						break;
					case ESCAPE_MOOD:
						item->goalAnimState = 4;
						break;
					case STALK_MOOD:
						item->goalAnimState = 3;
						break;

					default:
						if (!info.ahead)
							item->goalAnimState = 3;
						break;
					}
				}
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;
		case 3:
			dual->maximumTurn = ANGLE(3);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = 1;
				}
				else
				{
					if (info.angle >= 0)
						item->goalAnimState = 6;
					else
						item->goalAnimState = 5;
				}
			}

			if (dual->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
			}
			else if (dual->mood == ATTACK_MOOD || dual->mood == STALK_MOOD)
			{
				if (info.distance > 0x19000000 || !info.ahead)
					item->goalAnimState = 4;
			}
			else if (LaraItem->hitPoints > 0)
			{
				if (info.ahead)
					item->goalAnimState = 1;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 4:
			dual->maximumTurn = ANGLE(6);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			tilt = angle / 4;

			if (Targetable(item, &info))
			{
				if (info.zoneNumber == info.enemyZone)
				{
					if (info.angle >= 0)
						item->goalAnimState = 6;
					else
						item->goalAnimState = 5;
				}
				else
				{
					item->goalAnimState = 3;
				}
			}
			else if (dual->mood == ATTACK_MOOD)
			{
				if (info.ahead && info.distance < 0x19000000)
					item->goalAnimState = 3;
			}
			else if (LaraItem->hitPoints > 0)
			{
				item->goalAnimState = 1;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 5:
			dual->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 7;
			else
				item->goalAnimState = 3;
			break;
		case 6:
			dual->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 8;
			else
				item->goalAnimState = 3;
			break;
		case 7:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->flags)
			{
				ShotLara(item, &info, &workerDualGunL, torso_y, 50);
				dual->flags = 1;
			}
			break;
		case 8:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->flags)
			{
				ShotLara(item, &info, &workerDualGunR, torso_y, 50);
				dual->flags = 1;
			}
			break;
		case 9:
			dual->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 10;
			else
				item->goalAnimState = 1;
			break;
		case 10:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->flags)
			{
				ShotLara(item, &info, &workerDualGunL, torso_y, 50);
				ShotLara(item, &info, &workerDualGunR, torso_y, 50);
				dual->flags = 1;
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