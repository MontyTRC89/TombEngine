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
	item->TargetState = 8;

	if (LaraItem->RoomNumber != item->RoomNumber)
		ItemNewRoom(Lara.itemNumber, item->RoomNumber);

	LaraItem->Position.xPos = item->Position.xPos;
	LaraItem->Position.yPos = item->Position.yPos;
	LaraItem->Position.zPos = item->Position.zPos;
	LaraItem->Position.yRot = item->Position.yRot;
	LaraItem->Position.xRot = 0;
	LaraItem->Position.zRot = 0;
	LaraItem->Airborne = false;

	LaraItem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + 1;
	LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
	LaraItem->ActiveState = LS_DEATH;
	LaraItem->TargetState = LS_DEATH;

	LaraItem->HitPoints = -16384;
	Lara.Air = -1;
	Lara.Control.HandStatus = HandStatus::Busy;
	Lara.Control.WeaponControl.GunType = WEAPON_NONE;

	Camera.flags = 1;
	Camera.targetAngle = ANGLE(170);
	Camera.targetElevation = -ANGLE(25);
}

void TyrannosaurControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState == 1)
			item->TargetState = 5;
		else
			item->TargetState = 1;
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

		if (item->TouchBits)
			LaraItem->HitPoints -= (item->ActiveState == 3) ? 10 : 1;

		creature->flags = (creature->mood != ESCAPE_MOOD && !info.ahead &&
			info.enemyFacing > -FRONT_ARC && info.enemyFacing < FRONT_ARC);

		if (!creature->flags && info.distance > SQUARE(1500) && info.distance < SQUARE(4096) && info.bite)
			creature->flags = 1;

		switch (item->ActiveState)
		{
		case 1:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info.distance < SQUARE(1500) && info.bite)
				item->TargetState = 7;
			else if (creature->mood == BORED_MOOD || creature->flags)
				item->TargetState = 2;
			else
				item->TargetState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood != BORED_MOOD || !creature->flags)
				item->TargetState = 1;
			else if (info.ahead && GetRandomControl() < 0x200)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(4);

			if (info.distance < SQUARE(5120) && info.bite)
				item->TargetState = 1;
			else if (creature->flags)
				item->TargetState = 1;
			else if (creature->mood != ESCAPE_MOOD && info.ahead && GetRandomControl() < 0x200)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
			}
			else if (creature->mood == BORED_MOOD)
				item->TargetState = 1;
			break;

		case 7:
			if (item->TouchBits & 0x3000)
			{
				LaraItem->HitPoints -= 1500;
				LaraItem->HitStatus = true;
				item->TargetState = 8;
				if (LaraItem == LaraItem)
					LaraTyrannosaurDeath(item);
			}

			item->RequiredState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, (short)(head * 2));
	creature->jointRotation[1] = creature->jointRotation[0];

	CreatureAnimation(itemNum, angle, 0);

	item->Collidable = true;
}