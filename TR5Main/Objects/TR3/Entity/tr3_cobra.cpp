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
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + 45;
	item->ActiveState = item->TargetState = 3;
	item->ItemFlags[2] = item->HitStatus;
}

void CobraControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 4)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 4;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		info.angle += 0xC00;

		GetCreatureMood(item, &info, 1);
		CreatureMood(item, &info, 1);

		creature->target.x = LaraItem->Position.xPos;
		creature->target.z = LaraItem->Position.zPos;
		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
			head = info.angle;

		if (abs(info.angle) < ANGLE(10))
			item->Position.yRot += info.angle;
		else if (info.angle < 0)
			item->Position.yRot -= ANGLE(10);
		else
			item->Position.yRot += ANGLE(10);

		switch (item->ActiveState)
		{
		case 1:
			creature->flags = 0;
			if (info.distance > SQUARE(2560))
				item->TargetState = 3;
			else if ((LaraItem->HitPoints > 0) && ((info.ahead && info.distance < SQUARE(1024)) || item->HitStatus || (LaraItem->Velocity > 15)))
				item->TargetState = 2;
			break;

		case 3:
			creature->flags = 0;
			if (item->HitPoints != -16384)
			{
				item->ItemFlags[2] = item->HitPoints;
				item->HitPoints = -16384;
			}
			if (info.distance < SQUARE(1536) && LaraItem->HitPoints > 0)
			{
				item->TargetState = 0;
				item->HitPoints = item->ItemFlags[2];
			}
			break;

		case 2:
			if (creature->flags != 1 && (item->TouchBits & 0x2000))
			{
				creature->flags = 1;
				LaraItem->HitPoints -= 80;
				LaraItem->HitStatus = true;
				Lara.poisoned = 0x100;

				CreatureEffect(item, &cobraBite, DoBloodSplat);
			}
			break;

		case 0:
			item->HitPoints = item->ItemFlags[2];
			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureAnimation(itemNum, angle, tilt);
}