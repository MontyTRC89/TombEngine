#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"

BITE_INFO scubaGun = { 17, 164, 44, 18 };

__int32 __cdecl GetWaterSurface(__int32 x, __int32 y, __int32 z, __int16 roomNumber)
{
	ROOM_INFO* room = &Rooms[roomNumber];
	FLOOR_INFO* floor = &room->floor[((z - room->z) >> WALL_SHIFT) + ((x - room->x) >> WALL_SHIFT) * room->xSize];

	if (room->flags & ENV_FLAG_WATER)
	{
		while (floor->skyRoom != NO_ROOM)
		{
			room = &Rooms[floor->skyRoom];
			if (!(room->flags & ENV_FLAG_WATER))
				return (floor->ceiling << 8);
			floor = &room->floor[((z - room->z) >> WALL_SHIFT) + ((x - room->x) >> WALL_SHIFT) * room->xSize];
		}
		return NO_HEIGHT;
	}
	else
	{
		// Go down until hit water
		while (floor->pitRoom != NO_ROOM)
		{
			room = &Rooms[floor->pitRoom];
			if (room->flags & ENV_FLAG_WATER)
				return (floor->floor << 8);
			floor = &room->floor[((z - room->z) >> WALL_SHIFT) + ((x - room->x) >> WALL_SHIFT) * room->xSize];
		}
	}
	return NO_HEIGHT;
}

void __cdecl ShootHarpoon(ITEM_INFO* frogman, __int32 x, __int32 y, __int32 z, __int16 speed, __int16 yRot, __int16 roomNumber)
{
	__int16 harpoonItemNum = CreateItem();
	if (harpoonItemNum != NO_ITEM)
	{
		ITEM_INFO* harpoon = &Items[harpoonItemNum];

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

		//SoundEffect(247, &dart->pos, 0);
	}
}

void __cdecl HarpoonControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 50;
		LaraItem->hitStatus = true;
		DoBloodSplat(item->pos.xPos, item->pos.yPos, item->pos.zPos, (GetRandomControl() & 3) + 4, LaraItem->pos.yRot, LaraItem->roomNumber);
		KillItem(itemNum);
	}
	else
	{
		__int32 ox = item->pos.xPos;
		__int32 oz = item->pos.zPos;

		__int16 speed = (item->speed * COS(item->pos.xRot)) >> W2V_SHIFT;
		item->pos.zPos += (speed * COS(item->pos.yRot)) >> W2V_SHIFT;
		item->pos.xPos += (speed * SIN(item->pos.yRot)) >> W2V_SHIFT;
		item->pos.yPos += -((item->speed * SIN(item->pos.xRot)) >> W2V_SHIFT);

		__int16 roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNum, roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (item->pos.yPos >= item->floor)
		{
			//for (__int32 i = 0; i < 4; i++)
			//	TriggerDartSmoke(ox, item->pos.yPos, oz, 0, 0, 1);
			KillItem(itemNum);
			//		SoundEffect(258, &item->pos, 0);
		}
	}
}

void __cdecl ScubaControl(__int16 itemNumber)
{
	__int32 waterHeight;

	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	__int16 angle = 0;
	__int16 head = 0;
	__int16 neck = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 16;
			item->frameNumber = Anims[item->animNumber].frameBase;
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
			target.y = LaraItem->pos.yPos - (LARA_HITE - 150); // don't look at her origin, aim higher
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
			// Diver underwater
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