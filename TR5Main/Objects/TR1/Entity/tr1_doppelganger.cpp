#include "framework.h"
#include "Objects/TR1/Entity/tr1_doppelganger.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/misc.h"
#include "Specific/level.h"

// TODO: Evil lara is not targetable and cant move like lara.

// get weapon damage to damage lara instead. (*25)
static short GetWeaponDamage(int weaponType)
{
	return short(Weapons[weaponType].damage) * 25;
}

ITEM_INFO* findReference(ITEM_INFO* item, short objectNum)
{
	int itemNum;
	bool found = false;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		ITEM_INFO* itemz = &g_Level.Items[i];
		if (itemz->objectNumber == objectNum && itemz->roomNumber == item->roomNumber)
		{
			itemNum = i;
			found = true;
		}
	}
	if (!found)
		itemNum = NO_ITEM;

	return (itemNum == NO_ITEM ? NULL : &g_Level.Items[itemNum]);
}

// original:
void InitialiseDoppelganger(short itemNum)
{
	ClearItem(itemNum);
}

void DoppelgangerControl(short itemNum)
{
	ITEM_INFO* item;
	ITEM_INFO* ref;
	FLOOR_INFO* floor;
	int h, lh;
	int x, y, z;
	short room_num;

	item = &g_Level.Items[itemNum];


	if (item->hitPoints < 1000)                   			// If Evil Lara being Injured
	{                                                       // then take the hits off Lara instead...
		LaraItem->hitPoints -= GetWeaponDamage(Lara.gunType);
		item->hitPoints = 1000;
	}

	ref = findReference(item, ID_BACON_REFERENCE); // find reference point

	if (item->data == NULL)
	{
		if (ref == nullptr) // if no reference found, she doesn't move
		{
			x = item->pos.xPos;
			y = LaraItem->pos.yPos;
			z = item->pos.zPos;
		}
		else
		{
			x = 2 * ref->pos.xPos - LaraItem->pos.xPos;
			y = LaraItem->pos.yPos;
			z = 2 * ref->pos.zPos - LaraItem->pos.zPos;
		}
		// get bacon height
		room_num = item->roomNumber;
		floor = GetFloor(x, y, z, &room_num);
		h = GetFloorHeight(floor, x, y, z);
		item->floor = h;
		// get lara height
		room_num = LaraItem->roomNumber;
		floor = GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &room_num);
		lh = GetFloorHeight(floor, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
		// animate bacon
		item->frameNumber = LaraItem->frameNumber;
		item->animNumber = LaraItem->animNumber;
		// move bacon
		item->pos.xPos = x;
		item->pos.yPos = y;
		item->pos.zPos = z;
		item->pos.xRot = LaraItem->pos.xRot;
		item->pos.yRot = LaraItem->pos.yRot - ANGLE(180); // make sure she's facing Lara
		item->pos.zRot = LaraItem->pos.zRot;

		ItemNewRoom(itemNum, LaraItem->roomNumber);				// Follow Laras Room

		if (h >= lh + WALL_SIZE + 1 && !LaraItem->gravityStatus) // added +1 to avoid bacon dying when exiting water rooms
		{
			SetAnimation(item, LA_JUMP_WALL_SMASH_START);
			item->gravityStatus = true;
			item->fallspeed = 0;
			item->speed = 0;
			item->data = -1;
			item->pos.yPos += 50;
		}
	}
	
	if (item->data)
	{
		AnimateItem(item);
		room_num = item->roomNumber;
		x = item->pos.xPos;
		y = item->pos.yPos;
		z = item->pos.zPos;
		floor = GetFloor(x, y, z, &room_num);
		h = GetFloorHeight(floor, x, y, z);
		item->floor = h;
		TestTriggers(x, y, z, item->roomNumber, true);
		if (item->pos.yPos >= h)
		{
			item->floor = item->pos.yPos = h;
			TestTriggers(x, h, z, item->roomNumber, true);

			item->gravityStatus = false;
			item->fallspeed = 0;
			item->goalAnimState = LS_DEATH;
			item->requiredAnimState = LS_DEATH;
		}
	}
}

// TODO: DrawLara not exist ! use Renderer11.cpp DrawLara instead or create DrawLara() function with old behaviour.
void DrawEvilLara(ITEM_INFO* item)
{
	/*
	short* meshstore[15];
	short** meshpp;
	int i;

	meshpp = &Meshes[Objects[item->objectNumber].meshIndex];           	// Save Laras Mesh Pointers
	for (i = 0; i < 15; i++)
	{
		meshstore[i] = Lara.meshPtrs[i];
		Lara.meshPtrs[i] = *(meshpp++);
	}

	DrawLara(item);

	for (i = 0; i < 15; i++)
		Lara.meshPtrs[i] = meshstore[i];*/
}