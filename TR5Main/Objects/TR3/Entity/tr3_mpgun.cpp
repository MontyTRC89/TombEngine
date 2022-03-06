#include "framework.h"
#include "Objects/TR3/Entity/tr3_mpgun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

enum MPGUN_STATES
{
	MPGUN_EMPTY, 
	MPGUN_WAIT,
	MPGUN_WALK, 
	MPGUN_RUN,
	MPGUN_AIM1,
	MPGUN_SHOOT1, 
	MPGUN_AIM2,
	MPGUN_SHOOT2, 
	MPGUN_SHOOT3A, 
	MPGUN_SHOOT3B, 
	MPGUN_SHOOT4A, 
	MPGUN_AIM3,
	MPGUN_AIM4, 
	MPGUN_DEATH, 
	MPGUN_SHOOT4B,
	MPGUN_DUCK,
	MPGUN_DUCKED,
	MPGUN_DUCKAIM, 
	MPGUN_DUCKSHOT, 
	MPGUN_DUCKWALK,
	MPGUN_STAND
};

BITE_INFO mpgunBite = { 0, 160, 40, 13 };

void MPGunControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	short torsoY = 0;
	short torsoX = 0;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (creature->FiredWeapon)
	{
		PHD_VECTOR pos;

		pos.x = mpgunBite.x;
		pos.y = mpgunBite.y;
		pos.z = mpgunBite.z;

		GetJointAbsPosition(item, &pos, mpgunBite.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, (creature->FiredWeapon * 2) + 4, 24, 16, 4);
		creature->FiredWeapon--;
	}

	if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
	{
		DoLotsOfBlood(item->Position.xPos, item->Position.yPos - (GetRandomControl() & 255) - 32, item->Position.zPos, (GetRandomControl() & 127) + 128, GetRandomControl() * 2, item->RoomNumber, 3);
		item->HitPoints -= 20;
	}

	AI_INFO info;
	AI_INFO laraInfo;
	ITEM_INFO* target;
	int dx;
	int dz;
	int random;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->ActiveState != 13)
		{
			item->AnimNumber = Objects[ID_MP_WITH_GUN].animIndex + 14;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 13;
		}
		else if (!(GetRandomControl() & 3) && item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 1)  
		{
			CreatureAIInfo(item, &info);

			if (Targetable(item, &info))
			{
				if (info.angle > -ANGLE(45) && info.angle < ANGLE(45))
				{
					torsoY = info.angle;
					head = info.angle;
					ShotLara(item, &info, &mpgunBite, torsoY, 32);
					SoundEffect(SFX_TR3_OIL_SMG_FIRE, &item->Position, 24576);
				}
			}

		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
		{
			creature->Enemy = LaraItem;

			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;

			laraInfo.distance = SQUARE(dx) + SQUARE(dx);
			
			for (int slot = 0; slot < ActiveCreatures.size(); slot++)
			{
				CreatureInfo* currentCreature = ActiveCreatures[slot];
				if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
					continue;

				target = &g_Level.Items[currentCreature->ItemNumber];
				if (target->ObjectNumber != ID_LARA)
					continue;

				dx = target->Position.xPos - item->Position.xPos;
				dz = target->Position.zPos - item->Position.zPos;
				int distance = SQUARE(dx) + SQUARE(dz);
				if (distance < laraInfo.distance)
					creature->Enemy = target;
			}
		}

		CreatureAIInfo(item, &info);

		if (creature->Enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot; 
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, creature->Enemy != LaraItem ? VIOLENT : TIMID);
		CreatureMood(item, &info, creature->Enemy != LaraItem ? VIOLENT : TIMID);

		angle = CreatureTurn(item, creature->MaxTurn);

		int x = item->Position.xPos + WALL_SIZE * phd_sin(item->Position.yRot + laraInfo.angle);
		int y = item->Position.yPos;
		int z = item->Position.zPos + WALL_SIZE * phd_cos(item->Position.yRot + laraInfo.angle);
		
		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		int height = GetFloorHeight(floor, x, y, z);

		bool cover = (item->Position.yPos > (height + 768) && item->Position.yPos < (height + 1152) && laraInfo.distance > SQUARE(1024));

		ITEM_INFO* enemy = creature->Enemy; 
		creature->Enemy = LaraItem;
		if (laraInfo.distance < SQUARE(1024) || item->HitStatus || TargetVisible(item, &laraInfo)) 
		{
			if (!creature->Alerted)
				SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Position, 0);
			AlertAllGuards(itemNumber);
		}
		creature->Enemy = enemy;

		switch (item->ActiveState)
		{
		case MPGUN_WAIT:
			head = laraInfo.angle;
			creature->MaxTurn = 0;

			if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + 17 ||
				item->AnimNumber == Objects[item->ObjectNumber].animIndex + 27 ||
				item->AnimNumber == Objects[item->ObjectNumber].animIndex + 28)
			{
				if (abs(info.angle) < ANGLE(10))
					item->Position.yRot += info.angle;
				else if (info.angle < 0)
					item->Position.yRot -= ANGLE(10);
				else
					item->Position.yRot += ANGLE(10);
			}

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);
				item->TargetState = MPGUN_WAIT;
				break;
			}

			else if (item->AIBits & PATROL1)
			{
				item->TargetState = MPGUN_WALK;
				head = 0;
			}

			else if (cover && (Lara.target == item || item->HitStatus))
				item->TargetState = MPGUN_DUCK;
			else if (item->RequiredState == MPGUN_DUCK)
				item->TargetState = MPGUN_DUCK;
			else if (creature->Mood == MoodType::Escape)
				item->TargetState = MPGUN_RUN;
			else if (Targetable(item, &info))
			{
				random = GetRandomControl();
				if (random < 0x2000)
					item->TargetState = MPGUN_SHOOT1;
				else if (random < 0x4000)
					item->TargetState = MPGUN_SHOOT2;
				else
					item->TargetState = MPGUN_AIM3;
			}
			else if (creature->Mood == MoodType::Bored || ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraInfo.distance > SQUARE(2048))))
			{
				if (info.ahead)
					item->TargetState = MPGUN_WAIT;
				else
					item->TargetState = MPGUN_WALK;
			}
			else
				item->TargetState = MPGUN_RUN;
			break;

		case MPGUN_WALK:
			head = laraInfo.angle;	
			creature->MaxTurn = ANGLE(6);
			if (item->AIBits & PATROL1)
			{
				item->TargetState = MPGUN_WALK;
				head = 0;
			}
			else if (cover && (Lara.target == item || item->HitStatus))
			{
				item->RequiredState = MPGUN_DUCK;
				item->TargetState = MPGUN_WAIT;
			}
			else if (creature->Mood == MoodType::Escape)
				item->TargetState = MPGUN_RUN;
			else if (Targetable(item, &info))
			{
				if (info.distance > SQUARE(1536) && info.zoneNumber == info.enemyZone)
					item->TargetState = MPGUN_AIM4;
				else
					item->TargetState = MPGUN_WAIT;
			}
			else if (creature->Mood == MoodType::Bored)
			{
				if (info.ahead)
					item->TargetState = MPGUN_WALK;
				else
					item->TargetState = MPGUN_WAIT;
			}
			else
				item->TargetState = MPGUN_RUN;
			break;

		case MPGUN_RUN:
			if (info.ahead)
				head = info.angle;

			creature->MaxTurn = ANGLE(10);
			tilt = angle / 2;
			if (item->AIBits & GUARD)
				item->TargetState = MPGUN_WAIT;
			else if (cover && (Lara.target == item || item->HitStatus))
			{
				item->RequiredState = MPGUN_DUCK;
				item->TargetState = MPGUN_WAIT;
			}
			else if (creature->Mood == MoodType::Escape)
				break;
			else if (Targetable(item, &info) || ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraInfo.distance > SQUARE(2048))))
				item->TargetState = MPGUN_WAIT;
			else if (creature->Mood == MoodType::Bored)
				item->TargetState = MPGUN_WALK;
			break;

		case MPGUN_AIM1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}

			if ((item->AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 12) || (item->AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 1 && item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 10))  
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->RequiredState = MPGUN_WAIT;
			}
			else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->RequiredState = MPGUN_DUCK;
				item->TargetState = MPGUN_WAIT;
			}
			break;

		case MPGUN_SHOOT1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->RequiredState == MPGUN_WAIT)
				item->TargetState = MPGUN_WAIT;
			break;

		case MPGUN_SHOOT2:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->TargetState = MPGUN_WAIT;
			}
			else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->RequiredState = MPGUN_DUCK;
				item->TargetState = MPGUN_WAIT;
			}
			break;

		case MPGUN_SHOOT3A:
		case MPGUN_SHOOT3B:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase || (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 11))
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->TargetState = MPGUN_WAIT;
			}
			else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->RequiredState = MPGUN_DUCK;
				item->TargetState = MPGUN_WAIT;
			}
			break;

		case MPGUN_AIM4:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if ((item->AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 18 && item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 17) || (item->AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 19 && item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 6))  
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->RequiredState = MPGUN_WALK;
			}
			else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->RequiredState = MPGUN_DUCK;
				item->TargetState = MPGUN_WAIT;
			}
			if (info.distance < SQUARE(1536))
				item->RequiredState = MPGUN_WALK;
			break;

		case MPGUN_SHOOT4A:
		case MPGUN_SHOOT4B:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->RequiredState == MPGUN_WALK)
			{
				item->TargetState = MPGUN_WALK;
			}
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 16)
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->TargetState = MPGUN_WALK;
			}
			if (info.distance < SQUARE(1536))
				item->TargetState = MPGUN_WALK;
			break;

		case MPGUN_DUCKED:
			if (info.ahead)
				head = info.angle;

			creature->MaxTurn = 0;

			if (Targetable(item, &info))
				item->TargetState = MPGUN_DUCKAIM;
			else if (item->HitStatus || !cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->TargetState = MPGUN_STAND;
			else
				item->TargetState = MPGUN_DUCKWALK;
			break;

		case MPGUN_DUCKAIM:
			creature->MaxTurn = ONE_DEGREE;

			if (info.ahead)
				torsoY = info.angle;

			if (Targetable(item, &info))
				item->TargetState = MPGUN_DUCKSHOT;
			else
				item->TargetState = MPGUN_DUCKED;
			break;

		case MPGUN_DUCKSHOT:
			if (info.ahead)
				torsoY = info.angle;

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32) || !(GetRandomControl() & 0x7))
					item->TargetState = MPGUN_DUCKED;
			}

			break;

		case MPGUN_DUCKWALK:
			if (info.ahead)
				head = info.angle;

			creature->MaxTurn = ANGLE(6);

			if (Targetable(item, &info) || item->HitStatus || !cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->TargetState = MPGUN_DUCKED;
			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);
	CreatureAnimation(itemNumber, angle, tilt);
}