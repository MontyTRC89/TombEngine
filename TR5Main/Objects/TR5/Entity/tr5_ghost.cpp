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
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
    item->pos.yPos += CLICK(2);
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
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
		
		if (item->aiBits)
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
				item->pos.yRot += ANGLE(3);
			else
				item->pos.yRot -= ANGLE(3);
		}
		else
		{
			item->pos.yRot += info.angle;
		}

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			joint1 = info.xAngle;
		}

		creature->maximumTurn = 0;
		
		if (item->currentAnimState == 1)
		{
			creature->flags = 0;
			if (info.distance < SQUARE(614))
			{
				if (GetRandomControl() & 1)
					item->goalAnimState = 2;
				else
					item->goalAnimState = 3;
			}
		}
		else if (item->currentAnimState > 1
			&& item->currentAnimState <= 3
			&& !creature->flags
			&& item->touchBits & 0x9470
			&& item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 18)
		{
			LaraItem->hitPoints -= 400;
			LaraItem->hitStatus = true;
			CreatureEffect2(item, &InvisibleGhostBite, 10, item->pos.yRot, DoBloodSplat);
			creature->flags = 1;
		}

		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		if (info.distance >= SQUARE(1536))
		{
			item->afterDeath = 125;
			item->itemFlags[0] = 0;
		}
		else
		{
			item->afterDeath = sqrt(info.distance) / 16;
			if (item->itemFlags[0] == 0)
			{
				item->itemFlags[0] = 1;
				SoundEffect(SFX_TR5_SKELETON_APPEAR, &item->pos, 0);
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}