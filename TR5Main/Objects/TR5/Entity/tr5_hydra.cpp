#include "framework.h"
#include "tr5_hydra.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

#define STATE_HYDRA_STOP			0
#define STATE_HYDRA_BITE_ATTACK1	1
#define STATE_HYDRA_AIM				2
#define STATE_HYDRA_HURT			4
#define STATE_HYDRA_BITE_ATTACK2	7
#define STATE_HYDRA_BITE_ATTACK3	8
#define STATE_HYDRA_BITE_ATTACK4	9
#define STATE_HYDRA_DEATH			11

BITE_INFO HydraBite{ 0, 0, 0, 0x0B };

void InitialiseHydra(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = 30 * item->triggerFlags + g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = STATE_HYDRA_STOP;
    item->currentAnimState = STATE_HYDRA_STOP;

    if (item->triggerFlags == 1)
        item->pos.zPos += 384;

    if (item->triggerFlags == 2)
        item->pos.zPos -= 384;

    item->pos.yRot = ANGLE(90);
    item->pos.xPos -= 256;
}

static void HydraBubblesAttack(PHD_3DPOS* pos, short roomNumber, int count)
{
	short fxNum = CreateNewEffect(roomNumber);
	if (fxNum != NO_ITEM)
	{
		FX_INFO* fx = &EffectList[fxNum];
		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		fx->pos.yRot = pos->yRot;
		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 16 * count + 15;
		fx->flag1 = 0;
		fx->objectNumber = ID_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 64;
		fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 8;
	}
}

void TriggerHydraMissileSparks(PHD_VECTOR* pos, short xv, short yv, short zv)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = true;
	spark->sB = 0;
	spark->sR = (GetRandomControl() & 0x3F) - 96;
	spark->sG = spark->sR / 2;
	spark->dB = 0;
	spark->dR = (GetRandomControl() & 0x3F) - 96;
	spark->dG = spark->dR / 2;
	spark->fadeToBlack = 8;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->transType = TransTypeEnum::COLADD;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 20;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->y = 0;
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->x += pos->x;
	spark->y += pos->y;
	spark->z += pos->z;
	spark->xVel = xv;
	spark->yVel = yv;
	spark->zVel = zv;
	spark->friction = 68;
	spark->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
	spark->rotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
		spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
	else
		spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
	spark->gravity = 0;
	spark->maxYvel = 0;
	spark->scalar = 1;
	spark->sSize = spark->size = (GetRandomControl() & 0xF) + 96;
	spark->dSize = spark->size / 4;
}

static void TriggerHydraSparks(short itemNumber, int frame)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->sB = 0;
	spark->sR = (GetRandomControl() & 0x3F) - 96;
	spark->dR = (GetRandomControl() & 0x3F) - 96;
	spark->dB = 0;
	if (frame < 16)
	{
		spark->sR = frame * spark->sR / 16;
		spark->dR = frame * spark->dR / 16;
	}
	spark->sG = spark->sR / 2;
	spark->dG = spark->dR / 2;
	spark->fadeToBlack = 4;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->transType = TransTypeEnum::COLADD;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->y = 0;
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->xVel = (byte)GetRandomControl() - 128;
	spark->yVel = 0;
	spark->zVel = (byte)GetRandomControl() - 128;
	spark->friction = 4;
	spark->flags = 4762;
	spark->fxObj = itemNumber;
	spark->nodeNumber = 5;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->maxYvel = 0;
	spark->gravity = -8 - (GetRandomControl() & 7);
	spark->scalar = 0;
	spark->dSize = 4;
	spark->sSize = spark->size = (frame * ((GetRandomControl() & 0xF) + 16)) / 16;
}

void HydraControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short tilt = 0;
	short joint3 = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
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

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (item->currentAnimState != 5
			&& item->currentAnimState != 10
			&& item->currentAnimState != STATE_HYDRA_DEATH)
		{
			if (abs(info.angle) >= ANGLE(1))
			{
				if (info.angle > 0)
					item->pos.yRot += ANGLE(1);
				else
					item->pos.yRot -= ANGLE(1);
			}
			else
			{
				item->pos.yRot += info.angle;
			}

			if (item->triggerFlags == 1)
			{
				tilt = -512;
			}
			else if (item->triggerFlags == 2)
			{
				tilt = 512;
			}
		}

		if (item->currentAnimState != 12)
		{
			joint1 = info.angle / 2;
			joint3 = info.angle / 2;
			joint2 = -info.xAngle;
		}

		joint0 = -joint1;

		int distance, damage, frame;
		PHD_VECTOR pos1, pos2;
		short angles[2];
		short roomNumber;
		PHD_3DPOS pos;

		switch (item->currentAnimState)
		{
		case STATE_HYDRA_STOP:
			creature->maximumTurn = ANGLE(1);
			creature->flags = 0;

			if (item->triggerFlags == 1)
			{
				tilt = -512;
			}
			else if (item->triggerFlags == 2)
			{
				tilt = 512;
			}

			if (info.distance >= SQUARE(1792) && GetRandomControl() & 0x1F)
			{
				if (info.distance >= SQUARE(2048) && GetRandomControl() & 0x1F)
				{
					if (!(GetRandomControl() & 0xF))
						item->goalAnimState = STATE_HYDRA_AIM;
				}
				else
				{
					item->goalAnimState = STATE_HYDRA_BITE_ATTACK1;
				}
			}
			else
			{
				item->goalAnimState = 6;
			}
			break;

		case STATE_HYDRA_BITE_ATTACK1:
		case STATE_HYDRA_BITE_ATTACK2:
		case STATE_HYDRA_BITE_ATTACK3:
		case STATE_HYDRA_BITE_ATTACK4:
			creature->maximumTurn = 0;

			if (creature->flags == 0)
			{
				if (item->touchBits & 0x400)
				{
					LaraItem->hitPoints -= 120;
					LaraItem->hitStatus = true;
					CreatureEffect2(item, &HydraBite, 10, item->pos.yRot, DoBloodSplat);
					creature->flags = 1;
				}

				if (item->hitStatus && info.distance < SQUARE(1792))
				{
					distance = sqrt(info.distance);
					damage = 5 - distance / 1024;

					if (Lara.gunType == WEAPON_SHOTGUN)
						damage *= 3;

					if (damage > 0)
					{
						item->hitPoints -= damage;
						item->goalAnimState = STATE_HYDRA_HURT;
						CreatureEffect2(item, &HydraBite, 10 * damage, item->pos.yRot, DoBloodSplat);
					}
				}
			}
			break;

		case STATE_HYDRA_AIM:
			creature->maximumTurn = 0;

			if (item->hitStatus)
			{
				// TEST: uncomment this for making HYDRA die on first hit event
				/*item->hitPoints = 0;
				break;*/

				damage = 6 - sqrt(info.distance) / 1024;

				if (Lara.gunType == WEAPON_SHOTGUN)
					damage *= 3;

				if ((GetRandomControl() & 0xF) < damage && info.distance < SQUARE(10240) && damage > 0)
				{
					item->hitPoints -= damage;
					item->goalAnimState = 4;
					CreatureEffect2(item, &HydraBite, 10 * damage, item->pos.yRot, DoBloodSplat);
				}
			}

			if (item->triggerFlags == 1)
			{
				tilt = -512;
			}
			else if (item->triggerFlags == 2)
			{
				tilt = 512;
			}

			if (!(GlobalCounter & 3))
			{
				frame = ((g_Level.Anims[item->animNumber].frameBase - item->frameNumber) / 8) + 1;
				if (frame > 16)
					frame = 16;
				TriggerHydraSparks(itemNumber, frame);
			}
			break;

		case 3:
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
			{
				pos1.x = 0;
				pos1.y = 1024;
				pos1.z = 40;
				GetJointAbsPosition(item, &pos1, 10);

				pos2.x = 0;
				pos2.y = 144;
				pos2.z = 40;
				GetJointAbsPosition(item, &pos2, 10);

				phd_GetVectorAngles(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z, angles);

				pos.xPos = pos1.x;
				pos.yPos = pos1.y;
				pos.zPos = pos1.z;
				pos.xRot = angles[1];
				pos.yRot = angles[0];
				pos.zRot = 0;

				roomNumber = item->roomNumber;
				GetFloor(pos2.x, pos2.y, pos2.z, &roomNumber);

				// TEST: uncomment this for making HYDRA not firing bubbles
				HydraBubblesAttack(&pos, roomNumber, 1);
			}
			break;

		case 6:
			creature->maximumTurn = ANGLE(1);
			creature->flags = 0;

			if (item->triggerFlags == 1)
			{
				tilt = -512;
			}
			else if (item->triggerFlags == 2)
			{
				tilt = 512;
			}

			if (info.distance >= SQUARE(768))
			{
				if (info.distance >= SQUARE(1280))
				{
					if (info.distance >= SQUARE(1792))
						item->goalAnimState = 0;
					else
						item->goalAnimState = 9;
				}
				else
				{
					item->goalAnimState = 8;
				}
			}
			else
			{
				item->goalAnimState = 7;
			}
			break;

		default:
			break;

		}
	}
	else
	{
		item->hitPoints = 0;

		if (item->currentAnimState != STATE_HYDRA_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 15;
			item->currentAnimState = STATE_HYDRA_DEATH;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		if (!((item->frameNumber - g_Level.Anims[item->animNumber].frameBase) & 7))
		{
			if (item->itemFlags[3] < 12)
			{
				ExplodeItemNode(item, 11 - item->itemFlags[3], 0, 64);
				SoundEffect(SFX_TR4_HIT_ROCK, &item->pos, 0);
				item->itemFlags[3]++;
			}
		}
	}

	CreatureTilt(item, tilt);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);

	CreatureAnimation(itemNumber, 0, 0);
}