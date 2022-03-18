#include "framework.h"
#include "tr5_fallingceiling.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/control/control.h"
#include "Game/animation.h"

void FallingCeilingControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->currentAnimState)
	{
		if (item->currentAnimState == 1 && item->touchBits)
		{
			LaraItem->hitPoints -= 300;
			LaraItem->hitStatus = true;
		}
	}
	else
	{
		item->goalAnimState = 1;
		item->gravityStatus = true;;
	}

	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED)
	{
		RemoveActiveItem(itemNumber);
	}
	else
	{
		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (roomNumber != item->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (item->currentAnimState == 1)
		{
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->gravityStatus = false;
				item->goalAnimState = 2;
				item->fallspeed = 0;
			}
		}
	}
}
