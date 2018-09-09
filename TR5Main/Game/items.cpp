#include "items.h"
#include "..\Global\global.h"
#include "effect2.h"
#include <stdio.h>

void __cdecl ClearItem(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	item->data = NULL;
	item->collidable = true;
}

void __cdecl ItemNewRoom(__int16 itemNum, __int16 roomNumber)
{
	if (InItemControlLoop)
	{
		printf("ItemNewRoomNo: %d\n", ItemNewRoomNo);

		ItemNewRooms[ItemNewRoomNo * 2] = itemNum;
		ItemNewRooms[ItemNewRoomNo * 2 + 1] = roomNumber;
		ItemNewRoomNo++;
	}
	else
	{
		ITEM_INFO* item = &Items[itemNum];

		if (item->roomNumber != 255)
		{
			ROOM_INFO* r = &Rooms[item->roomNumber];

			if (r->itemNumber == roomNumber)
			{
				r->itemNumber = item->nextItem;
			}
			else if (r->itemNumber != -1)
			{
				short linknum;
				for (linknum = Items[r->itemNumber].nextItem; linknum != -1; linknum = Items[linknum].nextItem)
				{
					if (linknum == itemNum)
					{
						Items[r->itemNumber].nextItem = item->nextItem;
						break;
					}
				}
			}
		}

		item->roomNumber = roomNumber;
		item->nextItem = Rooms[roomNumber].itemNumber;
		Rooms[roomNumber].itemNumber = itemNum;
	}
}

void Inject_Items()
{
	INJECT(0x00440DA0, ItemNewRoom);
}
