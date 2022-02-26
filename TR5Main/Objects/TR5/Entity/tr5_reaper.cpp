#include "framework.h"
#include "tr5_reaper.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
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
		auto* info = GetCreatureInfo(item);

		if (item->AIBits)
			GetAITarget(info);
		else
			info->enemy = LaraItem;

		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, TIMID);
		CreatureMood(item, &aiInfo, TIMID);

		short angle = CreatureTurn(item, ANGLE(2.0f));

		if (item->ActiveState == 2 &&
			!(GetRandomControl() & 0x3F))
			item->TargetState = 1;

		if (info->reachedGoal)
		{
			if (info->enemy)
			{
				if (info->enemy->Flags & 2)
					item->ItemFlags[3] = (item->Tosspad & 0xFF) - 1;

				item->ItemFlags[3]++;

				info->reachedGoal = false;
				info->enemy = NULL;
			}
		}

		item->Position.xRot = -ANGLE(67.5f);
		CreatureAnimation(itemNumber, angle, 0);
		CreatureUnderwater(item, SECTOR(1));
	}
}
