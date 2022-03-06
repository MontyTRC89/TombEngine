#include "framework.h"
#include "Objects/TR3/Entity/tr3_flamethrower.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO FlamethrowerBite = { 0, 340, 64, 7 };

// TODO
enum FlamethrowerState
{

};

// TODO
enum FlamethrowerAnim
{

};

static void TriggerPilotFlame(int itemNumber)
{
	int dx = LaraItem->Position.xPos - g_Level.Items[itemNumber].Position.xPos;
	int dz = LaraItem->Position.zPos - g_Level.Items[itemNumber].Position.zPos;

	if (dx < -SECTOR(16) || dx > SECTOR(16) || dz < -SECTOR(16) || dz > SECTOR(16))
		return;

	auto* spark = &Sparks[GetFreeSpark()];

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
	spark->transType = TransTypeEnum::COLADD;
	spark->extras = 0;
	spark->dynamic = -1;
	spark->x = (GetRandomControl() & 31) - 16;
	spark->y = (GetRandomControl() & 31) - 16;
	spark->z = (GetRandomControl() & 31) - 16;

	spark->xVel = (GetRandomControl() & 31) - 16;
	spark->yVel = -(GetRandomControl() & 3);
	spark->zVel = (GetRandomControl() & 31) - 16;

	spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
	spark->fxObj = itemNumber;
	spark->nodeNumber = 0;
	spark->friction = 4;
	spark->gravity = -(GetRandomControl() & 3) - 2;
	spark->maxYvel = -(GetRandomControl() & 3) - 4;
	//spark->def = Objects[EXPLOSION1].mesh_index;
	spark->scalar = 0;
	int size = (GetRandomControl() & 7) + 32;
	spark->size = size / 2;
	spark->dSize = size;
}

static void TriggerFlamethrowerFlame(int x, int y, int z, int xv, int yv, int zv, int fxNumber)
{
	auto* spark = &Sparks[GetFreeSpark()];

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

	spark->transType = TransTypeEnum::COLADD;

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
		if (fxNumber >= 0)
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
		if (fxNumber >= 0)
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
		else
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;
	}

	spark->fxObj = fxNumber;
	spark->gravity = 0;
	spark->maxYvel = 0;

	int size = (GetRandomControl() & 31) + 64;

	if (xv || yv || zv)
	{
		spark->size = size / 32;
		if (fxNumber == -2)
			spark->scalar = 2;
		else
			spark->scalar = 3;
	}
	else
	{
		spark->size = size / 16;
		spark->scalar = 4;
	}

	spark->dSize = size / 2;
}

static short TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, short speed)
{
	short effectNumber = CreateNewEffect(item->RoomNumber);
	if (effectNumber != NO_ITEM)
	{
		auto* fx = &EffectList[effectNumber];

		PHD_VECTOR pos1 = { bite->x, bite->y, bite->z };
		GetJointAbsPosition(item, &pos1, bite->meshNum);

		PHD_VECTOR pos2 = { bite->x, bite->y / 2, bite->z };
		GetJointAbsPosition(item, &pos2, bite->meshNum);

		short angles[2];
		phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

		fx->pos.xPos = pos1.x;
		fx->pos.yPos = pos1.y;
		fx->pos.zPos = pos1.z;

		fx->roomNumber = item->RoomNumber;

		fx->pos.xRot = angles[1];
		fx->pos.zRot = 0;
		fx->pos.yRot = angles[0];
		fx->speed = speed * 4;
		fx->counter = 20;
		fx->flag1 = 0;

		TriggerFlamethrowerFlame(0, 0, 0, 0, 0, 0, effectNumber);

		int velocity;
		int xv, yv, zv;
		
		for (int i = 0; i < 2; i++)
		{
			speed = (GetRandomControl() % (speed * 4)) + 32;
			velocity = speed * phd_cos(fx->pos.xRot);

			xv = velocity * phd_sin(fx->pos.yRot);
			yv = -speed * phd_sin(fx->pos.xRot);
			zv = velocity * phd_cos(fx->pos.yRot);

			TriggerFlamethrowerFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, xv * 32, yv * 32, zv * 32, -1);
		}

		velocity = (speed * 2) * phd_cos(fx->pos.xRot);
		zv = velocity * phd_cos(fx->pos.yRot);
		xv = velocity * phd_sin(fx->pos.yRot);
		yv = -(speed * 2) * phd_sin(fx->pos.xRot);

		TriggerFlamethrowerFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, xv * 32, yv * 32, zv * 32, -2);
	}

	return effectNumber;
}

void FlameThrowerControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short torsoX = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;
	short head = 0;

	PHD_VECTOR pos = { FlamethrowerBite.x, FlamethrowerBite.y, FlamethrowerBite.z };
	GetJointAbsPosition(item, &pos, FlamethrowerBite.meshNum);

	int random = GetRandomControl();
	if (item->ActiveState != 6 && item->ActiveState != 11)
	{
		TriggerDynamicLight(pos.x, pos.y, pos.z, (random & 3) + 6, 24 - ((random / 16) & 3), 16 - ((random / 64) & 3), random & 3); 
		TriggerPilotFlame(itemNumber);
	}
	else
		TriggerDynamicLight(pos.x, pos.y, pos.z, (random & 3) + 10, 31 - ((random / 16) & 3), 24 - ((random / 64) & 3), random & 7);  

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 7)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 7;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->hurtByLara)
			creature->enemy = LaraItem;
		else
		{
			creature->enemy = NULL;

			ITEM_INFO* target = NULL;
			int minDistance = INT_MAX;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreature = ActiveCreatures[i];
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				target = &g_Level.Items[currentCreature->itemNum];
				if (target->ObjectNumber == ID_LARA || target->HitPoints <= 0)
					continue;

				int x = target->Position.xPos - item->Position.xPos;
				int z = target->Position.zPos - item->Position.zPos;

				int distance = pow(x, 2) + pow(z, 2);
				if (distance < minDistance)
				{
					creature->enemy = target;
					minDistance = distance;
				}
			}
		}

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		AI_INFO laraAI;
		if (creature->enemy == LaraItem)
		{
			laraAI.angle = AI.angle;
			laraAI.distance = AI.distance;

			if (!creature->hurtByLara)
				creature->enemy = NULL;
		}
		else
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;
			
			laraAI.angle = phd_atan(dz, dz) - item->Position.yRot; 
			laraAI.distance = pow(dx, 2) + pow(dz, 2);
			
			AI.xAngle -= 0x800;
		}

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		auto* realEnemy = creature->enemy; 

		if (item->HitStatus || laraAI.distance < pow(SECTOR(1), 2) || TargetVisible(item, &laraAI))
		{
			if (!creature->alerted)
				SoundEffect(300, &item->Position, 0);

			AlertAllGuards(itemNumber);
		}
		
		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;
			head = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);

				if (!(GetRandomControl() & 0xFF))
					item->TargetState = 4;

				break;
			}
			else if (item->AIBits & PATROL1)
				item->TargetState = 2;
			else if (creature->mood == MoodType::Escape)
				item->TargetState = 2;
			else if (Targetable(item, &AI) && (realEnemy != LaraItem || creature->hurtByLara))
			{
				if (AI.distance < pow(SECTOR(4), 2))
					item->TargetState = 10;
				else
					item->TargetState = 2;
			}
			else if (creature->mood == MoodType::Bored && AI.ahead && !(GetRandomControl() & 0xFF))
				item->TargetState = 4;
			else if (creature->mood == MoodType::Attack || !(GetRandomControl() & 0xFF))
				item->TargetState = 2;
			
			break;

		case 4:
			head = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);

				if (!(GetRandomControl() & 0xFF))
					item->TargetState = 1;

				break;
			}
			else if ((Targetable(item, &AI) &&
				AI.distance < pow(SECTOR(4), 2) &&
				(realEnemy != LaraItem || creature->hurtByLara) ||
				creature->mood != MoodType::Bored ||
				!(GetRandomControl() & 0xFF)))
			{
				item->TargetState = 1;
			}

			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(5.0f);
			head = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = 1;
				item->TargetState = 1;
			}
			else if (item->AIBits & PATROL1)
				item->TargetState = 2;
			else if (creature->mood == MoodType::Escape)
				item->TargetState = 2;
			else if (Targetable(item, &AI) && 
				(realEnemy != LaraItem || creature->hurtByLara))
			{
				if (AI.distance < pow(SECTOR(4), 2))
					item->TargetState = 1;
				else
					item->TargetState = 9;
			}
			else if (creature->mood == MoodType::Bored && AI.ahead)
				item->TargetState = 1;
			else
				item->TargetState = 2;

			break;

		case 10:
			creature->flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->hurtByLara))
				{
					item->TargetState = 11;
				}
				else
					item->TargetState = 1;
			}

			break;

		case 9:
			creature->flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->hurtByLara))
				{
					item->TargetState = 6;
				}
				else
					item->TargetState = 2;
			}

			break;

		case 11:
			if (creature->flags < 40)
				creature->flags += (creature->flags / 4) + 1;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->hurtByLara))
				{
					item->TargetState = 11;
				}
				else
					item->TargetState = 1;
			}
			else
				item->TargetState = 1;

			if (creature->flags < 40)
				TriggerFlameThrower(item, &FlamethrowerBite, creature->flags);
			else
			{
				TriggerFlameThrower(item, &FlamethrowerBite, (GetRandomControl() & 31) + 12);
				if (realEnemy)
				{
					/*code*/
				}
			}

			SoundEffect(204, &item->Position, 0);
			break;
			
		case 6:
			if (creature->flags < 40)
				creature->flags += (creature->flags / 4) + 1;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->hurtByLara))
				{
					item->TargetState = 6;
				}
				else
					item->TargetState = 2;
			}
			else
				item->TargetState = 2;

			if (creature->flags < 40)
				TriggerFlameThrower(item, &FlamethrowerBite, creature->flags);
			else
			{
				TriggerFlameThrower(item, &FlamethrowerBite, (GetRandomControl() & 31) + 12);
				if (realEnemy)
				{
					/*code*/
				}
			}

			SoundEffect(204, &item->Position, 0);
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	CreatureAnimation(itemNumber, angle, 0);
}
