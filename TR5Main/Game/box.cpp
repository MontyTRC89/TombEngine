#include "Box.h"
#include "..\Global\global.h"
#include "items.h"
#include "tomb4fx.h"
#include "lot.h"
#include "deltapak.h"
#include "items.h"

/*void __cdecl CreatureDie(__int16 itemNum, __int32 explode)
{
	ITEM_INFO* item = &Items[itemNum];

	item->hitPoints = -16384;
	item->collidable = false;

	if (explode)
	{
		if (Objects[item->objectNumber].hitEffect == 1)
			ExplodingDeath2(itemNum, -1, 258);
		else
			ExplodingDeath2(itemNum, -1, 256);

		KillItem(itemNum);
	}
	else
		RemoveActiveItem(itemNum);

	DisableBaddieAI(itemNum);
	item->flags |= IFLAG_KILLED;
	DropBaddyPickups(item);

	if (item->objectNumber == ID_SCIENTIST && item->aiBits == 20)
	{
		item = FindItem(ID_ROLLINGBALL);
		if (item)
		{
			if (!(item->flags & IFLAG_INVISIBLE))
			{
				item->flags |= IFLAG_ACTIVATION_MASK;
				AddActiveItem(item - Items);
			}
		}
	}
}*/

__int32 CreatureVault(__int16 itemNum, __int16 angle, __int32 vault, __int32 shift)
{
	ITEM_INFO* item = &Items[itemNum];

	__int32 xBlock = item->pos.xPos >> WALL_SHIFT;
	__int32 y = item->pos.yPos;
	__int32 zBlock = item->pos.zPos >> WALL_SHIFT;
	__int32 roomNumber = item->roomNumber;

	CreatureAnimation(itemNum, angle, 0);

	if (item->floor > y + STEP_SIZE * 7 / 2)
		vault = -4;
	else if (item->floor > y + STEP_SIZE * 5)
		vault = -3;
	else if (item->floor > y + STEP_SIZE * 3 / 2 && item->objectNumber != ID_BADDY1 && item->objectNumber != ID_BADDY2) // Baddy 1&2 don't have some climb down animations
		vault = -2;
	else if (item->pos.yPos > y - STEP_SIZE * 3 / 2)
		return 0;
	else if (item->pos.yPos > y - STEP_SIZE * 5 / 2)
		vault = 2;
	else if (item->pos.yPos > y - STEP_SIZE * 7 / 2)
		vault = 3;
	else
		vault = 4;

	__int32 newXblock = item->pos.xPos >> WALL_SHIFT;
	__int32 newZblock = item->pos.zPos >> WALL_SHIFT;
	
	if (zBlock == newZblock)
	{
		if (xBlock == newXblock)
			return 0;

		if (xBlock < newXblock)
		{
			item->pos.xPos = (newXblock << WALL_SHIFT) - shift;
			item->pos.yRot = ANGLE(90);
		}
		else
		{
			item->pos.xPos = (xBlock << WALL_SHIFT) + shift;
			item->pos.yRot = -ANGLE(90);
		}
	}
	else if (xBlock == newXblock)
	{
		if (zBlock < newZblock)
		{
			item->pos.zPos = (newZblock << WALL_SHIFT) - shift;
			item->pos.yRot = 0;
		}
		else
		{
			item->pos.zPos = (zBlock << WALL_SHIFT) + shift;
			item->pos.yRot = -ANGLE(180);
		}
	}

	item->pos.yPos = item->floor = y;
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNum, roomNumber);

	return vault;
}

void Inject_Box()
{
	INJECT(0x0040B5D0, CreatureVault);
	//INJECT(0x0040A090, CreatureDie);
}