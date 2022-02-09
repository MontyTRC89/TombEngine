#include "framework.h"
#include "Objects/TR3/Entity/tr3_trex.h"

#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void LaraTyrannosaurDeath(ITEM_INFO* item)
{
	item->targetState = 8;

	if (LaraItem->roomNumber != item->roomNumber)
		ItemNewRoom(Lara.itemNumber, item->roomNumber);

	LaraItem->pos.xPos = item->pos.xPos;
	LaraItem->pos.yPos = item->pos.yPos;
	LaraItem->pos.zPos = item->pos.zPos;
	LaraItem->pos.yRot = item->pos.yRot;
	LaraItem->pos.xRot = 0;
	LaraItem->pos.zRot = 0;
	LaraItem->Airborne = false;

	LaraItem->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + 1;
	LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
	LaraItem->activeState = LS_DEATH;
	LaraItem->targetState = LS_DEATH;

	LaraItem->hitPoints = -16384;
	Lara.air = -1;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.gunType = WEAPON_NONE;

	Camera.flags = 1;
	Camera.targetAngle = ANGLE(170);
	Camera.targetElevation = -ANGLE(25);
}

void TyrannosaurControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short head = 0;
	short angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState == 1)
			item->targetState = 5;
		else
			item->targetState = 1;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->touchBits)
			LaraItem->hitPoints -= (item->activeState == 3) ? 10 : 1;

		creature->flags = (creature->mood != ESCAPE_MOOD && !info.ahead &&
			info.enemyFacing > -FRONT_ARC && info.enemyFacing < FRONT_ARC);

		if (!creature->flags && info.distance > SQUARE(1500) && info.distance < SQUARE(4096) && info.bite)
			creature->flags = 1;

		switch (item->activeState)
		{
		case 1:
			if (item->requiredState)
				item->targetState = item->requiredState;
			else if (info.distance < SQUARE(1500) && info.bite)
				item->targetState = 7;
			else if (creature->mood == BORED_MOOD || creature->flags)
				item->targetState = 2;
			else
				item->targetState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood != BORED_MOOD || !creature->flags)
				item->targetState = 1;
			else if (info.ahead && GetRandomControl() < 0x200)
			{
				item->requiredState = 6;
				item->targetState = 1;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(4);

			if (info.distance < SQUARE(5120) && info.bite)
				item->targetState = 1;
			else if (creature->flags)
				item->targetState = 1;
			else if (creature->mood != ESCAPE_MOOD && info.ahead && GetRandomControl() < 0x200)
			{
				item->requiredState = 6;
				item->targetState = 1;
			}
			else if (creature->mood == BORED_MOOD)
				item->targetState = 1;
			break;

		case 7:
			if (item->touchBits & 0x3000)
			{
				LaraItem->hitPoints -= 1500;
				LaraItem->hitStatus = true;
				item->targetState = 8;
				if (LaraItem == LaraItem)
					LaraTyrannosaurDeath(item);
			}

			item->requiredState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, (short)(head * 2));
	creature->jointRotation[1] = creature->jointRotation[0];

	CreatureAnimation(itemNum, angle, 0);

	item->collidable = true;
}