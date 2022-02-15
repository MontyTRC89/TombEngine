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
	return short(Weapons[weaponType].Damage) * 25;
}

ITEM_INFO* findReference(ITEM_INFO* item, short objectNum)
{
	int itemNum;
	bool found = false;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		ITEM_INFO* itemz = &g_Level.Items[i];
		if (itemz->ObjectNumber == objectNum && itemz->RoomNumber == item->RoomNumber)
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


	if (item->HitPoints < 1000)                   			// If Evil Lara being Injured
	{                                                       // then take the hits off Lara instead...
		LaraItem->HitPoints -= GetWeaponDamage(Lara.Control.WeaponControl.GunType);
		item->HitPoints = 1000;
	}

	ref = findReference(item, ID_BACON_REFERENCE); // find reference point

	if (item->Data == NULL)
	{
		if (ref == nullptr) // if no reference found, she doesn't move
		{
			x = item->Position.xPos;
			y = LaraItem->Position.yPos;
			z = item->Position.zPos;
		}
		else
		{
			x = 2 * ref->Position.xPos - LaraItem->Position.xPos;
			y = LaraItem->Position.yPos;
			z = 2 * ref->Position.zPos - LaraItem->Position.zPos;
		}
		// get bacon height
		room_num = item->RoomNumber;
		floor = GetFloor(x, y, z, &room_num);
		h = GetFloorHeight(floor, x, y, z);
		item->Floor = h;
		// get lara height
		room_num = LaraItem->RoomNumber;
		floor = GetFloor(LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos, &room_num);
		lh = GetFloorHeight(floor, LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos);
		// animate bacon
		item->FrameNumber = LaraItem->FrameNumber;
		item->AnimNumber = LaraItem->AnimNumber;
		// move bacon
		item->Position.xPos = x;
		item->Position.yPos = y;
		item->Position.zPos = z;
		item->Position.xRot = LaraItem->Position.xRot;
		item->Position.yRot = LaraItem->Position.yRot - ANGLE(180); // make sure she's facing Lara
		item->Position.zRot = LaraItem->Position.zRot;

		ItemNewRoom(itemNum, LaraItem->RoomNumber);				// Follow Laras Room

		if (h >= lh + WALL_SIZE + 1 && !LaraItem->Airborne) // added +1 to avoid bacon dying when exiting water rooms
		{
			SetAnimation(item, LA_JUMP_WALL_SMASH_START);
			item->Airborne = true;
			item->VerticalVelocity = 0;
			item->Velocity = 0;
			item->Data = -1;
			item->Position.yPos += 50;
		}
	}
	
	if (item->Data)
	{
		AnimateItem(item);
		room_num = item->RoomNumber;
		x = item->Position.xPos;
		y = item->Position.yPos;
		z = item->Position.zPos;
		floor = GetFloor(x, y, z, &room_num);
		h = GetFloorHeight(floor, x, y, z);
		item->Floor = h;
		TestTriggers(x, y, z, item->RoomNumber, true);
		if (item->Position.yPos >= h)
		{
			item->Floor = item->Position.yPos = h;
			TestTriggers(x, h, z, item->RoomNumber, true);

			item->Airborne = false;
			item->VerticalVelocity = 0;
			item->TargetState = LS_DEATH;
			item->RequiredState = LS_DEATH;
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