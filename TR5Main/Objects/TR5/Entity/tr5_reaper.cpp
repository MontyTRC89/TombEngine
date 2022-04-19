#include "framework.h"
#include "tr5_reaper.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

void InitialiseReaper(short itemNum)
{
    ITEM_INFO* item;

    ClearItem(itemNum);

    item = &g_Level.Items[itemNum];
    item->animNumber = Objects[item->objectNumber].animIndex + 1;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = 2;
    item->currentAnimState = 2;
}

void ReaperControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item,&info);
		GetCreatureMood(item,&info, TIMID);
		CreatureMood(item,&info, TIMID);

		short angle = CreatureTurn(item, ANGLE(2));

		if (item->currentAnimState == 2 
			&& !(GetRandomControl() & 0x3F))
			item->goalAnimState = 1;

		if (creature->reachedGoal)
		{
			if (creature->enemy)
			{
				if (creature->enemy->flags & 2)
					item->itemFlags[3] = (item->TOSSPAD & 0xFF) - 1;

				item->itemFlags[3]++;

				creature->reachedGoal = false;
				creature->enemy = NULL;
			}
		}

		item->pos.xRot = -12288;
		CreatureAnimation(itemNumber, angle, 0);
		CreatureUnderwater(item, 1024);
	}
}
