#include "framework.h"
#include "tr5_imp.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/effects/effects.h"
#include "Game/control/box.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Objects/Generic/Object/burning_torch.h"

using namespace TEN::Entities::Generic;

#define STATE_IMP_WALK			0
#define STATE_IMP_STOP			1
#define STATE_IMP_RUN			2
#define STATE_IMP_ATTACK1		3
#define STATE_IMP_ATTACK2		5
#define STATE_IMP_SCARED		6
#define STATE_IMP_START_CLIMB	7
#define STATE_IMP_START_ROLL	8
#define STATE_IMP_DEATH			9
#define STATE_IMP_THROW_STONES	11

#define ANIMATION_IMP_DEATH		18

BITE_INFO ImpBite = { 0, 0x64, 0, 9 };

void InitialiseImp(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	short stateID;

	if (item->TriggerFlags == 2 || item->TriggerFlags == 12)
	{
		stateID = STATE_IMP_START_ROLL;
		item->AnimNumber = Objects[ID_IMP].animIndex + 8;
	}
	else if (item->TriggerFlags == 1 || item->TriggerFlags == 11)
	{
		stateID = STATE_IMP_START_CLIMB;
		item->AnimNumber = Objects[ID_IMP].animIndex + 7;
	}
	else
	{
		stateID = 1;
		item->AnimNumber = Objects[ID_IMP].animIndex + 1;
	}

	item->TargetState = stateID;
	item->ActiveState = stateID;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
}

static void ImpThrowStones(ITEM_INFO* item)
{
	PHD_VECTOR pos1;
	pos1.x = 0;
	pos1.y = 0;
	pos1.z = 0;
	GetJointAbsPosition(item, &pos1, 9);

	PHD_VECTOR pos2;
	pos2.x = 0;
	pos2.y = 0;
	pos2.z = 0;
	GetLaraJointPosition(&pos2, LM_HEAD);

	int dx = pos1.x - pos2.x;
	int dy = pos1.y - pos2.y;
	int dz = pos1.z - pos2.z;

	short angles[2];
	phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
	
	int distance = sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));
	if (distance < 8)
		distance = 8;

	angles[0] += GetRandomControl() % (distance / 4) - (distance / 8);
	angles[1] += GetRandomControl() % (distance / 2) - (distance / 4);
	
	short fxNum = CreateNewEffect(item->RoomNumber);
	if (fxNum != NO_ITEM)
	{
		FX_INFO* fx = &EffectList[fxNum];
		fx->pos.xPos = pos1.x;
		fx->pos.yPos = pos1.y;
		fx->pos.zPos = pos1.z;
		fx->roomNumber = item->RoomNumber;
		fx->pos.xRot = (angles[1] + distance) / 2;
		fx->pos.yRot = angles[0];
		fx->pos.zRot = 0;
		fx->speed = 4 * sqrt(distance);
		if (fx->speed < 256)
			fx->speed = 256;
		fx->fallspeed = 0;
		fxNum = Objects[ID_IMP_ROCK].meshIndex + (GetRandomControl() & 7);
		fx->objectNumber = ID_IMP_ROCK;
		fx->shade = 16912;
		fx->counter = 0;
		fx->frameNumber = fxNum;
		fx->flag1 = 2;
		fx->flag2 = 0x2000;
	}
}

void ImpControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short joint0 = 0;
		short joint1 = 0;
		short angle = 0;
		short joint2 = 0;
		short joint3 = 0;
		short angle2 = 0;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
			{
				GetAITarget(creature);
			}
			else if (creature->hurtByLara)
			{
				creature->enemy = LaraItem;
			}

			AI_INFO info;
			CreatureAIInfo(item, &info);
			
			if (creature->enemy == LaraItem)
			{
				angle2 = info.angle;
			}
			else
			{
				angle2 = phd_atan(LaraItem->Position.zPos - item->Position.zPos, LaraItem->Position.xPos - item->Position.xPos) - item->Position.yRot;
			}

			int d1 = item->Position.yPos - LaraItem->Position.yPos + 384;

			if (LaraItem->ActiveState == LS_CROUCH_IDLE
				|| LaraItem->ActiveState == LS_CROUCH_ROLL
				|| LaraItem->ActiveState > LS_MONKEY_TURN_180
				&& LaraItem->ActiveState < LS_HANG_TO_CRAWL
				|| LaraItem->ActiveState == LS_CROUCH_TURN_LEFT
				|| LaraItem->ActiveState == LS_CROUCH_TURN_RIGHT)
			{
				d1 = item->Position.yPos - LaraItem->Position.yPos;
			}

			int d2 = sqrt(info.distance);

			info.xAngle = phd_atan(d2, d1);

			GetCreatureMood(item, &info, VIOLENT);
			if (item->ActiveState == STATE_IMP_SCARED)
				creature->mood = ESCAPE_MOOD;

			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			joint1 = info.angle / 2;
			joint0 = info.xAngle / 2;
			joint3 = info.angle / 2;
			joint2 = info.xAngle / 2;

			if (Wibble & 0x10)
				item->SwapMeshFlags = 1024;
			else
				item->SwapMeshFlags = 0;

			switch (item->ActiveState)
			{
			case STATE_IMP_WALK:
				creature->maximumTurn = ANGLE(7);
				if (info.distance <= SQUARE(2048))
				{
					if (info.distance < SQUARE(2048))
						item->TargetState = STATE_IMP_STOP;
				}
				else
				{
					item->TargetState = STATE_IMP_RUN;
				}
				break;

			case STATE_IMP_STOP:
				creature->maximumTurn = -1;
				creature->flags = 0;
				if (info.bite && info.distance < SQUARE(170) && item->TriggerFlags < 10)
				{
					if (GetRandomControl() & 1)
						item->TargetState = STATE_IMP_ATTACK1;
					else
						item->TargetState = STATE_IMP_ATTACK2;
				}
				else if (item->AIBits & FOLLOW)
				{
					item->TargetState = STATE_IMP_WALK;
				}
				else
				{
					if (item->TriggerFlags == 3)
					{
						item->TargetState = STATE_IMP_THROW_STONES;
					}
					else if (info.distance <= SQUARE(2048))
					{
						if (info.distance > SQUARE(512) || item->TriggerFlags < 10)
							item->TargetState = STATE_IMP_WALK;
					}
					else
					{
						item->TargetState = STATE_IMP_RUN;
					}
				}
				break;

			case STATE_IMP_RUN:
				creature->maximumTurn = ANGLE(7);
				if (info.distance >= SQUARE(512))
				{
					if (info.distance < SQUARE(2048))
						item->TargetState = STATE_IMP_WALK;
				}
				else
				{
					item->TargetState = STATE_IMP_STOP;
				}
				break;

			case STATE_IMP_ATTACK1:
			case STATE_IMP_ATTACK2:
				creature->maximumTurn = -1;
				if (creature->flags == 0 
					&& item->TouchBits & 0x280)
				{
					LaraItem->HitPoints -= 3;
					LaraItem->HitStatus = true;
					CreatureEffect2(item, &ImpBite, 10, item->Position.yRot, DoBloodSplat);
				}
				break;

			case STATE_IMP_SCARED:
				creature->maximumTurn = ANGLE(7);
				break;

			case STATE_IMP_START_CLIMB:
			case STATE_IMP_START_ROLL:
				creature->maximumTurn = 0;
				break;

			case STATE_IMP_THROW_STONES:
				creature->maximumTurn = -1;
				if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 40)
					ImpThrowStones(item);
				break;

			default:
				break;

			}
		}
		else
		{
			item->HitPoints = 0;
			if (item->ActiveState != STATE_IMP_DEATH)
			{
				item->AnimNumber = Objects[ID_IMP].animIndex + ANIMATION_IMP_DEATH;
				item->ActiveState = STATE_IMP_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}

		if (creature->maximumTurn == -1)
		{
			creature->maximumTurn = 0;
			if (abs(angle2) >= ANGLE(2))
			{
				if (angle2 >= 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += angle2;
			}
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 3, joint3);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
