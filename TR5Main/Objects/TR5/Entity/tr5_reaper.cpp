#include "framework.h"
#include "tr5_reaper.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

void InitialiseReaper(short itemNumber)
{
	ClearItem(itemNumber);

	auto* item = &g_Level.Items[itemNumber];

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 2;
	item->ActiveState = 2;
}

void ReaperControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = (CREATURE_INFO*)item->Data;

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		short angle = CreatureTurn(item, ANGLE(2.0f));

		if (item->ActiveState == 2 &&
			!(GetRandomControl() & 0x3F))
			item->TargetState = 1;

		if (creature->reachedGoal)
		{
			if (creature->enemy)
			{
				if (creature->enemy->Flags & 2)
					item->ItemFlags[3] = (item->Tosspad & 0xFF) - 1;

				item->ItemFlags[3]++;

				creature->reachedGoal = false;
				creature->enemy = NULL;
			}
		}

		item->Position.xRot = -12288;
		CreatureAnimation(itemNumber, angle, 0);
		CreatureUnderwater(item, SECTOR(1));
	}
}
