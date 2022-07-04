#include "framework.h"
#include "Objects/TR2/Entity/tr2_sword_guardian.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"

BITE_INFO SwordBite = { 0, 37, 550, 15 };

void InitialiseSwordGuardian(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
}

static void SwordGuardianFly(ItemInfo* item)
{
	Vector3Int pos;
	pos.x = (GetRandomControl() * 256 / 32768) + item->Pose.Position.x - 128;
	pos.y = (GetRandomControl() * 256 / 32768) + item->Pose.Position.y - 256;
	pos.z = (GetRandomControl() * 256 / 32768) + item->Pose.Position.z - 128;

	TriggerGunSmoke(pos.x, pos.y, pos.z, 1, 1, 1, 1, LaraWeaponType::GrenadeLauncher, 32);
	SoundEffect(SFX_TR2_WARRIOR_HOVER, &item->Pose);
}

void SwordGuardianControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;
	short torso = 0;

	bool laraAlive = LaraItem->HitPoints > 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 12)
		{
			SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->Pose);
			SoundEffect(SFX_TR4_EXPLOSION2, &LaraItem->Pose);
			ExplodingDeath(itemNumber, BODY_EXPLODE);
			DisableEntityAI(itemNumber);
			KillItem(itemNumber);
			item->Animation.ActiveState = 12;
		}

		return;
	}
	else
	{
		creature->LOT.Step = STEP_SIZE;
		creature->LOT.Drop = -STEP_SIZE;
		creature->LOT.Fly = NO_FLYING;
		creature->LOT.Zone = ZONE_BASIC;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (item->Animation.ActiveState == 8)
		{
			if (AI.zoneNumber != AI.enemyZone)
			{
				creature->LOT.Step = WALL_SIZE * 20;
				creature->LOT.Drop = -WALL_SIZE * 20;
				creature->LOT.Fly = STEP_SIZE / 4;
				creature->LOT.Zone = ZONE_FLYER;
				CreatureAIInfo(item, &AI);
			}
		}

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (item->Animation.ActiveState != 9)
			item->MeshBits = 0xFFFFFFFF;

		switch (item->Animation.ActiveState)
		{
		case 9:
			creature->MaxTurn = 0;

			if (!creature->Flags)
			{
				item->MeshBits = (item->MeshBits << 1) + 1;
				creature->Flags = 3;
			}
			else
				creature->Flags--;
			
			break;

		case 1:
			creature->MaxTurn = 0;

			if (AI.ahead)
				head = AI.angle;

			if (laraAlive)
			{
				if (AI.bite && AI.distance < pow(SECTOR(1), 2))
				{
					if (GetRandomControl() >= 0x4000)
						item->Animation.TargetState = 5;
					else
						item->Animation.TargetState = 3;
				}
				else
				{
					if (AI.zoneNumber == AI.enemyZone)
						item->Animation.TargetState = 2;
					else
						item->Animation.TargetState = 8;
				}
			}
			else
				item->Animation.TargetState = 7;
			
			break;

		case 2:
			creature->MaxTurn = ANGLE(9.0f);

			if (AI.ahead)
				head = AI.angle;

			if (laraAlive)
			{
				if (AI.bite && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 10;
				else if (AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = 1;
			}
			else
				item->Animation.TargetState = 1;
			
			break;

		case 3:
			creature->Flags = 0;

			if (AI.ahead)
				torso = AI.angle;

			if (!AI.bite || AI.distance > pow(SECTOR(1), 2))
				item->Animation.TargetState = 1;
			else
				item->Animation.TargetState = 4;
			break;

		case 5:
			creature->Flags = 0;

			if (AI.ahead)
				torso = AI.angle;

			if (!AI.bite || AI.distance > pow(SECTOR(1), 2))
				item->Animation.TargetState = 1;
			else
				item->Animation.TargetState = 6;

			break;

		case 10:
			creature->Flags = 0;

			if (AI.ahead)
				torso = AI.angle;

			if (!AI.bite || AI.distance > pow(SECTOR(2), 2))
				item->Animation.TargetState = 1;
			else
				item->Animation.TargetState = 11;

			break;

		case 8:
			creature->MaxTurn = ANGLE(7.0f);
			SwordGuardianFly(item);

			if (AI.ahead)
				head = AI.angle;

			if (!creature->LOT.Fly)
				item->Animation.TargetState = 1;

			break;

		case 4:
		case 6:
		case 11:
			if (AI.ahead)
				torso = AI.angle;

			if (!creature->Flags && (item->TouchBits & 0xC000))
			{
				CreatureEffect(item, &SwordBite, DoBloodSplat);
				DoDamage(creature->Enemy, 300);
				creature->Flags = 1;
			}

			break;
		}
	}

	if (item->HitPoints > 0)
	{
		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
