#include "framework.h"
#include "tr4_crocodile.h"
#include "box.h"
#include "effect.h"
#include "people.h"
#include "items.h"
#include "setup.h"
#include "level.h"
#include "lara.h"

BITE_INFO crocodileBite = { 0, -156, 500, 9 };

void InitialiseCrocodile(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	ObjectInfo* obj = &Objects[item->objectNumber];
	ROOM_INFO* room = &Rooms[item->roomNumber];

	ClearItem(itemNumber);

	if (room->flags & ENV_FLAG_WATER)
	{
		item->animNumber = obj->animIndex + 12;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 8;
		item->currentAnimState = 8;
	}
	else
	{
		item->animNumber = obj->animIndex;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 1;
		item->currentAnimState = 1;
	}
}

void CrocodileControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ObjectInfo* obj = &Objects[item->objectNumber];

	int x = item->pos.xPos + phd_sin(item->pos.yRot) << 10 >> W2V_SHIFT;
	int y = item->pos.yPos;
	int z = item->pos.zPos + phd_cos(item->pos.yRot) << 10 >> W2V_SHIFT;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (abs(y - height1) > 512)
		height1 = y;

	x = item->pos.xPos - phd_sin(item->pos.yRot) << 10 >> W2V_SHIFT;
	y = item->pos.yPos;
	z = item->pos.zPos - phd_cos(item->pos.yRot) << 10 >> W2V_SHIFT;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	if (abs(y - height2) > 512)
		height2 = y;

	short at = phd_atan(2048, height2 - height1);
	short angle = 0;
	short joint0 = 0;
	short joint2 = 0;

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;

		if (item->currentAnimState != 7 && item->currentAnimState != 10)
		{
			if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			{
				item->animNumber = obj->animIndex + 16;
				item->goalAnimState = 10;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 10;
				item->hitPoints = -16384;
			}
			else
			{
				item->animNumber = obj->animIndex + 11;
				item->goalAnimState = 7;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 7;
			}
		}

		if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			CreatureFloat(itemNumber);
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else if (creature->hurtByLara)
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus || info.distance < SQUARE(1536) || TargetVisible(item, &info) && info.distance < SQUARE(5120))
		{
			if (!creature->alerted)
				creature->alerted = true;
			AlertAllGuards(itemNumber);
		}

		joint0 = 4 * angle;

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;

			if (item->aiBits & GUARD)
			{
				joint0 = item->itemFlags[0];
				item->goalAnimState = 1;
				item->itemFlags[0] += item->itemFlags[1];

				if (!(GetRandomControl() & 0x1F))
				{
					if (GetRandomControl() & 1)
					{
						item->itemFlags[1] = 0;
					}
					else
					{
						item->itemFlags[1] = (GetRandomControl() & 1) != 0 ? 12 : -12;
					}
				}

				if (item->itemFlags[0] <= 1024)
				{
					if (item->itemFlags[0] < -1024)
					{
						item->itemFlags[0] = -1024;
					}
				}
				else
				{
					item->itemFlags[0] = 1024;
				}
			}
			else if (info.angle && info.distance < SQUARE(768))
			{
				item->goalAnimState = 5;
			}
			else
			{
				if (!info.ahead || info.distance >= SQUARE(1024))
				{
					item->goalAnimState = 2;
					break;
				}
				item->goalAnimState = 3;
			}

			break;

		case 2:
			creature->maximumTurn = ANGLE(3);

			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else
			{
				if (info.angle && info.distance < SQUARE(768))
				{
					item->goalAnimState = 1;
				}
				if (info.ahead)
				{
					if (info.distance < SQUARE(1024))
					{
						item->goalAnimState = 3;
					}
				}
			}

			break;

		case 3:
			creature->maximumTurn = ANGLE(3);
			creature->LOT.step = 256;
			creature->LOT.drop = -256;

			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (info.angle && info.distance < SQUARE(768))
			{
				item->goalAnimState = 1;
			}
			else if (!info.ahead || info.distance > SQUARE(1536))
			{
				item->goalAnimState = 2;
			}

			break;

		case 5:
			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				item->requiredAnimState = 0;
			}
			if (info.angle && item->touchBits & 0x300)
			{
				if (!item->requiredAnimState)
				{
					CreatureEffect2(
						item,
						&crocodileBite,
						10,
						-1,
						DoBloodSplat);

					LaraItem->hitPoints -= 120;
					LaraItem->hitStatus = true;

					item->requiredAnimState = 1;
				}
			}
			else
			{
				item->goalAnimState = 1;
			}

			break;

		case 8:
			creature->maximumTurn = ANGLE(3);
			creature->LOT.step = 20480;
			creature->LOT.drop = -20480;

			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (info.angle)
			{
				if (item->touchBits & 0x300)
				{
					item->goalAnimState = 9;
				}
			}

			break;

		case 9:
			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				item->requiredAnimState = 0;
			}
			if (info.angle && item->touchBits & 0x300)
			{
				if (!item->requiredAnimState)
				{
					CreatureEffect2(
						item,
						&crocodileBite,
						10,
						-1,
						DoBloodSplat);

					LaraItem->hitPoints -= 120;
					LaraItem->hitStatus = true;

					item->requiredAnimState = 8;
				}
			}
			else
			{
				item->goalAnimState = 8;
			}

			break;

		default:
			break;
		}
	}

	CreatureTilt(item, 0);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint0);
	CreatureJoint(item, 2, -joint0);
	CreatureJoint(item, 3, -joint0);

	short xRot = item->pos.xRot;

	if (!(abs(angle - item->pos.xRot) < 256 || item->currentAnimState >= 8))
	{
		if (angle <= xRot)
		{
			if (angle < xRot)
				item->pos.xRot = xRot - 256;
		}
		else
		{
			item->pos.xRot = xRot + 256;
		}
	}
	else
	{
		if (item->currentAnimState < 8)
			item->pos.xRot = angle;

		CreatureAnimation(itemNumber, angle, 0);

		roomNumber = item->roomNumber;

		if (item->currentAnimState == 8)
		{
			GetFloor(
				item->pos.xPos + (phd_sin(item->pos.yRot) << 10 >> W2V_SHIFT),
				item->pos.yPos,
				item->pos.zPos + (phd_cos(item->pos.yRot) << 10 >> W2V_SHIFT),
				&roomNumber);
		}
		else
		{
			GetFloor(
				item->pos.xPos + (phd_sin(item->pos.yRot) << 9 >> W2V_SHIFT),
				item->pos.yPos,
				item->pos.zPos + (phd_cos(item->pos.yRot) << 10 >> W2V_SHIFT),
				&roomNumber);
		}

		if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		{
			if (Rooms[roomNumber].flags & ENV_FLAG_WATER)
			{
				if (item->currentAnimState == 2)
				{
					item->requiredAnimState = 3;
					item->goalAnimState = 3;
				}
				else if (item->currentAnimState == 3)
				{
					item->requiredAnimState = 8;
					item->goalAnimState = 8;
				}
				else if (item->animNumber != obj->animIndex + 17)
				{
					creature->LOT.step = 20480;
					creature->LOT.drop = -20480;
					creature->LOT.fly = 16;

					CreatureUnderwater(item, STEP_SIZE);
				}
			}
			else
			{
				item->requiredAnimState = 3;
				item->goalAnimState = 3;
				creature->LOT.step = 256;
				creature->LOT.drop = -256;
				creature->LOT.fly = 0;

				CreatureUnderwater(item, 0);
			}
		}
		else
		{
			creature->LOT.fly = 0;
		}

		return;
	}
}