#include "framework.h"
#include "tr4_harpy.h"
#include "Game/people.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO harpyBite1 = { 0, 0, 0, 4 };
BITE_INFO harpyBite2 = { 0, 0, 0, 2 };
BITE_INFO harpyBite3 = { 0, 0, 0, 21 };
BITE_INFO harpyAttack1 = { 0, 128, 0, 2 };
BITE_INFO harpyAttack2 = { 0, 128, 0, 4 };

#define STATE_HARPY_STOP				1
#define STATE_HARPY_ATTACK				5
#define STATE_HARPY_POISON_ATTACK		6
#define STATE_HARPY_FLAME_ATTACK		8
#define STATE_HARPY_FALLING				10
#define STATE_HARPY_DEATH				11

static void TriggerHarpyMissile(PHD_3DPOS* pos, short roomNumber, int count)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != -1)
	{
		FX_INFO* fx = &EffectList[fxNumber];

		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		fx->pos.yRot = pos->yRot;
		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 2 * GetRandomControl() + -32768;
		fx->objectNumber = ID_ENERGY_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->flag1 = count;
		fx->frameNumber = Objects[fx->objectNumber].meshIndex + 2 * count;
	}
}

static void TriggerHarpyFlame(short itemNumber, byte num, int size)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	int dx = LaraItem->Position.xPos - item->Position.xPos;
	int dz = LaraItem->Position.zPos - item->Position.zPos;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dB = 0;
		spark->dG = spark->dR = (GetRandomControl() & 0x7F) + 32;
		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 20;
		spark->x = (GetRandomControl() & 0xF) - 8;
		spark->y = 0;
		spark->z = (GetRandomControl() & 0xF) - 8;
		spark->xVel = (byte)GetRandomControl() - 128;
		spark->yVel = 0;
		spark->zVel = (byte)GetRandomControl() - 128;
		spark->friction = 5;
		spark->flags = 4762;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
		{
			spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
		}
		else
		{
			spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
		}
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 0x1F) + 16;
		spark->fxObj = itemNumber;
		spark->nodeNumber = num;
		spark->scalar = 2;
		spark->sSize = spark->size = GetRandomControl() & 0xF + size;
		spark->dSize = spark->size / 8;
	}
}

static void TriggerHarpySparks(int x, int y, int z, int xv, int yv, int zv)
{
	int dx = LaraItem->Position.xPos - x;
	int dz = LaraItem->Position.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dR = spark->dG = (GetRandomControl() & 0x7F) + 64;
		spark->dB = 0;
		spark->life = 16;
		spark->sLife = 16;
		spark->colFadeSpeed = 4;
		spark->y = y;
		spark->transType = TransTypeEnum::COLADD;
		spark->fadeToBlack = 4;
		spark->x = x;
		spark->z = z;
		spark->xVel = xv;
		spark->yVel = yv;
		spark->zVel = zv;
		spark->friction = 34;
		spark->scalar = 1;
		spark->sSize = spark->size = (GetRandomControl() & 3) + 4;
		spark->maxYvel = 0;
		spark->gravity = 0;
		spark->dSize = (GetRandomControl() & 1) + 1;
		spark->flags = SP_NONE;
	}
}

static void DoHarpyEffects(ITEM_INFO* item, short itemNumber)
{
	item->ItemFlags[0]++;

	PHD_VECTOR pos1;

	pos1.x = harpyAttack1.x;
	pos1.y = harpyAttack1.y;
	pos1.z = harpyAttack1.z;

	GetJointAbsPosition(item, &pos1, harpyAttack1.meshNum);

	PHD_VECTOR pos2;

	pos2.x = harpyAttack2.x;
	pos2.y = harpyAttack2.y;
	pos2.z = harpyAttack2.z;

	GetJointAbsPosition(item, &pos2, harpyAttack2.meshNum);

	if (item->ItemFlags[0] >= 24 
		&& item->ItemFlags[0] <= 47 
		&& (GetRandomControl() & 0x1F) < item->ItemFlags[0])
	{
		for (int i = 0; i < 2; i++)
		{
			int dx = (GetRandomControl() & 0x7FF) + pos1.x - 1024;
			int dy = (GetRandomControl() & 0x7FF) + pos1.y - 1024;
			int dz = (GetRandomControl() & 0x7FF) + pos1.z - 1024;

			TriggerHarpySparks(dx, dy, dz, 8 * (pos1.x - dx), 8 * (pos1.y - dy), 8 * (pos1.z - dz));

			dx = (GetRandomControl() & 0x7FF) + pos2.x - 1024;
			dy = (GetRandomControl() & 0x7FF) + pos2.y - 1024;
			dz = (GetRandomControl() & 0x7FF) + pos2.z - 1024;

			TriggerHarpySparks(dx, dy, dz, 8 * (pos2.x - dx), 8 * (pos2.y - dy), 8 * (pos2.z - dz));
		}
	}

	int something = 2 * item->ItemFlags[0];
	if (something > 64)
	{
		something = 64;
	}
	if (something < 80)
	{
		if ((Wibble & 0xF) == 8)
		{
			TriggerHarpyFlame(itemNumber, 4, something);
		}
		else if (!(Wibble & 0xF))
		{
			TriggerHarpyFlame(itemNumber, 5, something);
		}
	}

	if (item->ItemFlags[0] >= 61)
	{
		if (item->ItemFlags[0] <= 65 && GlobalCounter & 1)
		{
			PHD_VECTOR pos3;

			pos3.x = harpyAttack1.x;
			pos3.y = harpyAttack1.y * 2;
			pos3.z = harpyAttack1.z;

			GetJointAbsPosition(item, &pos3, harpyAttack1.meshNum);

			PHD_3DPOS pos;

			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;

			short angles[2];
			phd_GetVectorAngles(pos3.x - pos1.x,
				pos3.y - pos1.y,
				pos3.z - pos1.z,
				angles);

			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			TriggerHarpyMissile(&pos, item->RoomNumber, 2);
		}
		if (item->ItemFlags[0] >= 61 && item->ItemFlags[0] <= 65 && !(GlobalCounter & 1))
		{
			PHD_VECTOR pos3;

			pos3.x = harpyAttack2.x;
			pos3.y = harpyAttack2.y * 2;
			pos3.z = harpyAttack2.z;

			GetJointAbsPosition(item, &pos3, harpyAttack2.meshNum);

			PHD_3DPOS pos;

			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;

			short angles[2];
			phd_GetVectorAngles(pos3.x - pos1.x,
				pos3.y - pos1.y,
				pos3.z - pos1.z,
				angles);

			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			TriggerHarpyMissile(&pos, item->RoomNumber, 2);
		}
	}
}

void InitialiseHarpy(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = STATE_HARPY_STOP;
	item->ActiveState = STATE_HARPY_STOP;
}

void HarpyControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!CreatureActive(itemNumber))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	if (item->HitPoints <= 0)
	{
		short state = item->ActiveState - 9;
		item->HitPoints = 0;

		if (state)
		{
			state--;
			if (state)
			{
				if (state == 1)
				{
					item->Position.xRot = 0;
					item->Position.yPos = item->Floor;
				}
				else
				{
					item->AnimNumber = obj->animIndex + 5;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					item->ActiveState = 9;
					item->Velocity = 0;
					item->Airborne = true;
					item->Position.xRot = 0;
				}

				CreatureTilt(item, 0);

				CreatureJoint(item, 0, joint0);
				CreatureJoint(item, 1, joint1);
				CreatureJoint(item, 2, joint2);

				CreatureAnimation(itemNumber, angle, 0);

				return;
			}
		}
		else
		{
			item->TargetState = STATE_HARPY_FALLING;
		}

		if (item->Position.yPos >= item->Floor)
		{
			item->Position.yPos = item->Floor;
			item->VerticalVelocity = 0;
			item->TargetState = STATE_HARPY_DEATH;
			item->Airborne = false;
		}

		item->Position.xRot = 0;
	}
	else
	{
		if (item->AIBits)
		{
			GetAITarget(creature);
		}

		int minDistance = 0x7FFFFFFF;

		creature->enemy = NULL;

		for(int i = 0; i < ActiveCreatures.size(); i++) {
			CREATURE_INFO* baddie = ActiveCreatures[i];
			if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNumber)
				continue;

			ITEM_INFO* target = &g_Level.Items[baddie->itemNum];

			if (target->ObjectNumber == ID_LARA_DOUBLE)
			{
				int dx = target->Position.xPos - item->Position.xPos;
				int dz = target->Position.zPos - item->Position.zPos;
				int distance = dx * dx + dz * dz;

				if (distance < minDistance)
				{
					creature->enemy = target;
					minDistance = distance;
				}
			}
		}

		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (creature->enemy != LaraItem)
		{
			phd_atan(LaraItem->Position.zPos - item->Position.zPos, LaraItem->Position.xPos - item->Position.xPos);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			joint1 = info.xAngle;
		}

		int height = 0;
		int dy = 0;

		switch (item->ActiveState)
		{
		case STATE_HARPY_STOP:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(7);

			if (creature->enemy)
			{
				height = item->Position.yPos + 2048;
				if (creature->enemy->Position.yPos > height&& item->Floor > height)
				{
					item->TargetState = 3;
					break;
				}
			}
			if (info.ahead)
			{
				dy = abs(creature->enemy->Position.yPos - item->Position.yPos);
				if (dy <= 1024)
				{
					if (info.distance < SQUARE(341))
					{
						item->TargetState = STATE_HARPY_POISON_ATTACK;
						break;
					}
					if (dy <= 1024 && info.distance < SQUARE(2048))
					{
						item->TargetState = 4;
						break;
					}
				}
			}

			if (creature->enemy != LaraItem 
				|| !Targetable(item, &info)
				|| info.distance <= SQUARE(3584)
				|| !(GetRandomControl() & 1))
			{
				item->TargetState = 2;
				break;
			}

			item->TargetState = STATE_HARPY_FLAME_ATTACK;
			item->ItemFlags[0] = 0;
			break;

		case 2:
			creature->maximumTurn = ANGLE(7);
			creature->flags = 0;

			if (item->RequiredState)
			{
				item->TargetState = item->RequiredState;
				if (item->RequiredState == 8)
				{
					item->ItemFlags[0] = 0;
				}
				break;
			}
			if (item->HitStatus)
			{
				item->TargetState = 7;
				break;
			}
			if (info.ahead)
			{
				if (info.distance >= SQUARE(341))
				{
					if (info.ahead 
						&& info.distance >= SQUARE(2048) 
						&& info.distance > SQUARE(3584) 
						&& GetRandomControl() & 1)
					{
						item->TargetState = STATE_HARPY_FLAME_ATTACK;
						item->ItemFlags[0] = 0;
					}
					else
					{
						item->TargetState = 4;
					}
				}
				else
				{
					item->TargetState = STATE_HARPY_POISON_ATTACK;
				}

				break;
			}
			if (GetRandomControl() & 1)
			{
				item->TargetState = 7;
				break;
			}
			if (!info.ahead)
			{
				item->TargetState = 4;
				break;
			}
			if (info.distance >= SQUARE(341))
			{
				if (info.ahead && info.distance >= SQUARE(2048) &&
					info.distance > SQUARE(3584) && GetRandomControl() & 1)
				{
					item->TargetState = STATE_HARPY_FLAME_ATTACK;
					item->ItemFlags[0] = 0;
				}
				else
				{
					item->TargetState = 4;
				}
			}
			else
			{
				item->TargetState = STATE_HARPY_POISON_ATTACK;
			}

			break;

		case 3:
			if (!creature->enemy 
				|| creature->enemy->Position.yPos < item->Position.yPos + 2048)
			{
				item->TargetState = STATE_HARPY_STOP;
			}

			break;

		case 4:
			creature->maximumTurn = ANGLE(2);

			if (info.ahead && info.distance < SQUARE(2048))
			{
				item->TargetState = STATE_HARPY_ATTACK;
			}
			else
			{
				item->TargetState = 13;
			}

			break;

		case STATE_HARPY_ATTACK:
			creature->maximumTurn = ANGLE(2);
			item->TargetState = 2;

			if (item->TouchBits & 0x14
				|| creature->enemy && creature->enemy != LaraItem &&
				abs(creature->enemy->Position.yPos - item->Position.yPos) <= 1024 &&
				info.distance < SQUARE(2048))
			{
				LaraItem->HitPoints -= 10;
				LaraItem->HitStatus = true;

				if (item->TouchBits & 0x10)
				{
					CreatureEffect2(
						item,
						&harpyBite1,
						5,
						-1,
						DoBloodSplat);
				}
				else
				{
					CreatureEffect2(
						item,
						&harpyBite2,
						5,
						-1,
						DoBloodSplat);
				}
			}

			break;

		case STATE_HARPY_POISON_ATTACK:
			creature->maximumTurn = ANGLE(2);

			if (creature->flags == 0
				&& (item->TouchBits & 0x300000
					|| creature->enemy && creature->enemy != LaraItem &&
					abs(creature->enemy->Position.yPos - item->Position.yPos) <= 1024 &&
					info.distance < SQUARE(2048)))
			{
				LaraItem->HitPoints -= 100;
				LaraItem->HitStatus = true;

				CreatureEffect2(
					item,
					&harpyBite3,
					10,
					-1,
					DoBloodSplat);

				if (creature->enemy == LaraItem)
				{
					Lara.poisoned += 2048;
				}

				creature->flags = 1;
			}

			break;

		case STATE_HARPY_FLAME_ATTACK:
			DoHarpyEffects(item, itemNumber);
			break;

		case 12:
			if (info.ahead && info.distance > SQUARE(3584))
			{
				item->TargetState = 2;
				item->RequiredState = STATE_HARPY_FLAME_ATTACK;
			}
			else if (GetRandomControl() & 1)
			{
				item->TargetState = STATE_HARPY_STOP;
			}

			break;

		case 13:
			item->TargetState = 2;
			break;

		default:
			break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}
