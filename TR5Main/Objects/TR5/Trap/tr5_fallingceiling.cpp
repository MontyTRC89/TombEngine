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

	if (item->activeState)
	{
		if (item->activeState == 1 && item->touchBits)
		{
			LaraItem->hitPoints -= 300;
			LaraItem->hitStatus = true;
		}
	}
	else
	{
		item->targetState = 1;
		item->Airborne = true;;
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

		if (item->activeState == 1)
		{
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->Airborne = false;
				item->targetState = 2;
				item->VerticalVelocity = 0;
			}
		}
	}
}
