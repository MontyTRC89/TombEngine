#include "framework.h"
#include "Objects/TR3/Entity/tr3_cobra.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO cobraBite = { 0, 0, 0, 13 };

void InitialiseCobra(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	ClearItem(itemNum);
	item->animNumber = Objects[item->objectNumber].animIndex + 2;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 45;
	item->activeState = item->targetState = 3;
	item->itemFlags[2] = item->hitStatus;
}

void CobraControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != 4)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 4;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 4;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		info.angle += 0xC00;

		GetCreatureMood(item, &info, 1);
		CreatureMood(item, &info, 1);

		creature->target.x = LaraItem->pos.xPos;
		creature->target.z = LaraItem->pos.zPos;
		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
			head = info.angle;

		if (abs(info.angle) < ANGLE(10))
			item->pos.yRot += info.angle;
		else if (info.angle < 0)
			item->pos.yRot -= ANGLE(10);
		else
			item->pos.yRot += ANGLE(10);

		switch (item->activeState)
		{
		case 1:
			creature->flags = 0;
			if (info.distance > SQUARE(2560))
				item->targetState = 3;
			else if ((LaraItem->hitPoints > 0) && ((info.ahead && info.distance < SQUARE(1024)) || item->hitStatus || (LaraItem->Velocity > 15)))
				item->targetState = 2;
			break;

		case 3:
			creature->flags = 0;
			if (item->hitPoints != -16384)
			{
				item->itemFlags[2] = item->hitPoints;
				item->hitPoints = -16384;
			}
			if (info.distance < SQUARE(1536) && LaraItem->hitPoints > 0)
			{
				item->targetState = 0;
				item->hitPoints = item->itemFlags[2];
			}
			break;

		case 2:
			if (creature->flags != 1 && (item->touchBits & 0x2000))
			{
				creature->flags = 1;
				LaraItem->hitPoints -= 80;
				LaraItem->hitStatus = true;
				Lara.poisoned = 0x100;

				CreatureEffect(item, &cobraBite, DoBloodSplat);
			}
			break;

		case 0:
			item->hitPoints = item->itemFlags[2];
			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureAnimation(itemNum, angle, tilt);
}