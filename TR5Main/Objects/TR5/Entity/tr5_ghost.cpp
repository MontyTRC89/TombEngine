#include "framework.h"
#include "tr5_ghost.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO InvisibleGhostBite = { 0, 0, 0, 17 };

void InitialiseInvisibleGhost(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = 1;
    item->ActiveState = 1;
    item->Position.yPos += CLICK(2);
}

void InvisibleGhostControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short joint0 = 0;
		short joint2 = 0;
		short joint1 = 0;
		short angle = 0;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
		
		if (item->AIBits)
		{
			GetAITarget(creature);
		}
		else if (creature->hurtByLara)
		{
			creature->enemy = LaraItem;
		}

		AI_INFO info;
		CreatureAIInfo(item, &info);

		angle = CreatureTurn(item, creature->maximumTurn);
		if (abs(info.angle) >= ANGLE(3))
		{
			if (info.angle > 0)
				item->Position.yRot += ANGLE(3);
			else
				item->Position.yRot -= ANGLE(3);
		}
		else
		{
			item->Position.yRot += info.angle;
		}

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			joint1 = info.xAngle;
		}

		creature->maximumTurn = 0;
		
		if (item->ActiveState == 1)
		{
			creature->flags = 0;
			if (info.distance < SQUARE(614))
			{
				if (GetRandomControl() & 1)
					item->TargetState = 2;
				else
					item->TargetState = 3;
			}
		}
		else if (item->ActiveState > 1
			&& item->ActiveState <= 3
			&& !creature->flags
			&& item->TouchBits & 0x9470
			&& item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 18)
		{
			LaraItem->HitPoints -= 400;
			LaraItem->HitStatus = true;
			CreatureEffect2(item, &InvisibleGhostBite, 10, item->Position.yRot, DoBloodSplat);
			creature->flags = 1;
		}

		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		if (info.distance >= SQUARE(1536))
		{
			item->AfterDeath = 125;
			item->ItemFlags[0] = 0;
		}
		else
		{
			item->AfterDeath = sqrt(info.distance) / 16;
			if (item->ItemFlags[0] == 0)
			{
				item->ItemFlags[0] = 1;
				SoundEffect(SFX_TR5_SKELETON_APPEAR, &item->Position, 0);
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}