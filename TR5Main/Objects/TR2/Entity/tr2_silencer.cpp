#include "framework.h"
#include "Objects/TR2/Entity/tr2_silencer.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO silencerGun = { 3, 331, 56, 10 };

void SilencerControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* silencer = (CREATURE_INFO*)item->data;
	AI_INFO info;
	short angle = 0, torso_y = 0, torso_x = 0, head_y = 0, tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 12 && item->currentAnimState != 13)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 13;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, silencer->maximumTurn);

		switch (item->currentAnimState)
		{
		case 3:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 0;
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			break;

		case 4:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 0;

			if (silencer->mood == ESCAPE_MOOD)
			{
				item->requiredAnimState = 2;
				item->goalAnimState = 3;
			}
			else
			{
				if (Targetable(item, &info))
				{
					item->requiredAnimState = (GetRandomControl() >= 0x4000 ? 10 : 6);
					item->goalAnimState = 3;
				}

				if (silencer->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance >= 0x400000)
					{
						item->requiredAnimState = 2;
						item->goalAnimState = 3;
					}
					else
					{
						item->requiredAnimState = 1;
						item->goalAnimState = 3;
					}
				}
				else
				{
					if (GetRandomControl() >= 1280)
					{
						if (GetRandomControl() < 2560)
						{
							item->requiredAnimState = 1;
							item->goalAnimState = 3;
						}
					}
					else
					{
						item->requiredAnimState = 5;
						item->goalAnimState = 3;
					}
				}
			}
			break;
		case 1:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 910;

			if (silencer->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = (GetRandomControl() >= 0x4000 ? 10 : 6);
				item->goalAnimState = 3;
			}
			else
			{

				if (info.distance > 0x400000 || !info.ahead)
					item->goalAnimState = 2;
				if (silencer->mood == BORED_MOOD && GetRandomControl() < 0x300)
					item->goalAnimState = 3;
			}
			break;
		case 2:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 910;
			silencer->flags = 0;
			tilt = (angle / 4);

			if (silencer->mood == ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
					item->goalAnimState = 9;
				break;
			}

			if (Targetable(item, &info))
			{
				if (info.distance >= 0x400000 && info.zoneNumber == info.enemyZone)
					item->goalAnimState = 9;
				break;
			}
			else if (silencer->mood == ATTACK_MOOD)
				item->goalAnimState = (GetRandomControl() >= 0x4000) ? 3 : 2;
			else
				item->goalAnimState = 3;
			break;

		case 5:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 0;

			if (Targetable(item, &info))
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 3;
			}
			else
			{
				if (silencer->mood == ATTACK_MOOD || GetRandomControl() < 0x100)
					item->goalAnimState = 3;
				if (!info.ahead)
					item->goalAnimState = 3;
			}
			break;

		case 6:
		case 10:
			silencer->maximumTurn = 0;
			silencer->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			else
			{
				head_y = info.angle;
			}

			if (silencer->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (Targetable(item, &info))
				item->goalAnimState = item->currentAnimState != 6 ? 11 : 7;
			else
				item->goalAnimState = 3;
			break;
		case 7:
		case 11:
			silencer->maximumTurn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			else
			{
				head_y = info.angle;
			}

			if (!silencer->flags)
			{
				ShotLara(item, &info, &silencerGun, torso_y, 50);
				silencer->flags = 1;
			}
			break;
		case 9:
			silencer->maximumTurn = 910;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			else
			{
				head_y = info.angle;
			}

			if (!item->requiredAnimState)
			{
				if (!ShotLara(item, &info, &silencerGun, torso_y, 50))
					item->goalAnimState = 2;

				item->requiredAnimState = 9;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureAnimation(itemNum, angle, tilt);
}