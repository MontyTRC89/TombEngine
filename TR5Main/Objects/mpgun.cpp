#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"
#include "..\Game\Lara.h"

enum MPGUN_STATES {
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

void __cdecl MPGunControl(__int16 itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	__int16 torsoY = 0;
	__int16 torsoX = 0;
	__int16 head = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

	if (item->firedWeapon)
	{
		PHD_VECTOR pos;

		pos.x = mpgunBite.x;
		pos.y = mpgunBite.y;
		pos.z = mpgunBite.z;

		GetJointAbsPosition(item, &pos, mpgunBite.meshNum);

		TriggerDynamicLight(pos.x, pos.y, pos.z, (item->firedWeapon << 1) + 4, 24, 16, 4);
	}

	if (Boxes[item->boxNumber].overlapIndex & BLOCKED)
	{
		DoLotsOfBlood(item->pos.xPos, item->pos.yPos - (GetRandomControl() & 255) - 32, item->pos.zPos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->roomNumber, 3);
		item->hitPoints -= 20;
	}

	AI_INFO info;
	AI_INFO laraInfo;
	ITEM_INFO* target;
	__int32 dx;
	__int32 dz;
	__int32 random;

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		if (item->currentAnimState != 13)
		{
			item->animNumber = Objects[ID_MP_WITH_GUN].animIndex + 14;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 13;
		}
		else if (!(GetRandomControl() & 3) && item->frameNumber == Anims[item->animNumber].frameBase + 1)  
		{
			CreatureAIInfo(item, &info);
			if (Targetable(item, &info))
			{
				if (info.angle > -ANGLE(45) && info.angle < ANGLE(45))
				{
					torsoY = info.angle;
					head = info.angle;
					ShotLara(item, &info, &mpgunBite, torsoY, 32);
					SoundEffect(SFX_TR3_OIL_SMG_FIRE_ID72, &item->pos, 3 << 13);
				}
			}

		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
		{
			creature->enemy = LaraItem;

			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;

			laraInfo.distance = SQUARE(dx) + SQUARE(dx);
			
			CREATURE_INFO* currentCreature = BaddieSlots;
			for (__int32 slot = 0; slot < NUM_SLOTS; slot++, currentCreature++)
			{
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				target = &Items[currentCreature->itemNum];
				if (target->objectNumber != ID_LARA /*&& target->objectNumber != ID_BOB*/)
					continue;

				dx = target->pos.xPos - item->pos.xPos;
				dz = target->pos.zPos - item->pos.zPos;
				__int32 distance = SQUARE(dx) + SQUARE(dz);
				if (distance < laraInfo.distance)
					creature->enemy = target;
			}
		}

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = ATAN(dz, dx) - item->pos.yRot; 
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, creature->enemy != LaraItem ? VIOLENT : TIMID);
		CreatureMood(item, &info, creature->enemy != LaraItem ? VIOLENT : TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		__int32 x = item->pos.xPos + (WALL_SIZE * SIN(item->pos.yRot + laraInfo.angle) >> W2V_SHIFT);
		__int32 y = item->pos.yPos;
		__int32 z = item->pos.zPos + (WALL_SIZE * COS(item->pos.yRot + laraInfo.angle) >> W2V_SHIFT);
		
		__int16 roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		__int32 height = GetFloorHeight(floor, x, y, z);

		bool cover = (item->pos.yPos > (height + 768) && item->pos.yPos < (height + 1152) && laraInfo.distance > SQUARE(1024));

		ITEM_INFO* enemy = creature->enemy; 
		creature->enemy = LaraItem;
		if (laraInfo.distance < SQUARE(1024) || item->hitStatus || TargetVisible(item, &laraInfo)) 
		{
			if (!creature->alerted)
				SoundEffect(SFX_TR3_AMERCAN_HOY_ID300, &item->pos, 0);
			AlertAllGuards(itemNumber);
		}
		creature->enemy = enemy;

		switch (item->currentAnimState)
		{
		case MPGUN_WAIT:
			head = laraInfo.angle;
			creature->maximumTurn = 0;

			if (item->animNumber == Objects[item->objectNumber].animIndex + 17 ||
				item->animNumber == Objects[item->objectNumber].animIndex + 27 ||
				item->animNumber == Objects[item->objectNumber].animIndex + 28)
			{
				if (abs(info.angle) < ANGLE(10))
					item->pos.yRot += info.angle;
				else if (info.angle < 0)
					item->pos.yRot -= ANGLE(10);
				else
					item->pos.yRot += ANGLE(10);
			}

			if (item->aiBits & GUARD)
			{
				head = AIGuard(creature);
				item->goalAnimState = MPGUN_WAIT;
				break;
			}

			else if (item->aiBits & PATROL1)
			{
				item->goalAnimState = MPGUN_WALK;
				head = 0;
			}

			else if (cover && (Lara.target == item || item->hitStatus))
				item->goalAnimState = MPGUN_DUCK;
			else if (item->requiredAnimState == MPGUN_DUCK)
				item->goalAnimState = MPGUN_DUCK;
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = MPGUN_RUN;
			else if (Targetable(item, &info))
			{
				random = GetRandomControl();
				if (random < 0x2000)
					item->goalAnimState = MPGUN_SHOOT1;
				else if (random < 0x4000)
					item->goalAnimState = MPGUN_SHOOT2;
				else
					item->goalAnimState = MPGUN_AIM3;
			}
			else if (creature->mood == BORED_MOOD || ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048))))
			{
				if (info.ahead)
					item->goalAnimState = MPGUN_WAIT;
				else
					item->goalAnimState = MPGUN_WALK;
			}
			else
				item->goalAnimState = MPGUN_RUN;
			break;

		case MPGUN_WALK:
			head = laraInfo.angle;	
			creature->maximumTurn = ANGLE(6);
			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = MPGUN_WALK;
				head = 0;
			}
			else if (cover && (Lara.target == item || item->hitStatus))
			{
				item->requiredAnimState = MPGUN_DUCK;
				item->goalAnimState = MPGUN_WAIT;
			}
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = MPGUN_RUN;
			else if (Targetable(item, &info))
			{
				if (info.distance > SQUARE(1536) && info.zoneNumber == info.enemyZone)
					item->goalAnimState = MPGUN_AIM4;
				else
					item->goalAnimState = MPGUN_WAIT;
			}
			else if (creature->mood == BORED_MOOD)
			{
				if (info.ahead)
					item->goalAnimState = MPGUN_WALK;
				else
					item->goalAnimState = MPGUN_WAIT;
			}
			else
				item->goalAnimState = MPGUN_RUN;
			break;

		case MPGUN_RUN:
			if (info.ahead)
				head = info.angle;

			creature->maximumTurn = ANGLE(10);
			tilt = angle / 2;
			if (item->aiBits & GUARD)
				item->goalAnimState = MPGUN_WAIT;
			else if (cover && (Lara.target == item || item->hitStatus))
			{
				item->requiredAnimState = MPGUN_DUCK;
				item->goalAnimState = MPGUN_WAIT;
			}
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (Targetable(item, &info) || ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048))))
				item->goalAnimState = MPGUN_WAIT;
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = MPGUN_WALK;
			break;

		case MPGUN_AIM1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}

			if ((item->animNumber == Objects[ID_MP_WITH_GUN].animIndex + 12) || (item->animNumber == Objects[ID_MP_WITH_GUN].animIndex + 1 && item->frameNumber == Anims[item->animNumber].frameBase + 10))  
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->requiredAnimState = MPGUN_WAIT;
			}
			else if (item->hitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->requiredAnimState = MPGUN_DUCK;
				item->goalAnimState = MPGUN_WAIT;
			}
			break;

		case MPGUN_SHOOT1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->requiredAnimState == MPGUN_WAIT)
				item->goalAnimState = MPGUN_WAIT;
			break;

		case MPGUN_SHOOT2:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->goalAnimState = MPGUN_WAIT;
			}
			else if (item->hitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->requiredAnimState = MPGUN_DUCK;
				item->goalAnimState = MPGUN_WAIT;
			}
			break;

		case MPGUN_SHOOT3A:
		case MPGUN_SHOOT3B:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->frameNumber == Anims[item->animNumber].frameBase || (item->frameNumber == Anims[item->animNumber].frameBase + 11))
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->goalAnimState = MPGUN_WAIT;
			}
			else if (item->hitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->requiredAnimState = MPGUN_DUCK;
				item->goalAnimState = MPGUN_WAIT;
			}
			break;

		case MPGUN_AIM4:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if ((item->animNumber == Objects[ID_MP_WITH_GUN].animIndex + 18 && item->frameNumber == Anims[item->animNumber].frameBase + 17) || (item->animNumber == Objects[ID_MP_WITH_GUN].animIndex + 19 && item->frameNumber == Anims[item->animNumber].frameBase + 6))  
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->requiredAnimState = MPGUN_WALK;
			}
			else if (item->hitStatus && !(GetRandomControl() & 0x3) && cover)
			{
				item->requiredAnimState = MPGUN_DUCK;
				item->goalAnimState = MPGUN_WAIT;
			}
			if (info.distance < SQUARE(1536))
				item->requiredAnimState = MPGUN_WALK;
			break;

		case MPGUN_SHOOT4A:
		case MPGUN_SHOOT4B:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			if (item->requiredAnimState == MPGUN_WALK)
			{
				item->goalAnimState = MPGUN_WALK;
			}
			if (item->frameNumber == Anims[item->animNumber].frameBase + 16)
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32))
					item->goalAnimState = MPGUN_WALK;
			}
			if (info.distance < SQUARE(1536))
				item->goalAnimState = MPGUN_WALK;
			break;

		case MPGUN_DUCKED:
			if (info.ahead)
				head = info.angle;

			creature->maximumTurn = 0;

			if (Targetable(item, &info))
				item->goalAnimState = MPGUN_DUCKAIM;
			else if (item->hitStatus || !cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goalAnimState = MPGUN_STAND;
			else
				item->goalAnimState = MPGUN_DUCKWALK;
			break;

		case MPGUN_DUCKAIM:
			creature->maximumTurn = ONE_DEGREE;

			if (info.ahead)
				torsoY = info.angle;

			if (Targetable(item, &info))
				item->goalAnimState = MPGUN_DUCKSHOT;
			else
				item->goalAnimState = MPGUN_DUCKED;
			break;

		case MPGUN_DUCKSHOT:
			if (info.ahead)
				torsoY = info.angle;

			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				if (!ShotLara(item, &info, &mpgunBite, torsoY, 32) || !(GetRandomControl() & 0x7))
					item->goalAnimState = MPGUN_DUCKED;
			}

			break;

		case MPGUN_DUCKWALK:
			if (info.ahead)
				head = info.angle;

			creature->maximumTurn = ANGLE(6);

			if (Targetable(item, &info) || item->hitStatus || !cover || (info.ahead && !(GetRandomControl() & 0x1F)))
				item->goalAnimState = MPGUN_DUCKED;
			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	CreatureAnimation(itemNumber, angle, tilt);
}