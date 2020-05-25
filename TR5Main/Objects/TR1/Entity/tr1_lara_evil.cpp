#include "framework.h"
#include "newobjects.h"
#include "box.h"
#include "items.h"
#include "larafire.h"
#include "misc.h"
#include "level.h"
#include "lara.h"

// TODO: Evil lara is not targetable and cant move like lara.

// get weapon damage to damage lara instead. (*25)
static short GetWeaponDamage(int weaponType)
{
	return short(Weapons[weaponType].damage) * 25;
}

// original:
void InitialiseEvilLara(short itemNum)
{
	ClearItem(itemNum);
}

void LaraEvilControl(short itemNum)
{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	int h, lh;
	int x, y, z;
	short room_num;

	item = &Items[itemNum];

	if (item->hitPoints < 1000)                   			// If Evil Lara being Injured
	{                                                       // then take the hits off Lara instead...
		LaraItem->hitPoints -= GetWeaponDamage(Lara.gunType);
		item->hitPoints = 1000;
	}

	if (item->data == NULL)
	{
		// TODO: fix evil lara moving.
		room_num = item->roomNumber;
		x = item->pos.xPos; // 2*36*WALL_SIZE - LaraItem->pos.xPos;
		y = item->pos.yPos; // LaraItem->pos.yPos;
		z = item->pos.zPos; // 2*60*WALL_SIZE - LaraItem->pos.zPos;
		floor = GetFloor(x, y, z, &room_num);
		h = GetFloorHeight(floor, x, y, z);
		item->floor = h;
		room_num = LaraItem->roomNumber;
		floor = GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &room_num);
		lh = GetFloorHeight(floor, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
		item->frameNumber = LaraItem->frameNumber;
		item->animNumber = LaraItem->animNumber;
		//item->pos.xPos = x;
		//item->pos.yPos = y;
		//item->pos.zPos = z;
		item->pos.xRot = LaraItem->pos.xRot;
		item->pos.yRot = LaraItem->pos.yRot - ANGLE(180);
		item->pos.zRot = LaraItem->pos.zRot;
		ItemNewRoom(itemNum, LaraItem->roomNumber);				// Follow Laras Room

		if (h >= lh + WALL_SIZE && !LaraItem->gravityStatus)
		{
			item->goalAnimState = STATE_LARA_FREEFALL;      	// Make Player Stop Immediately
			item->currentAnimState = STATE_LARA_FREEFALL;   	// and Skip directly into fastfall
			item->frameNumber = GF(ANIMATION_LARA_SMASH_JUMP, 0);
			item->animNumber = ANIMATION_LARA_SMASH_JUMP;
			item->gravityStatus = true;
			item->fallspeed = 0;
			item->speed = 0;
			item->data = (void*)-1;
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
		TestTriggers(TriggerIndex, TRUE, 0);
		if (item->pos.yPos >= h)
		{
			item->floor = item->pos.yPos = h;
			floor = GetFloor(x, h, z, &room_num);
			GetFloorHeight(floor, x, h, z);
			TestTriggers(TriggerIndex, TRUE, 0);
			item->gravityStatus = false;
			item->fallspeed = 0;
			item->goalAnimState = STATE_LARA_DEATH;
			item->requiredAnimState = STATE_LARA_DEATH;
		}
	}
}

// TODO: drawLara not exist ! use Renderer11.cpp drawLara instead or create DrawLara() function with old behaviour.
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

	drawLara(item);

	for (i = 0; i < 15; i++)
		Lara.meshPtrs[i] = meshstore[i];*/
}