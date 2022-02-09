#include "framework.h"
#include "Objects/TR3/Entity/tr3_scuba.h"

#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO scubaGun = { 17, 164, 44, 18 };

static void ShootHarpoon(ITEM_INFO* frogman, int x, int y, int z, short speed, short yRot, short roomNumber)
{
	short harpoonItemNum = CreateItem();
	if (harpoonItemNum != NO_ITEM)
	{
		ITEM_INFO* harpoon = &g_Level.Items[harpoonItemNum];

		harpoon->ObjectNumber = ID_SCUBA_HARPOON;
		harpoon->RoomNumber = frogman->RoomNumber;

		harpoon->Position.xPos = x;
		harpoon->Position.yPos = y;
		harpoon->Position.zPos = z;

		InitialiseItem(harpoonItemNum);

		harpoon->Position.xRot = 0;
		harpoon->Position.yRot = yRot;
		harpoon->VerticalVelocity = 150;

		AddActiveItem(harpoonItemNum);
		harpoon->Status = ITEM_ACTIVE;
	}
}

void ScubaHarpoonControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->TouchBits)
	{
		LaraItem->HitPoints -= 50;
		LaraItem->HitStatus = true;
		DoBloodSplat(item->Position.xPos, item->Position.yPos, item->Position.zPos, (GetRandomControl() & 3) + 4, LaraItem->Position.yRot, LaraItem->RoomNumber);
		KillItem(itemNum);
	}
	else
	{
		int ox = item->Position.xPos;
		int oz = item->Position.zPos;

		short speed = item->VerticalVelocity * phd_cos(item->Position.xRot);
		item->Position.zPos += speed * phd_cos(item->Position.yRot);
		item->Position.xPos += speed * phd_sin(item->Position.yRot);
		item->Position.yPos += -item->VerticalVelocity * phd_sin(item->Position.xRot);

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		if (item->RoomNumber != roomNumber)
			ItemNewRoom(itemNum, roomNumber);
		item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
		if (item->Position.yPos >= item->Floor)
			KillItem(itemNum);
	}
}

void ScubaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->Data;
	int waterHeight;
	short angle = 0;
	short head = 0;
	short neck = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 16;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
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
			start.x = item->Position.xPos;
			start.y = item->Position.yPos - STEP_SIZE;
			start.z = item->Position.zPos;
			start.roomNumber = item->RoomNumber;

			target.x = LaraItem->Position.xPos;
			target.y = LaraItem->Position.yPos - (LARA_HEIGHT - 150);
			target.z = LaraItem->Position.zPos;

			shoot = LOS(&start, &target);
			if (shoot)
			{
				creature->target.x = LaraItem->Position.xPos;
				creature->target.y = LaraItem->Position.yPos;
				creature->target.z = LaraItem->Position.zPos;
			}

			if (info.angle < -ANGLE(45) || info.angle > ANGLE(45))
				shoot = false;
		}
		else if (info.angle > -ANGLE(45) && info.angle < ANGLE(45))
		{
			start.x = item->Position.xPos;
			start.y = item->Position.yPos;
			start.z = item->Position.zPos;
			start.roomNumber = item->RoomNumber;

			target.x = LaraItem->Position.xPos;
			target.y = LaraItem->Position.yPos;
			target.z = LaraItem->Position.zPos;

			shoot = LOS(&start, &target);
		}
		else
			shoot = false;

		angle = CreatureTurn(item, creature->maximumTurn);
		waterHeight = GetWaterSurface(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber) + WALL_SIZE / 2;

		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = ANGLE(3);
			if (shoot)
				neck = -info.angle;

			if (creature->target.y < waterHeight && item->Position.yPos < waterHeight + creature->LOT.fly)
				item->TargetState = 2;
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (shoot)
				item->TargetState = 4;
			break;

		case 4:
			creature->flags = 0;

			if (shoot)
				neck = -info.angle;

			if (!shoot || creature->mood == ESCAPE_MOOD || (creature->target.y < waterHeight && item->Position.yPos < waterHeight + creature->LOT.fly))
				item->TargetState = 1;
			else
				item->TargetState = 3;
			break;

		case 3:
			if (shoot)
				neck = -info.angle;

			if (!creature->flags)
			{
				ShootHarpoon(item, item->Position.xPos, item->Position.yPos, item->Position.zPos, item->VerticalVelocity, item->Position.yRot, item->RoomNumber);
				creature->flags = 1;
			}
			break;


		case 2:
			creature->maximumTurn = ANGLE(3);

			if (shoot)
				head = info.angle;

			if (creature->target.y > waterHeight)
				item->TargetState = 1;
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (shoot)
				item->TargetState = 6;
			break;

		case 6:
			creature->flags = 0;

			if (shoot)
				head = info.angle;

			if (!shoot || creature->mood == ESCAPE_MOOD || creature->target.y > waterHeight)
				item->TargetState = 2;
			else
				item->TargetState = 7;
			break;

		case 7:
			if (shoot)
				head = info.angle;

			if (!creature->flags)
			{
				ShootHarpoon(item, item->Position.xPos, item->Position.yPos, item->Position.zPos, item->VerticalVelocity, item->Position.yRot, item->RoomNumber);
				creature->flags = 1;
			}
			break;
			
		}
	}

	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNumber, angle, 0);

	switch (item->ActiveState)
	{
	case 1:
	case 4:
	case 3:
		CreatureUnderwater(item, 512);
		break;
	default:
		item->Position.yPos = waterHeight - 512;
	}
}