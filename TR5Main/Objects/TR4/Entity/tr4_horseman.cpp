#include "framework.h"
#include "tr4_horseman.h"
#include "items.h"
#include "effect2.h"
#include "setup.h"
#include "level.h"
#include "control.h"
#include "trmath.h"

BITE_INFO horseBite1 = { 0, 0, 0, 0x0D };
BITE_INFO horseBite2 = { 0, 0, 0, 0x11 };
BITE_INFO horseBite3 = { 0, 0, 0, 0x13 };
BITE_INFO horsemanBite1 = { 0, 0, 0, 0x06 };
BITE_INFO horsemanBite2 = { 0, 0, 0, 0x0E };
BITE_INFO horsemanBite3 = { 0, 0, 0, 0x0A };

static void HorsemanSparks(PHD_3DPOS* pos, int param1, int num)
{
	for (int i = 0; i < num; i++)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		int r = GetRandomControl();

		spark->on = 1;
		spark->sG = -128;
		spark->sB = (r & 0xF) + 16;
		spark->sR = 0;
		spark->dG = 96;
		spark->dB = ((r >> 4) & 0x1F) + 48;
		spark->dR = 0;
		spark->colFadeSpeed = 2;
		spark->fadeToBlack = 4;
		spark->life = 9;
		spark->sLife = 9;
		spark->transType = COLADD;
		spark->x = pos->xPos;
		spark->y = pos->yPos;
		spark->z = pos->zPos;
		spark->friction = 34;
		spark->yVel = (r & 0xFFF) - 2048;
		spark->flags = SP_NONE;
		spark->gravity = (r >> 7) & 0x1F;
		spark->maxYvel = 0;
		spark->zVel = phd_cos((r & 0x7FF) + param1 - 1024) >> 2;
		spark->xVel = -phd_sin((r & 0x7FF) + param1 - 1024) >> 2;
	}

	for (int i = 0; i < num; i++)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		int r = GetRandomControl();

		spark->on = 1;
		spark->sG = -128;
		spark->sR = 0;
		spark->dG = 96;
		spark->sB = (r & 0xF) + 16;
		spark->dR = 0;
		spark->colFadeSpeed = 2;
		spark->fadeToBlack = 4;
		spark->dB = ((r >> 4) & 0x1F) + 48;
		spark->life = 9;
		spark->sLife = 9;
		spark->transType = COLADD;
		spark->x = pos->xPos;
		spark->y = pos->yPos;
		spark->z = pos->zPos;
		spark->yVel = (r & 0xFFF) - 2048;
		spark->gravity = (r >> 7) & 0x1F;
		spark->rotAng = r >> 3;
		if (r & 1)
		{
			spark->rotAdd = -16 - (r & 0xF);
		}
		else
		{
			spark->rotAdd = spark->sB;
		}
		spark->scalar = 3;
		spark->friction = 34;
		spark->sSize = spark->size = ((r >> 5) & 7) + 4;
		spark->dSize = spark->sSize >> 1;
		spark->flags = 26;
		spark->maxYvel = 0;
		spark->zVel = phd_cos((r & 0x7FF) + param1 - 1024) >> 2;
		spark->xVel = -phd_sin((r & 0x7FF) + param1 - 1024) >> 2;
	}
}

void InitialiseHorse(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	OBJECT_INFO* obj = &Objects[ID_HORSE];

	item->animNumber = obj->animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void InitialiseHorseman(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	OBJECT_INFO* obj = &Objects[ID_HORSEMAN];

	ClearItem(itemNumber);

	item->animNumber = obj->animIndex + 8;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 9;
	item->currentAnimState = 9;
	item->itemFlags[0] = NO_ITEM; // No horse yet
}

void HorsemanControl(short itemNumber)
{
	printf("[Horseman] Not Implemented !");
#ifdef OLD_CODE
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	// Try to find the horse
	if (item->itemFlags[0] == NO_ITEM)
	{
		for (int i = 0; i < LevelItems; i++)
		{
			ITEM_INFO* currentItem = &Items[i];
			if (item->objectNumber == ID_HORSE && item->triggerFlags == currentItem->triggerFlags)
			{
				item->itemFlags[0] = i;
				currentItem->flags |= 0x20;
			}
		}
	}

	// If no horse was found, then set it to 0 so it won't be searched anymore in the future
	if (item->itemFlags[0] == NO_ITEM)
		item->itemFlags[0] = 0;

	// The horse
	ITEM_INFO * horseItem = NULL;
	if (item->itemFlags[0] != 0)
		horseItem = &Items[item->itemFlags[0]];

	int x;
	int y;
	int z;
	short roomNumber;
	int deltaX;
	int deltaZ;
	FLOOR_INFO * floor;
	short height;
	short height1;
	short height2;
	int xRot = 0;

	if (horseItem != NULL)
	{
		roomNumber = item->roomNumber;

		x = horseItem->pos.xPos;
		y = horseItem->pos.yPos;
		z = horseItem->pos.zPos;

		x = horseItem->pos.xPos + 341 * phd_sin(horseItem->pos.yRot) >> 14;
		y = horseItem->pos.yPos;
		z = horseItem->pos.zPos + 341 * phd_cos(horseItem->pos.yRot) >> 14;

		floor = GetFloor(x, y, z, &roomNumber);
		height1 = GetFloorHeight(floor, x, y, z);

		x = horseItem->pos.xPos - 341 * phd_sin(horseItem->pos.yRot) >> 14;
		y = horseItem->pos.yPos;
		z = horseItem->pos.zPos - 341 * phd_cos(horseItem->pos.yRot) >> 14;

		floor = GetFloor(x, y, z, &roomNumber);
		height2 = GetFloorHeight(floor, x, y, z);

		xRot = phd_atan(682, height2 - height1);
	}

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		if (item->itemFlags[1] == 0)
		{
			if (item->currentAnimState != 16)
			{
				item->animNumber = Objects[ID_HORSEMAN].animIndex + 21;
				item->currentAnimState = 16;
				item->frameNumber = Anims[item->animNumber].frameBase;

				if (item->itemFlags[0])
				{
					Items[item->itemFlags[0]].afterDeath = 1;
				}
			}
		}
		else
		{
			item->hitPoints = 100;
			item->aiBits = 0;
			item->itemFlags[1] = 0;
			item->animNumber = Objects[ID_HORSEMAN].animIndex + 3;
			item->currentAnimState = 8;
			item->frameNumber = Anims[item->animNumber].frameBase;

			creature->enemy = NULL;

			horseItem->goalAnimState = 1;
		}

		if (horseItem && item->itemFlags[1])
		{
			if (abs(xRot - item->pos.xRot) < 256)
			{
				item->pos.xRot = xRot;
			LABEL_182:
				horseItem->pos.xPos = item->pos.xPos;
				horseItem->pos.yPos = item->pos.yPos;
				horseItem->pos.zPos = item->pos.zPos;
				horseItem->pos.xRot = item->pos.xRot;
				horseItem->pos.zRot = item->pos.zRot;

				if (horseItem->roomNumber != item->roomNumber)
				{
					ItemNewRoom(item->itemFlags[0], item->roomNumber);
				}
				AnimateItem(horseItem);
			END:
				Objects[ID_HORSEMAN].radius = item->itemFlags[1] != 0 ? 409 : 170;
				CreatureAnimation(itemNumber, angle, 0);
				return;
			}
		}
	}

	if (item->aiBits)
	{
		GetAITarget(creature);
	}
	else if (creature->hurtByLara)
	{
		creature->enemy = LaraItem;
	}

	AI_INFO info;
	AI_INFO laraInfo;

	CreatureAIInfo(item, &info);

	if (creature->enemy == LaraItem)
	{
		laraInfo.angle = info.angle;
		laraInfo.distance = info.distance;
	}
	else
	{
		deltaX = LaraItem->pos.zPos - item->pos.zPos;
		deltaZ = LaraItem->pos.zPos - item->pos.zPos;

		laraInfo.angle = phd_atan(deltaX, deltaZ) - item->pos.yRot;
		laraInfo.distance = SQUARE(deltaX) + SQUARE(deltaZ);
	}

	short angle = 0;
	short tilt = 0;

	if (item->hitStatus
		&& laraInfo.angle < 12288
		&& laraInfo.angle > -12288
		&& laraInfo.distance < SQUARE(2048))
	{
		if (!horseItem || !item->itemFlags[1])
		{
			Objects[ID_HORSEMAN].radius = item->itemFlags[1] != 0 ? 409 : 170;
			CreatureAnimation(itemNumber, angle, 0);
			return;
		}

		if (abs(xRot - item->pos.xRot) < 256)
		{
			item->pos.xRot = xRot;
		}
		else
		{
			if (xRot > item->pos.xRot)
			{
				item->pos.xRot += 256;
			}
			else if (xRot < item->pos.xRot)
			{
				item->pos.xRot -= 256;
			}
		}

		horseItem->pos.xPos = item->pos.xPos;
		horseItem->pos.yPos = item->pos.yPos;
		horseItem->pos.zPos = item->pos.zPos;
		horseItem->pos.xRot = item->pos.xRot;
		horseItem->pos.yRot = item->pos.yRot;
		horseItem->pos.zRot = item->pos.zRot;

		if (horseItem->roomNumber != item->roomNumber)
		{
			ItemNewRoom(item->itemFlags[0], item->roomNumber);
		}

		AnimateItem(horseItem);

		Objects[ID_HORSEMAN].radius = item->itemFlags[1] != 0 ? 409 : 170;
		CreatureAnimation(itemNumber, angle, 0);

		if (item->currentAnimState != 15)
		{
			if (laraInfo.angle > 0)
			{
				goto LABEL_188;
			}

			if (laraInfo.angle <= 0)
			{
				if (item->itemFlags[1])
				{
				LABEL_47:
					if (!item->itemFlags[1])
					{
						v36 = item->meshBits;
						if (item->meshBits & 0x400)
						{
							item->requiredAnimState = 15;
						}
					}
					goto LABEL_50;
				}
			}

			if (item->itemFlags[1])
			{
			LABEL_47:
				if (!item->itemFlags[1])
				{
					v36 = item->meshBits;
					if (v36 & 0x400)
					{
						item->requiredAnimState = 15;
					}
				}
				goto LABEL_50;
			}
			v35 = item->meshBits;
			if (!(v35 & 0x400))
			{
			LABEL_188:
				if (Lara_GunType == 4)
				{
					item->hitPoints -= 10;
					LOBYTE(v34) = v34 | 0x10;
					item->MainFlags = v34;
				}
				else if (Lara_GunType == 2)
				{
					item->hitPoints -= 20;
					LOBYTE(v34) = v34 | 0x10;
					item->MainFlags = v34;
				}
				else
				{
					--item->hitPoints;
				}
				SoundEffect(301, &item->pos, 0);
				SoundEffect(213, &item->pos, 0);
				v68 = 0;
				v69 = -128;
				v70 = 80;
				GetJointAbsPosition((int)item, &v68, 1);
				sub_408D90(&v68, (signed short)item->pos.yRot, 7);
				goto LABEL_47;
			}
		}
		if (!(GetRandomControl() & 7))
		{
			if (item->currentAnimState == 15)
			{
				item->goalAnimState = 9;
			}
			ExplodeItemNode((int)item, 10, 1, -24);
		}

		return;
	}

	creature->hurtByLara = false;

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	angle = CreatureTurn(item, creature->maximumTurn);

	switch (item->currentAnimState)
	{
	case 1:
		creature->maximumTurn = ANGLE(3);
		horseItem->goalAnimState = 2;
		if (item->requiredAnimState)
		{
			item->goalAnimState = 17;
			horseItem->goalAnimState = 5;
		}
		else if (creature->flags || creature->reachedGoal || item->hitStatus && !GetRandomControl())
		{
			if (laraInfo.distance > SQUARE(4096) || creature->reachedGoal)
			{
				creature->flags = 0;
				creature->enemy = LaraItem;
				if (laraInfo.angle > -8192 && laraInfo.angle < 0x2000)
				{
					item->goalAnimState = 3;
					horseItem->goalAnimState = 1;
				}
			}
			else
			{
				item->aiBits = FOLLOW;
				item->itemFlags[3] = (-(item->itemFlags[3] != 1)) + 2;
			}
		}

		if (info.distance >= SQUARE(1024) || info.bite || info.angle >= -ANGLE(10) && info.angle <= ANGLE(10))
		{
			if (info.bite)
			{
				if (info.angle >= -ANGLE(10)
					|| info.distance >= SQUARE(1024) && (info.distance >= SQUARE(1365) || info.angle <= -ANGLE(20)))
				{
					if (info.angle > ANGLE(10) && (info.distance < SQUARE(1024) || info.distance < SQUARE(1365) &&
						info.angle < ANGLE(20)))
					{
						creature->maximumTurn = 0;
						item->goalAnimState = 6;
					}
				}
				else
				{
					creature->maximumTurn = 0;
					item->goalAnimState = 7;
				}
			}
		}
		else
		{
			item->goalAnimState = 3;
			horseItem->goalAnimState = 1;
		}

		break;

	case 2:
		creature->maximumTurn = 273;

		if (laraInfo.distance > SQUARE(4096) || creature->reachedGoal || creature->enemy == LaraItem)
		{
			creature->reachedGoal = false;
			creature->flags = 0;
			item->goalAnimState = 1;
			horseItem->goalAnimState = 2;
			creature->enemy = LaraItem;
		}

		break;

	case 3:
		creature->maximumTurn = 0;
		horseItem->goalAnimState = 1;

		if (creature->flags)
		{
			item->aiBits = FOLLOW;
			item->itemFlags[3] = -(item->itemFlags[3] != 1) + 2;
		}
		else
		{
			creature->flags = 0;
		}

		if (item->requiredAnimState)
		{
			item->goalAnimState = 1;
			horseItem->goalAnimState = 2;
			horseItem->flags = 0;
		}
		else if (creature->reachedGoal
			|| !horseItem->flags
			&& info.distance < SQUARE(1024)
			&& info.bite
			&& info.angle < ANGLE(10)
			&& info.angle > -ANGLE(10))
		{
			item->goalAnimState = 4;
			if (creature->reachedGoal)
			{
				item->requiredAnimState = 17;
			}
			horseItem->flags = 0;
		}
		else
		{
			item->goalAnimState = 1;
			horseItem->goalAnimState = 2;
			horseItem->flags = 0;
		}

		break;

	case 4:
		creature->maximumTurn = 0;
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			horseItem->animNumber = Objects[ID_HORSE].animIndex + 1;
			horseItem->currentAnimState = 4;
			horseItem->frameNumber = Anims[item->animNumber].frameBase;
		}
		if (!horseItem->flags)
		{
			if (horseItem->touchBits & 0x22000)
			{
				LaraItem->hitPoints -= 150;
				LaraItem->hitStatus = true;

				if (horseItem->touchBits & 0x2000)
				{
					CreatureEffect2(
						horseItem,
						&horseBite1,
						10,
						-1,
						DoBloodSplat);
				}
				else
				{
					CreatureEffect2(
						horseItem,
						&horseBite2,
						10,
						-1,
						DoBloodSplat);
				}

				horseItem->flags = 1;
			}
		}

		break;

	case 6:
		if (!creature->flags)
		{
			if (item->touchBits & 0x60)
			{
				LaraItem->hitPoints -= 250;
				LaraItem->hitStatus = true;

				CreatureEffect2(
					item,
					&horsemanBite1,
					10,
					item->pos.yRot,
					DoBloodSplat);

				creature->flags = 1;
			}
		}
		if (item->hitStatus)
		{
			item->goalAnimState = 9;
		}

		break;

	case 7:
		if (!creature->flags)
		{
			if (item->touchBits & 0x4000)
			{
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;

				CreatureEffect2(
					item,
					&horsemanBite2,
					3,
					item->pos.yRot,
					DoBloodSplat);

				creature->flags = 1;
			}
		}

		break;

	case 9:
		creature->maximumTurn = 0;
		creature->flags = 0;

		if (!item->aiBits || item->itemFlags[3])
		{
			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (info.bite && info.distance < SQUARE(682))
			{
				item->goalAnimState = 14;
			}
			else if (info.distance < SQUARE(6144) && info.distance > SQUARE(682))
			{
				item->goalAnimState = 10;
			}
		}
		else
		{
			item->goalAnimState = 10;
		}

		break;

	case 10:
		creature->maximumTurn = ANGLE(3);
		creature->flags = 0;

		if (creature->reachedGoal)
		{
			item->aiBits = 0;
			item->itemFlags[1] = 1;

			item->pos.xPos = horseItem->pos.xPos;
			item->pos.yPos = horseItem->pos.yPos;
			item->pos.zPos = horseItem->pos.zPos;
			item->pos.xRot = horseItem->pos.xRot;
			item->pos.yRot = horseItem->pos.yRot;
			item->pos.zRot = horseItem->pos.zRot;

			creature->reachedGoal = false;
			creature->enemy = NULL;

			item->animNumber = Objects[ID_HORSEMAN].animIndex + 14;
			item->currentAnimState = 5;
			item->frameNumber = Anims[item->animNumber].frameBase;

			creature->maximumTurn = 0;

			break;
		}

		if (item->hitStatus)
		{
			item->goalAnimState = 9;
			break;
		}

		if (info.bite && info.distance < SQUARE(682))
		{
			if (GetRandomControl() & 1)
			{
				item->goalAnimState = 12;
			}
			else if (GetRandomControl() & 1)
			{
				item->goalAnimState = 13;
			}
			else
			{
				item->goalAnimState = 9;
			}
		}
		else if (info.distance < SQUARE(5120) && info.distance > SQUARE(1365))
		{
			item->goalAnimState = 11;
		}

		break;

	case 11:
		if (info.distance < SQUARE(1365))
		{
			item->goalAnimState = 10;
		}

		break;

	case 12:
	case 13:
	case 14:
		creature->maximumTurn = 0;
		if (abs(info.angle) >= ANGLE(3))
		{
			if (info.angle >= 0)
			{
				item->pos.yRot += ANGLE(3);
			}
			else
			{
				item->pos.yRot -= ANGLE(3);
			}
		}
		else
		{
			item->pos.yRot += info.angle;
		}

		if (!creature->flags)
		{
			if (item->touchBits & 0x4000)
			{
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;

				CreatureEffect2(
					item,
					&horsemanBite2,
					3,
					item->pos.yRot,
					DoBloodSplat);

				creature->flags = 1;
			}
		}

		break;

	case 15:
		if (Lara.target != item || info.bite && info.distance < SQUARE(682))
		{
			item->goalAnimState = 9;
		}

		break;

	case 17:
		creature->reachedGoal = false;
		creature->maximumTurn = 546;

		if (!horseItem->flags)
		{
			if (horseItem->touchBits & 0xA2000)
			{
				LaraItem->hitPoints -= 150;
				LaraItem->hitStatus = true;

				if (horseItem->touchBits & 0x2000)
				{
					CreatureEffect2(
						horseItem,
						&horseBite1,
						10,
						-1,
						DoBloodSplat);
				}

				if (horseItem->touchBits & 0x20000)
				{
					CreatureEffect2(
						horseItem,
						&horseBite2,
						10,
						-1,
						DoBloodSplat);
				}

				if (horseItem->touchBits & 0x80000)
				{
					CreatureEffect2(
						horseItem,
						&horseBite3,
						10,
						-1,
						DoBloodSplat);
				}

				horseItem->flags = 1;
			}
		}
		if (!creature->flags)
		{
			if (item->touchBits & 0x460)
			{
				LaraItem->hitStatus = true;

				if (item->touchBits & 0x60)
				{
					CreatureEffect2(
						horseItem,
						&horsemanBite1,
						20,
						-1,
						DoBloodSplat);
					LaraItem->hitPoints -= 250;
				}
				else if (item->touchBits & 0x400)
				{
					CreatureEffect2(
						horseItem,
						&horsemanBite3,
						10,
						-1,
						DoBloodSplat);
					LaraItem->hitPoints -= 150;
				}

				creature->flags = 1;
			}
		}

		if (item->animNumber == Objects[ID_HORSEMAN].animIndex + 29 &&
			item->frameNumber == Anims[item->animNumber].frameBase)
		{
			horseItem->animNumber = Objects[ID_HORSE].animIndex + 10;
			horseItem->frameNumber = Anims[horseItem->animNumber].frameBase;
		}

		if (laraInfo.distance > SQUARE(4096) || creature->reachedGoal)
		{
			creature->reachedGoal = false;
			creature->flags = 0;
			creature->enemy = LaraItem;
		}
		else if (!info.ahead)
		{
			item->goalAnimState = 3;
			horseItem->goalAnimState = 1;
		}
		break;

	default:
		break;
	}
#endif
}