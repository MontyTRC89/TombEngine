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
#include "..\Game\lara.h"

BITE_INFO flamerBite = { 0, 340, 64, 7 };

void __cdecl FlamerControl(__int16 itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	__int16 angle = 0;
	__int16 tilt = 0;
	__int16 torsoX = 0;
	__int16 torsoY = 0;
	__int16 head = 0;

	PHD_VECTOR pos;
	pos.x = flamerBite.x;
	pos.y = flamerBite.y;
	pos.z = flamerBite.z;
	GetJointAbsPosition(item, &pos, flamerBite.meshNum);

	__int32 random = GetRandomControl();
	if (item->currentAnimState != 6 && item->currentAnimState != 11)
	{
		TriggerDynamics(pos.x, pos.y, pos.z, (random & 3) + 6, 24 - ((random >> 4) & 3), 16 - ((random >> 6) & 3), random & 3); 	// Pilot light.
		TriggerPilotFlame(itemNumber);
	}
	else
		TriggerDynamics(pos.x, pos.y, pos.z, (random & 3) + 10, 31 - ((random >> 4) & 3), 24 - ((random >> 6) & 3), random & 7); 	// Full monty.

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 19;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else if (creature->hurtByLara)
			creature->enemy = LaraItem;
		else
		{
			// Find another enemy different from Lara

			creature->enemy = NULL;

			__int32 minDistance = 0x7fffffff;
			CREATURE_INFO* currentCreature = BaddieSlots;
			ITEM_INFO* target = NULL;

			for (__int32 i = 0; i < NUM_SLOTS; i++, currentCreature++)
			{
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				target = &Items[currentCreature->itemNum];
				if (target->objectNumber == ID_LARA /*|| target->objectNumber == WHITE_SOLDIER || target->objectNumber == FLAMETHROWER_BLOKE*/ || target->hitPoints <= 0)
					continue;

				/* Is this target closest? */
				__int32 x = target->pos.xPos - item->pos.xPos;
				__int32 z = target->pos.zPos - item->pos.zPos;
				__int32 distance = SQUARE(x) + SQUARE(z);
				if (distance < minDistance)
				{
					creature->enemy = target;
					minDistance = distance;
				}
			}
		}

		AI_INFO info;
		AI_INFO laraInfo;

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
			if (!creature->hurtByLara /*&& CurrentLevel != LV_CHAMBER*/)
				creature->enemy = NULL;
		}
		else
		{
			__int32 dx = LaraItem->pos.xPos - item->pos.xPos;
			__int32 dz = LaraItem->pos.zPos - item->pos.zPos;
			
			laraInfo.angle = ATAN(dz, dz) - item->pos.yRot; //only need to fill out the bits of laraInfo that will be needed by TargetVisible
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
			
			info.xAngle -= 0x800;
		}

		//		if (creature->enemy != LaraItem) //Should eventually change this to be any person
		//			meta_mood = VIOLENT;	//Shooters run up to their target if it's not a person
		//		else
		//meta_mood = TIMID;		//Else they stay their distance

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		ITEM_INFO* realEnemy = creature->enemy; //TargetVisible uses enemy, so need to fill this in as lara if we're doing other things

		if (item->hitStatus || laraInfo.distance < SQUARE(1024) || TargetVisible(item, &laraInfo)) //Only change flamer's AI bits if hurt (by Lara or Sealmute)
		{
			if (!creature->alerted)
				SoundEffect(300, &item->pos, 0);
			AlertAllGuards(itemNumber);
		}
		
		switch (item->currentAnimState)
		{
		case 1:
			head = laraInfo.angle;

			creature->flags = 0;
			creature->maximumTurn = 0;

			if (item->aiBits & GUARD)
			{
				head = AIGuard(creature);
				if (!(GetRandomControl() & 0xFF))
					item->goalAnimState = 4;
				break;
			}
			else if (item->aiBits & PATROL1)
				item->goalAnimState = 2;
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 2;
			else if (Targetable(item, &info) && (realEnemy != LaraItem || creature->hurtByLara))
			{
				if (info.distance < SQUARE(4096))
					item->goalAnimState = 10;
				else
					item->goalAnimState = 2;
			}
			else if (creature->mood == BORED_MOOD && info.ahead && !(GetRandomControl() & 0xFF))
				item->goalAnimState = 4;
			else if (creature->mood == ATTACK_MOOD || !(GetRandomControl() & 0xFF))
				item->goalAnimState = 2;
			
			break;

		case 4:
			head = laraInfo.angle;

			if (item->aiBits & GUARD)
			{
				head = AIGuard(creature);

				if (!(GetRandomControl() & 0xFF))
					item->goalAnimState = 1;
				break;
			}
			else if ((Targetable(item, &info) && 
				info.distance < SQUARE(4096) && 
				(realEnemy != LaraItem || creature->hurtByLara) || 
				creature->mood != BORED_MOOD || 
				!(GetRandomControl() & 0xFF)))
				item->goalAnimState = 1;

			break;

		case 2:
			head = laraInfo.angle;

			creature->flags = 0;
			creature->maximumTurn = ANGLE(5);

			if (item->aiBits & GUARD)	//Complete frig so I don't have to write InitialiseFlamer - only ever happens immediately on being triggered
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 12;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 1;
				item->goalAnimState = 1;
			}
			else if (item->aiBits & PATROL1)
				item->goalAnimState = 2;
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 2;
			else if (Targetable(item, &info) && 
				(realEnemy != LaraItem || creature->hurtByLara))
			{
				if (info.distance < SQUARE(4096))
					item->goalAnimState = 1;
				else
					item->goalAnimState = 9;
			}
			else if (creature->mood == BORED_MOOD && info.ahead)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 2;

			break;

		case 10:
			creature->flags = 0;

			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;

				if (Targetable(item, &info) && 
					info.distance < SQUARE(4096) && 
					(realEnemy != LaraItem || creature->hurtByLara))
					item->goalAnimState = 11;
				else
					item->goalAnimState = 1;
			}
			break;

		case 9:
			creature->flags = 0;

			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;

				if (Targetable(item, &info) && 
					info.distance < SQUARE(4096) && 
					(realEnemy != LaraItem || creature->hurtByLara))
					item->goalAnimState = 6;
				else
					item->goalAnimState = 2;
			}

			break;

		case 11:

			if (creature->flags < 40)
				creature->flags += (creature->flags >> 2) + 1;	// Length of flame.

			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
				if (Targetable(item, &info) && 
					info.distance < SQUARE(4096) && 
					(realEnemy != LaraItem || creature->hurtByLara))
					item->goalAnimState = 11;
				else
					item->goalAnimState = 1;
			}
			else
				item->goalAnimState = 1;

			if (creature->flags < 40)
				TriggerFlameThrower(item, &flamerBite, creature->flags);
			else
			{
				TriggerFlameThrower(item, &flamerBite, (GetRandomControl() & 31) + 12);
				if (realEnemy)
				{
					/*if (realEnemy->objectNumber == BURNT_MUTANT)
						realEnemy->item_flags[0]++;*/
				}
			}

			SoundEffect(204, &item->pos, 0);

			break;


		case 6:

			if (creature->flags < 40)
				creature->flags += (creature->flags >> 2) + 1;	// Length of flame.

			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
				if (Targetable(item, &info) && 
					info.distance < SQUARE(4096) && 
					(realEnemy != LaraItem || creature->hurtByLara))
					item->goalAnimState = 6;
				else
					item->goalAnimState = 2;
			}
			else
				item->goalAnimState = 2;

			if (creature->flags < 40)
				TriggerFlameThrower(item, &flamerBite, creature->flags);
			else
			{
				TriggerFlameThrower(item, &flamerBite, (GetRandomControl() & 31) + 12);
				if (realEnemy)
				{
					/*if (realEnemy->objectNumber == BURNT_MUTANT)
						realEnemy->item_flags[0]++;*/
				}
			}

			SoundEffect(204, &item->pos, 0);

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	CreatureAnimation(itemNumber, angle, 0);
}

__int16 __cdecl TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, __int16 speed)
{
	__int32			xv, yv, zv, vel, speed, lp;

	PHD_VECTOR pos1;
	PHD_VECTOR pos2;
	__int16 angles[2];
	__int32 speed;
	__int32 velocity;
	__int32 xv;
	__int32 yv;
	__int32 zv;

	__int16 effectNumber = CreateNewEffect(item->roomNumber);
	if (effectNumber != NO_ITEM)
	{
		FX_INFO* fx = &Effects[effectNumber];

		pos1.x = bite->x;
		pos1.y = bite->y;
		pos1.z = bite->z;
		GetJointAbsPosition(item, &pos1, bite->meshNum);

		pos2.x = bite->x;
		pos2.y = bite->y / 2;
		pos2.z = bite->z;
		GetJointAbsPosition(item, &pos2, bite->meshNum);

		phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

		fx->pos.xPos = pos1.x;
		fx->pos.yPos = pos1.y;
		fx->pos.zPos = pos1.z;

		fx->roomNumber = item->roomNumber;

		fx->pos.xRot = angles[1];
		fx->pos.zRot = 0;
		fx->pos.yRot = angles[0];
		fx->speed = speed << 2;
		//fx->objectNumber = DRAGON_FIRE;
		fx->counter = 20;
		fx->flag1 = 0;	// Set to orange flame.

		TriggerFlamethrowerFlame(0, 0, 0, 0, 0, 0, effectNumber);

		for (__int32 i = 0; i < 2; i++)
		{
			speed = (GetRandomControl() % (speed << 2)) + 32;
			velocity = (speed * COS(fx->pos.xRot)) >> W2V_SHIFT;
			
			xv = (velocity * SIN(fx->pos.yRot)) >> W2V_SHIFT;
			yv = -((speed * SIN(fx->pos.xRot)) >> W2V_SHIFT);
			zv = (velocity * COS(fx->pos.yRot)) >> W2V_SHIFT;

			TriggerFlamethrowerFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, xv << 5, yv << 5, zv << 5, -1);
		}

		velocity = ((speed << 1) * COS(fx->pos.xRot)) >> W2V_SHIFT;
		zv = (velocity * COS(fx->pos.yRot)) >> W2V_SHIFT;
		xv = (velocity * SIN(fx->pos.yRot)) >> W2V_SHIFT;
		yv = -(((speed << 1) * SIN(fx->pos.xRot)) >> W2V_SHIFT);
		
		TriggerFlamethrowerFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, xv << 5, yv << 5, zv << 5, -2);
	}

	return effectNumber;
}

void __cdecl TriggerFlamethrowerFlame(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv, __int32 fxnum)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = true;
	spark->sR = 48 + (GetRandomControl() & 31);
	spark->sG = spark->sR;
	spark->sB = 192 + (GetRandomControl() & 63);

	spark->dR = 192 + (GetRandomControl() & 63);
	spark->dG = 128 + (GetRandomControl() & 63);
	spark->dB = 32;

	if (xv || yv || zv)
	{
		spark->colFadeSpeed = 6;
		spark->fadeToBlack = 2;
		spark->sLife = spark->life = (GetRandomControl() & 1) + 12;
	}
	else
	{
		spark->colFadeSpeed = 8;
		spark->fadeToBlack = 16;
		spark->sLife = spark->life = (GetRandomControl() & 3) + 20;
	}

	spark->transType = 2;

	spark->extras = 0;
	spark->dynamic = -1;

	spark->x = x + ((GetRandomControl() & 31) - 16);
	spark->y = y;
	spark->z = z + ((GetRandomControl() & 31) - 16);

	spark->xVel = ((GetRandomControl() & 15) - 16) + xv;
	spark->yVel = yv;
	spark->zVel = ((GetRandomControl() & 15) - 16) + zv;
	spark->friction = 0;

	if (GetRandomControl() & 1)
	{
		if (fxnum >= 0)
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		else
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			spark->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			spark->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
	{
		if (fxnum >= 0)
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
		else
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;
	}

	spark->fxObj = fxnum;
	spark->gravity = 0;
	spark->maxYvel = 0;
	//spark->def = Objects[EXPLOSION1].mesh_index;
	__int32 size = (GetRandomControl() & 31) + 64;
	
	if (xv || yv || zv)
	{
		spark->size = size >> 5;
		if (fxnum == -2)
			spark->scalar = 2;
		else
			spark->scalar = 3;
	}
	else
	{
		spark->size = size >> 4;
		spark->scalar = 4;
	}

	spark->dSize = size >> 1;
}

void TriggerPilotFlame(__int32 itemnum)
{
	__int32 dx = LaraItem->pos.xPos - Items[itemnum].pos.xPos;
	__int32 dz = LaraItem->pos.zPos - Items[itemnum].pos.zPos;

	if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
		return;

	SPARKS* spark = &spark[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 48 + (GetRandomControl() & 31);
	spark->sG = spark->sR;
	spark->sB = 192 + (GetRandomControl() & 63);

	spark->dR = 192 + (GetRandomControl() & 63);
	spark->dG = 128 + (GetRandomControl() & 63);
	spark->dB = 32;

	spark->colFadeSpeed = 12 + (GetRandomControl() & 3);
	spark->fadeToBlack = 4;
	spark->sLife = spark->life = (GetRandomControl() & 3) + 20;
	spark->transType = 2;
	spark->extras = 0;
	spark->dynamic = -1;
	spark->x = (GetRandomControl() & 31) - 16;
	spark->y = (GetRandomControl() & 31) - 16;
	spark->z = (GetRandomControl() & 31) - 16;

	spark->xVel = (GetRandomControl() & 31) - 16;
	spark->yVel = -(GetRandomControl() & 3);
	spark->zVel = (GetRandomControl() & 31) - 16;

	spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;
	spark->fxObj = itemnum;
	spark->nodeNumber = 0;
	spark->friction = 4;
	spark->gravity = -(GetRandomControl() & 3) - 2;
	spark->maxYvel = -(GetRandomControl() & 3) - 4;
	//spark->def = objects[EXPLOSION1].mesh_index;
	spark->scalar = 0;
	__int32 size = (GetRandomControl() & 7) + 32;
	spark->size = size >> 1;
	spark->dSize = size;
}