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

	if (item->ActiveState)
	{
		if (item->ActiveState == 1 && item->TouchBits)
		{
			LaraItem->HitPoints -= 300;
			LaraItem->HitStatus = true;
		}
	}
	else
	{
		item->TargetState = 1;
		item->Airborne = true;;
	}

	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
	{
		RemoveActiveItem(itemNumber);
	}
	else
	{
		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

		if (roomNumber != item->RoomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (item->ActiveState == 1)
		{
			if (item->Position.yPos >= item->Floor)
			{
				item->Position.yPos = item->Floor;
				item->Airborne = false;
				item->TargetState = 2;
				item->VerticalVelocity = 0;
			}
		}
	}
}
