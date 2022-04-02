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
	tRexItem->Animation.TargetState = 8;

	if (laraItem->RoomNumber != tRexItem->RoomNumber)
		ItemNewRoom(Lara.ItemNumber, tRexItem->RoomNumber);

	laraItem->Pose.Position.x = tRexItem->Pose.Position.x;
	laraItem->Pose.Position.y = tRexItem->Pose.Position.y;
	laraItem->Pose.Position.z = tRexItem->Pose.Position.z;
	laraItem->Pose.Orientation.x = 0;
	laraItem->Pose.Orientation.y = tRexItem->Pose.Orientation.y;
	laraItem->Pose.Orientation.z = 0;
	laraItem->Animation.Airborne = false;

	laraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + 1;
	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].FrameBase;
	laraItem->Animation.ActiveState = LS_DEATH;
	laraItem->Animation.TargetState = LS_DEATH;

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
		if (item->Animation.ActiveState == 1)
			item->Animation.TargetState = 5;
		else
			item->Animation.TargetState = 1;
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		if (item->TouchBits)
			LaraItem->HitPoints -= (item->Animation.ActiveState == 3) ? 10 : 1;

		info->Flags = (info->Mood != MoodType::Escape && !AI.ahead && AI.enemyFacing > -FRONT_ARC && AI.enemyFacing < FRONT_ARC);

		if (AI.distance > pow(1500, 2) &&
			AI.distance < pow(SECTOR(4), 2) &&
			AI.bite && !info->Flags)
		{
			info->Flags = 1;
		}

		switch (item->Animation.ActiveState)
		{
		case 1:
			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (AI.distance < pow(1500, 2) && AI.bite)
				item->Animation.TargetState = 7;
			else if (info->Mood == MoodType::Bored || info->Flags)
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;
			break;

		case 2:
			info->MaxTurn = ANGLE(2.0f);

			if (info->Mood != MoodType::Bored || !info->Flags)
				item->Animation.TargetState = 1;
			else if (AI.ahead && GetRandomControl() < 0x200)
			{
				item->Animation.RequiredState = 6;
				item->Animation.TargetState = 1;
			}

			break;

		case 3:
			info->MaxTurn = ANGLE(4.0f);

			if (AI.distance < pow(SECTOR(5), 2) && AI.bite)
				item->Animation.TargetState = 1;
			else if (info->Flags)
				item->Animation.TargetState = 1;
			else if (info->Mood != MoodType::Escape && AI.ahead && GetRandomControl() < 0x200)
			{
				item->Animation.RequiredState = 6;
				item->Animation.TargetState = 1;
			}
			else if (info->Mood == MoodType::Bored)
				item->Animation.TargetState = 1;

			break;

		case 7:
			if (item->TouchBits & 0x3000)
			{
				item->Animation.TargetState = 8;

				LaraItem->HitPoints -= 1500;
				LaraItem->HitStatus = true;
				LaraTRexDeath(item, LaraItem);
			}

			item->Animation.RequiredState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, (short)(head * 2));
	info->JointRotation[1] = info->JointRotation[0];

	CreatureAnimation(itemNumber, angle, 0);

	item->Collidable = true;
}
