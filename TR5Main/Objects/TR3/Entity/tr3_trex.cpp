#include "framework.h"
#include "Objects/TR3/Entity/tr3_trex.h"

#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

// TODO
enum TRexState
{

};

// TODO
enum TRexAnim
{

};

void LaraTRexDeath(ITEM_INFO* tRexItem, ITEM_INFO* laraItem)
{
	tRexItem->TargetState = 8;

	if (laraItem->RoomNumber != tRexItem->RoomNumber)
		ItemNewRoom(Lara.ItemNumber, tRexItem->RoomNumber);

	laraItem->Position.xPos = tRexItem->Position.xPos;
	laraItem->Position.yPos = tRexItem->Position.yPos;
	laraItem->Position.zPos = tRexItem->Position.zPos;
	laraItem->Position.xRot = 0;
	laraItem->Position.yRot = tRexItem->Position.yRot;
	laraItem->Position.zRot = 0;
	laraItem->Airborne = false;

	laraItem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + 1;
	laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
	laraItem->ActiveState = LS_DEATH;
	laraItem->TargetState = LS_DEATH;

	laraItem->HitPoints = -16384;
	Lara.Air = -1;
	Lara.Control.HandStatus = HandStatus::Busy;
	Lara.Control.Weapon.GunType = LaraWeaponType::None;

	Camera.flags = 1;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void TRexControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

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
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		if (item->TouchBits)
			LaraItem->HitPoints -= (item->ActiveState == 3) ? 10 : 1;

		info->Flags = (info->Mood != MoodType::Escape && !aiInfo.ahead && aiInfo.enemyFacing > -FRONT_ARC && aiInfo.enemyFacing < FRONT_ARC);

		if (aiInfo.distance > pow(1500, 2) &&
			aiInfo.distance < pow(SECTOR(4), 2)
			&& aiInfo.bite && !info->Flags)
		{
			info->Flags = 1;
		}

		switch (item->ActiveState)
		{
		case 1:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (aiInfo.distance < pow(1500, 2) && aiInfo.bite)
				item->TargetState = 7;
			else if (info->Mood == MoodType::Bored || info->Flags)
				item->TargetState = 2;
			else
				item->TargetState = 3;
			break;

		case 2:
			info->MaxTurn = ANGLE(2.0f);

			if (info->Mood != MoodType::Bored || !info->Flags)
				item->TargetState = 1;
			else if (aiInfo.ahead && GetRandomControl() < 0x200)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
			}

			break;

		case 3:
			info->MaxTurn = ANGLE(4.0f);

			if (aiInfo.distance < pow(SECTOR(5), 2) && aiInfo.bite)
				item->TargetState = 1;
			else if (info->Flags)
				item->TargetState = 1;
			else if (info->Mood != MoodType::Escape && aiInfo.ahead && GetRandomControl() < 0x200)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
			}
			else if (info->Mood == MoodType::Bored)
				item->TargetState = 1;

			break;

		case 7:
			if (item->TouchBits & 0x3000)
			{
				item->TargetState = 8;

				LaraItem->HitPoints -= 1500;
				LaraItem->HitStatus = true;
				LaraTRexDeath(item, LaraItem);
			}

			item->RequiredState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, (short)(head * 2));
	info->JointRotation[1] = info->JointRotation[0];

	CreatureAnimation(itemNumber, angle, 0);

	item->Collidable = true;
}
