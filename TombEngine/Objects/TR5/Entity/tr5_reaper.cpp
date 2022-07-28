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

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 2;
	item->Animation.ActiveState = 2;
}

void ReaperControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, TIMID);
		CreatureMood(item, &AI, TIMID);

		short angle = CreatureTurn(item, ANGLE(2.0f));

		if (item->Animation.ActiveState == 2 &&
			!(GetRandomControl() & 0x3F))
			item->Animation.TargetState = 1;

		if (creature->ReachedGoal)
		{
			if (creature->Enemy)
			{
				if (creature->Enemy->Flags & 2)
					item->ItemFlags[3] = (creature->Tosspad & 0xFF) - 1;

				item->ItemFlags[3]++;

				creature->ReachedGoal = false;
				creature->Enemy = NULL;
			}
		}

		item->Pose.Orientation.x = -ANGLE(67.5f);
		CreatureAnimation(itemNumber, angle, 0);
		CreatureUnderwater(item, SECTOR(1));
	}
}
