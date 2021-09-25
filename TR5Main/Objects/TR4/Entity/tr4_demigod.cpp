#include "framework.h"
#include "tr4_demigod.h"
#include "items.h"
#include "control/box.h"
#include "people.h"
#include "sphere.h"
#include "effects\effects.h"
#include "animation.h"
#include "effects\tomb4fx.h"
#include "camera.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "control/control.h"
#include "itemdata/creature_info.h"
#include "item.h"

extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];

static void DemigodThrowEnergyAttack(PHD_3DPOS* pos, short roomNumber, int flags)
{
	short fxNum = CreateNewEffect(roomNumber);
	if (fxNum != -1)
	{
		FX_INFO* fx = &EffectList[fxNum];

		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		if (flags < 4)
		{
			fx->pos.yRot = pos->yRot;
		}
		else
		{
			fx->pos.yRot = pos->yRot + (GetRandomControl() & 0x7FF) - 1024;
		}

		OBJECT_INFO* obj = &Objects[ID_ENERGY_BUBBLES];

		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 2 * GetRandomControl() + -ANGLE(180);
		fx->flag1 = flags;
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->objectNumber = ID_ENERGY_BUBBLES;
		if (flags >= 4)
			flags--;
		fx->frameNumber = Objects[ID_ENERGY_BUBBLES].meshIndex + 2 * flags;
	}
}

static void DemigodEnergyAttack(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short animIndex = item->animNumber - Objects[item->objectNumber].animIndex;

	if (animIndex == 8)
	{
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			PHD_VECTOR pos1;
			PHD_VECTOR pos2;

			pos1.x = -544;
			pos1.y = 96;
			pos1.z = 0;

			GetJointAbsPosition(item, &pos1, 16);

			pos2.x = -900;
			pos2.y = 96;
			pos2.z = 0;

			GetJointAbsPosition(item, &pos2, 16);

			short angles[2];
			phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

			PHD_3DPOS pos;
			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;
			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			if (item->objectNumber == ID_DEMIGOD3)
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 3);
			}
			else
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 5);
			}
		}

		return;
	}

	if (animIndex != 16)
	{
		if (animIndex != 19)
			return;

		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			PHD_VECTOR pos1;
			PHD_VECTOR pos2;

			pos1.x = -544;
			pos1.y = 96;
			pos1.z = 0;

			GetJointAbsPosition(item, &pos1, 16);

			pos2.x = -900;
			pos2.y = 96;
			pos2.z = 0;

			GetJointAbsPosition(item, &pos2, 16);

			short angles[2];
			phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

			PHD_3DPOS pos;
			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;
			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			if (item->objectNumber == ID_DEMIGOD3)
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 3);
			}
			else
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 5);
			}
		}

		return;
	}

	// Animation 16 (State 10) is the big circle attack of DEMIGOD_3
	int frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

	if (frameNumber >= 8 && frameNumber <= 64)
	{
		PHD_VECTOR pos1;
		PHD_VECTOR pos2;

		pos1.x = 0;
		pos1.y = 0;
		pos1.z = 192;

		pos2.x = 0;
		pos2.y = 0;
		pos2.z = 384;

		if (GlobalCounter & 1)
		{
			GetJointAbsPosition(item, &pos1, 18);
			GetJointAbsPosition(item, &pos2, 18);
		}
		else
		{
			GetJointAbsPosition(item, &pos1, 17);
			GetJointAbsPosition(item, &pos2, 17);
		}

		short angles[2];
		phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

		PHD_3DPOS pos;
		pos.xPos = pos1.x;
		pos.yPos = pos1.y;
		pos.zPos = pos1.z;
		pos.xRot = angles[1];
		pos.yRot = angles[0];
		pos.zRot = 0;

		DemigodThrowEnergyAttack(&pos, item->roomNumber, 4);
	}
}

static void DemigodHammerAttack(int x, int y, int z, int something)
{
	int angle = 2 * GetRandomControl();
	int deltaAngle = 0x10000 / something;

	if (something > 0)
	{
		for (int i = 0; i < something; i++)
		{
			SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];

			spark->on = true;
			spark->sShade = 0;
			spark->colFadeSpeed = 4;
			spark->dShade = (GetRandomControl() & 0x1F) + 96;
			spark->fadeToBlack = 24 - (GetRandomControl() & 7);
			spark->transType = COLADD;
			spark->life = spark->sLife = (GetRandomControl() & 7) + 48;
			spark->x = (GetRandomControl() & 0x1F) + x - 16;
			spark->y = (GetRandomControl() & 0x1F) + y - 16;
			spark->z = (GetRandomControl() & 0x1F) + z - 16;
			spark->xVel = (byte)(GetRandomControl() + 256) * phd_sin(angle);
			spark->yVel = -32 - (GetRandomControl() & 0x3F);
			spark->zVel = (byte)(GetRandomControl() + 256) * phd_cos(angle);
			spark->friction = 9;

			if (GetRandomControl() & 1)
			{
				spark->flags = 16;
				spark->rotAng = GetRandomControl() & 0xFFF;
				if (GetRandomControl() & 1)
				{
					spark->rotAdd = -64 - (GetRandomControl() & 0x3F);
				}
				else
				{
					spark->rotAdd = (GetRandomControl() & 0x3F) + 64;
				}
			}
			else if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WIND)
			{
				spark->flags = 256;
			}
			else
			{
				spark->flags = SP_NONE;
			}
			spark->gravity = -4 - (GetRandomControl() & 3);
			spark->maxYvel = -4 - (GetRandomControl() & 3);
			spark->dSize = ((GetRandomControl() & 0x3F) + 64);
			spark->sSize = spark->dSize / 8;
			spark->size = spark->dSize / 8;

			angle += deltaAngle;
		}
	}
}

void InitialiseDemigod(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->goalAnimState = 0;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = 0;

	/*if (g_Level.NumItems > 0)
	{
		ITEM_INFO* currentItem = &g_Level.Items[0];
		int k = 0;

		while (item == currentItem || currentItem->objectNumber != ID_DEMIGOD3 || currentItem->itemFlags[0])
		{
			k++;
			currentItem++;
			if (k >= g_Level.NumItems)
				return;
		}

		item->itemFlags[0] = k;
	}*/
}

void DemigodControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	int someitemNumber = item->itemFlags[0];
	if (someitemNumber && g_Level.Items[someitemNumber].status == ITEM_ACTIVE && g_Level.Items[someitemNumber].active)
	{
		item->hitPoints = Objects[item->objectNumber].hitPoints;
		return;
	}

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		if (item->currentAnimState != 8 && item->currentAnimState != 15)
		{
			if (item->currentAnimState == 1 || item->currentAnimState == 2)
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 27;
				item->currentAnimState = 15;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
			else
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 12;
				item->currentAnimState = 8;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);

		AI_INFO info;
		AI_INFO laraInfo;

		int dx = 0;
		int dy = 0;
		int dz = 0;

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.ahead = info.ahead;
			laraInfo.angle = info.angle;
			laraInfo.xAngle = 0;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
			laraInfo.xAngle = 0;

			laraInfo.ahead = true;
			if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
				laraInfo.ahead = false;

			dx = abs(dx);
			dy = item->pos.yPos - LaraItem->pos.yPos;
			dz = abs(dz);

			if (dx <= dz)
				laraInfo.xAngle = phd_atan(dz + (dx / 2), dy);
			else
				laraInfo.xAngle = phd_atan(dx + (dz / 2), dy);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (laraInfo.ahead)
		{
			joint0 = laraInfo.angle / 2;
			joint1 = -laraInfo.xAngle;
			joint2 = laraInfo.angle / 2;
			joint3 = laraInfo.angle / 2;
		}
		else if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint1 = -info.xAngle;
			joint2 = info.angle / 2;
			joint3 = info.angle / 2;
		}

		switch (item->currentAnimState)
		{
		case 0:
			creature->maximumTurn = 0;
			if (info.ahead)
				joint1 = -info.xAngle;

			if (item->objectNumber == ID_DEMIGOD1)
			{
				if (info.distance >= SQUARE(3072))
				{
					item->goalAnimState = 1;
					break;
				}
				if (info.bite
					|| LaraItem->currentAnimState >= 56
					&& LaraItem->currentAnimState <= 61
					&& !Lara.location)
				{
					item->goalAnimState = 13;
					break;
				}
			}
			else
			{
				if (Targetable(item, &info))
				{
					creature->flags = 1;
					if (item->objectNumber == ID_DEMIGOD2)
						item->goalAnimState = 3;
					else
						item->goalAnimState = 11;
					break;
				}

				if (item->objectNumber == ID_DEMIGOD3)
				{
					if (info.distance <= SQUARE(2048) || info.distance >= SQUARE(5120))
					{
						item->goalAnimState = 1;
						break;
					}
					if (!(GetRandomControl() & 3))
					{
						item->goalAnimState = 9;
						break;
					}
				}
			}

			if (info.distance <= SQUARE(3072) || item->objectNumber != ID_DEMIGOD2)
			{
				item->goalAnimState = 1;
				break;
			}

			item->goalAnimState = 5;

			break;

		case 1:
			creature->maximumTurn = 1274;

			if (info.distance < SQUARE(2048))
			{
				item->goalAnimState = 0;
				break;
			}

			if (item->objectNumber == ID_DEMIGOD1)
			{
				if (info.distance < SQUARE(3072))
				{
					item->goalAnimState = 0;
					break;
				}
			}
			else
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = 0;
					break;
				}

			}

			if (info.distance > SQUARE(3072))
			{
				if (item->objectNumber == ID_DEMIGOD2)
				{
					item->goalAnimState = 5;
				}
				else
				{
					item->goalAnimState = 2;
				}
			}

			break;

		case 2:
			creature->maximumTurn = 1274;

			if (info.distance < SQUARE(2048))
			{
				item->goalAnimState = 0;
				break;
			}
			if (item->objectNumber == ID_DEMIGOD1)
			{
				if (info.distance < SQUARE(3072))
				{
					item->goalAnimState = 0;
					break;
				}
			}
			else
			{
				if (Targetable(item, &info) || item->objectNumber == ID_DEMIGOD3 && info.distance > SQUARE(2048))
				{
					item->goalAnimState = 0;
					break;
				}

				if (info.distance < SQUARE(3072))
				{
					item->goalAnimState = 1;
				}
			}

			break;

		case 3:
			if (info.ahead)
			{
				joint1 = -info.xAngle;
			}

			creature->maximumTurn = 0;

			if (item->animNumber == Objects[item->objectNumber].animIndex + 6)
			{
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
					{
						item->pos.yRot += ANGLE(7);
					}
					else
					{
						item->pos.yRot -= ANGLE(7);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}

			if (Targetable(item, &info) || creature->flags)
			{
				item->goalAnimState = 4;
				creature->flags = 0;
			}
			else
			{
				item->goalAnimState = 0;
				creature->flags = 0;
			}

			break;

		case 4:
		case 12:
			DemigodEnergyAttack(itemNumber);
			break;

		case 6:
			creature->maximumTurn = ANGLE(7);
			if (Targetable(item, &info))
			{
				item->goalAnimState = 7;
			}

			break;

		case 9:
			creature->maximumTurn = ANGLE(7);
			if (!Targetable(item, &info) && info.distance < SQUARE(5120))
			{
				item->goalAnimState = 10;
			}

			break;

		case 10:
			creature->maximumTurn = ANGLE(7);

			DemigodEnergyAttack(itemNumber);

			if (!Targetable(item, &info) || info.distance < SQUARE(5120) || !GetRandomControl())
			{
				item->goalAnimState = 0;
				break;
			}

			break;

		case 11:
			joint2 = joint0;
			joint0 = 0;

			if (info.ahead)
			{
				joint1 = -info.xAngle;
			}

			creature->maximumTurn = 0;

			if (item->animNumber == Objects[(signed short)item->objectNumber].animIndex + 6)
			{
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
					{
						item->pos.yRot += ANGLE(7);
					}
					else
					{
						item->pos.yRot -= ANGLE(7);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}
			if (Targetable(item, &info) || creature->flags)
			{
				item->goalAnimState = 12;
				creature->flags = 0;
			}
			else
			{
				item->goalAnimState = 0;
				creature->flags = 0;
			}

			break;

		case 13:
			// Aiming
			creature->maximumTurn = 0;
			joint2 = joint0;
			joint0 = 0;
			if (abs(info.angle) >= ANGLE(7))
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += ANGLE(7);
				}
				else
				{
					item->pos.yRot -= ANGLE(7);
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (info.distance >= SQUARE(3072)
				|| !info.bite
				&& (LaraItem->currentAnimState < 56 || LaraItem->currentAnimState > 61 || Lara.location))
			{
				item->goalAnimState = 0;
			}
			else
			{
				item->goalAnimState = 14;
			}

			break;

		case 14:
			// Hammer attack
			if (item->frameNumber - g_Level.Anims[item->animNumber].frameBase == 26)
			{
				PHD_VECTOR pos;

				pos.x = 80;
				pos.y = -8;
				pos.z = -40;

				GetJointAbsPosition(item, &pos, 17);

				short roomNumber = item->roomNumber;
				FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
				int height = GetFloorHeight(floor, pos.x, pos.y, pos.z);
				if (height == -32512)
				{
					pos.y = pos.y - 128;
				}
				else
				{
					pos.y = height - 128;
				}

				TriggerShockwave((PHD_3DPOS *)&pos, 24, 88, 256, 128, 128, 128, 32, 0, 2);
				DemigodHammerAttack(pos.x, pos.y + 128, pos.z, 8);

				Camera.bounce = -128;

				if (LaraItem->currentAnimState >= 56 && LaraItem->currentAnimState <= 61 && !Lara.location)
				{
					Lara.torsoXrot = 0; // dword_80DF6C 
					Lara.torsoYrot = 0; // dword_80DF6C 
					Lara.headXrot = 0;
					Lara.headYrot = 0;
					LaraItem->goalAnimState = 3;
					LaraItem->currentAnimState = 3;
					LaraItem->animNumber = 34;
					LaraItem->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					LaraItem->hitStatus = true;
					LaraItem->speed = 2;
					LaraItem->fallspeed = 1;
					Lara.gunStatus = LG_NO_ARMS;
				}
			}

		default:
			break;

		}
	}

	CreatureTilt(item, 0);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);

	CreatureAnimation(itemNumber, angle, 0);
}
