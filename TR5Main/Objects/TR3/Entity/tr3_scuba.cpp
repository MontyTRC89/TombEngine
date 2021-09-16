#include "framework.h"
#include "tr3_scuba.h"
#include "items.h"
#include "box.h"
#include "lara.h"
#include "setup.h"
#include "level.h"
#include "itemdata/creature_info.h"
#include "control.h"
#include "control\los.h"
#include "effects\effects.h"

BITE_INFO scubaGun = { 17, 164, 44, 18 };

static void ShootHarpoon(ITEM_INFO* frogman, int x, int y, int z, short speed, short yRot, short roomNumber)
{
	short harpoonItemNum = CreateItem();
	if (harpoonItemNum != NO_ITEM)
	{
		ITEM_INFO* harpoon = &g_Level.Items[harpoonItemNum];

		harpoon->objectNumber = ID_SCUBA_HARPOON;
		harpoon->roomNumber = frogman->roomNumber;

		harpoon->pos.xPos = x;
		harpoon->pos.yPos = y;
		harpoon->pos.zPos = z;

		InitialiseItem(harpoonItemNum);

		harpoon->pos.xRot = 0;
		harpoon->pos.yRot = yRot;
		harpoon->speed = 150;

		AddActiveItem(harpoonItemNum);
		harpoon->status = ITEM_ACTIVE;
	}
}

void ScubaHarpoonControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 50;
		LaraItem->hitStatus = true;
		DoBloodSplat(item->pos.xPos, item->pos.yPos, item->pos.zPos, (GetRandomControl() & 3) + 4, LaraItem->pos.yRot, LaraItem->roomNumber);
		KillItem(itemNum);
	}
	else
	{
		int ox = item->pos.xPos;
		int oz = item->pos.zPos;

		short speed = item->speed * phd_cos(item->pos.xRot);
		item->pos.zPos += speed * phd_cos(item->pos.yRot);
		item->pos.xPos += speed * phd_sin(item->pos.yRot);
		item->pos.yPos += -item->speed * phd_sin(item->pos.xRot);

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNum, roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (item->pos.yPos >= item->floor)
			KillItem(itemNum);
	}
}

void ScubaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	int waterHeight;
	short angle = 0;
	short head = 0;
	short neck = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 16;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}

		CreatureFloat(itemNumber);
		return;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		GAME_VECTOR start;
		GAME_VECTOR target;
		bool shoot = false;

		if (Lara.waterStatus == false)
		{
			start.x = item->pos.xPos;
			start.y = item->pos.yPos - STEP_SIZE;
			start.z = item->pos.zPos;
			start.roomNumber = item->roomNumber;

			target.x = LaraItem->pos.xPos;
			target.y = LaraItem->pos.yPos - (LARA_HEIGHT - 150);
			target.z = LaraItem->pos.zPos;

			shoot = LOS(&start, &target);
			if (shoot)
			{
				creature->target.x = LaraItem->pos.xPos;
				creature->target.y = LaraItem->pos.yPos;
				creature->target.z = LaraItem->pos.zPos;
			}

			if (info.angle < -ANGLE(45) || info.angle > ANGLE(45))
				shoot = false;
		}
		else if (info.angle > -ANGLE(45) && info.angle < ANGLE(45))
		{
			start.x = item->pos.xPos;
			start.y = item->pos.yPos;
			start.z = item->pos.zPos;
			start.roomNumber = item->roomNumber;

			target.x = LaraItem->pos.xPos;
			target.y = LaraItem->pos.yPos;
			target.z = LaraItem->pos.zPos;

			shoot = LOS(&start, &target);
		}
		else
			shoot = false;

		angle = CreatureTurn(item, creature->maximumTurn);
		waterHeight = GetWaterSurface(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber) + WALL_SIZE / 2;

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = ANGLE(3);
			if (shoot)
				neck = -info.angle;

			if (creature->target.y < waterHeight && item->pos.yPos < waterHeight + creature->LOT.fly)
				item->goalAnimState = 2;
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (shoot)
				item->goalAnimState = 4;
			break;

		case 4:
			creature->flags = 0;

			if (shoot)
				neck = -info.angle;

			if (!shoot || creature->mood == ESCAPE_MOOD || (creature->target.y < waterHeight && item->pos.yPos < waterHeight + creature->LOT.fly))
				item->goalAnimState = 1;
			else
				item->goalAnimState = 3;
			break;

		case 3:
			if (shoot)
				neck = -info.angle;

			if (!creature->flags)
			{
				ShootHarpoon(item, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->speed, item->pos.yRot, item->roomNumber);
				creature->flags = 1;
			}
			break;


		case 2:
			creature->maximumTurn = ANGLE(3);

			if (shoot)
				head = info.angle;

			if (creature->target.y > waterHeight)
				item->goalAnimState = 1;
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (shoot)
				item->goalAnimState = 6;
			break;

		case 6:
			creature->flags = 0;

			if (shoot)
				head = info.angle;

			if (!shoot || creature->mood == ESCAPE_MOOD || creature->target.y > waterHeight)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 7;
			break;

		case 7:
			if (shoot)
				head = info.angle;

			if (!creature->flags)
			{
				ShootHarpoon(item, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->speed, item->pos.yRot, item->roomNumber);
				creature->flags = 1;
			}
			break;
			
		}
	}

	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNumber, angle, 0);

	switch (item->currentAnimState)
	{
	case 1:
	case 4:
	case 3:
		CreatureUnderwater(item, 512);
		break;
	default:
		item->pos.yPos = waterHeight - 512;
	}
}