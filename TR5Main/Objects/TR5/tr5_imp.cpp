#include "../newobjects.h"
#include "../../Game/items.h"
#include "../../Game/sphere.h"
#include "../../Game/lara.h"
#include "../../Game/draw.h"
#include "../../Game/effects.h"
#include "../../Game/effect2.h"
#include "../../Game/Box.h"

BITE_INFO ImpBite = { 0, 0x64, 0, 9 };

void InitialiseImp(short itemNum)
{
    ITEM_INFO* item;
    short stateid;

    item = &Items[itemNum];
    ClearItem(itemNum);
    if (item->triggerFlags == 2 || item->triggerFlags == 12)
    {
        stateid = 8;
        item->animNumber = Objects[ID_IMP].animIndex + 8;
    }
    else if (item->triggerFlags == 1 || item->triggerFlags == 11)
    {
        stateid = 7;
        item->animNumber = Objects[ID_IMP].animIndex + 7;
    }
    else
    {
        stateid = 1;
        item->animNumber = Objects[ID_IMP].animIndex + 1;
    }
    item->goalAnimState = stateid;
    item->currentAnimState = stateid;
    item->frameNumber = Anims[item->animNumber].frameBase;
}

void ImpThrowStones(ITEM_INFO* item)
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
	GetLaraJointPosition(&pos2, LJ_HEAD);

	int dx = pos1.x - pos2.x;
	int dy = pos1.y - pos2.y;
	int dz = pos1.z - pos2.z;

	short angles[2];
	phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
	
	int distance = SQRT_ASM(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));
	if (distance < 8)
		distance = 8;

	angles[0] += GetRandomControl() % (distance >> 2) - (distance >> 3);
	angles[1] += GetRandomControl() % (distance >> 1) - (distance >> 2);
	
	short fxNum = CreateNewEffect(item->roomNumber);
	if (fxNum != NO_ITEM)
	{
		FX_INFO* fx = &Effects[fxNum];
		fx->pos.xPos = pos1.x;
		fx->pos.yPos = pos1.y;
		fx->pos.zPos = pos1.z;
		fx->roomNumber = item->roomNumber;
		fx->pos.xRot = angles[1] + distance >> 1;
		fx->pos.yRot = angles[0];
		fx->pos.zRot = 0;
		fx->speed = 4 * SQRT_ASM(distance);
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

void ControlImp(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short joint0 = 0;
		short joint1 = 0;
		short angle = 0;
		short joint2 = 0;
		short joint3 = 0;
		short angle2 = 0;

		ITEM_INFO* item = &Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		if (item->hitPoints > 0)
		{
			if (item->aiBits)
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
				angle2 = ATAN(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos) - item->pos.yRot;
			}

			int d1 = item->pos.yPos - LaraItem->pos.yPos + 384;

			if (LaraItem->currentAnimState == STATE_LARA_CROUCH_IDLE
				|| LaraItem->currentAnimState == STATE_LARA_CROUCH_ROLL
				|| LaraItem->currentAnimState > STATE_LARA_MONKEYSWING_TURNAROUND
				&& LaraItem->currentAnimState < STATE_LARA_CLIMB_TO_CRAWL
				|| LaraItem->currentAnimState == STATE_LARA_CROUCH_TURN_LEFT
				|| LaraItem->currentAnimState == STATE_LARA_CROUCH_TURN_RIGHT)
			{
				d1 = item->pos.yPos - LaraItem->pos.yPos;
			}

			int d2 = SQRT_ASM(info.distance);

			info.xAngle = ATAN(d2, d1);

			GetCreatureMood(item, &info, VIOLENT);
			if (item->currentAnimState == 6)
				creature->mood = ESCAPE_MOOD;

			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			joint1 = info.angle >> 1;
			joint0 = info.xAngle >> 1;
			joint3 = info.angle >> 1;
			joint2 = info.xAngle >> 1;

			if (Wibble & 0x10)
				item->swapMeshFlags = 1024;
			else
				item->swapMeshFlags = 0;

			switch (item->currentAnimState)
			{
			case 0:
				creature->maximumTurn = ANGLE(7);
				if (info.distance <= SQUARE(2048))
				{
					if (info.distance < SQUARE(2048))
						item->goalAnimState = 1;
				}
				else
				{
					item->goalAnimState = 2;
				}
				break;

			case 1:
				creature->maximumTurn = -1;
				creature->flags = 0;
				if (info.bite && info.distance < SQUARE(170) && item->triggerFlags < 10)
				{
					if (GetRandomControl() & 1)
						item->goalAnimState = 3;
					else
						item->goalAnimState = 5;
				}
				else if (item->aiBits & FOLLOW)
				{
					item->goalAnimState = 0;
				}
				else
				{
					if (item->triggerFlags == 3)
					{
						item->goalAnimState = 11;
					}
					else if (info.distance <= SQUARE(2048))
					{
						if (info.distance > SQUARE(512) || item->triggerFlags < 10)
							item->goalAnimState = 0;
					}
					else
					{
						item->goalAnimState = 2;
					}
				}
				break;

			case 2:
				creature->maximumTurn = ANGLE(7);
				if (info.distance >= SQUARE(512))
				{
					if (info.distance < SQUARE(2048))
						item->goalAnimState = 0;
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;

			case 3:
			case 5:
				creature->maximumTurn = -1;
				if (creature->flags == 0 
					&& item->touchBits & 0x280)
				{
					LaraItem->hitPoints -= 3;
					LaraItem->hitStatus = true;
					CreatureEffect2(item, &ImpBite, 10, item->pos.yRot, DoBloodSplat);
				}
				break;

			case 6:
				creature->maximumTurn = ANGLE(7);
				if (TorchRoom != 11)
					item->goalAnimState = 1;
				break;

			case 7:
			case 8:
				creature->maximumTurn = 0;
				break;

			case 11:
				creature->maximumTurn = -1;
				if (item->frameNumber - Anims[item->animNumber].frameBase == 40)
					ImpThrowStones(item);
				break;

			default:
				break;

			}
		}
		else
		{
			item->hitPoints = 0;
			if (item->currentAnimState != 9)
			{
				item->animNumber = Objects[ID_IMP].animIndex + 45;
				item->currentAnimState = 9;
				item->frameNumber = Anims[item->animNumber].frameBase;
			}
		}

		if (creature->maximumTurn == -1)
		{
			creature->maximumTurn = 0;
			if (abs(angle2) >= ANGLE(2))
			{
				if (angle2 >= 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += angle2;
			}
		}

		if (TorchRoom == 11)
			item->goalAnimState = 6;

		CreatureTilt(item, 0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 3, joint3);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}