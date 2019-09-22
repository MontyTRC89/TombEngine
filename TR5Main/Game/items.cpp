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

__int16 __cdecl CreateItem()
{
	__int16 result = NextItemFree;
	if (NextItemFree != NO_ITEM)
	{
		Items[NextItemFree].flags = 0;
		NextItemFree = Items[NextItemFree].nextItem;
	}
	return result;
}

/*void __cdecl KillItem(__int16 itemNum)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = itemNum | 0x8000;
		ItemNewRoomNo++;
	}
	else
	{
		ITEM_INFO* item = &Items[itemNum];

		DetatchSpark(itemNum, 128);

		item->active = false;
		item->reallyActive = false;

		if (NextItemActive == itemNum)
		{
			NextItemActive = item->nextActive;
		}
		else if (NextItemActive != NO_ITEM)
		{
			__int16 linknum;
			for (linknum = Items[NextItemActive].nextActive; linknum != NO_ITEM; linknum = Items[linknum].nextActive)
			{
				if (linknum == itemNum)
				{
					Items[linknum].nextActive = item->nextActive;
					break;
				}
			}
		}

		if (item->roomNumber != 255)
		{
			if (Rooms[item->roomNumber].itemNumber == itemNum)
			{
				Rooms[item->roomNumber].itemNumber = item->nextItem;
			}
			else if (Rooms[item->roomNumber].itemNumber != -1)
			{
				short linknum;
				for (linknum = Items[Rooms[item->roomNumber].itemNumber].nextItem; linknum != NO_ITEM; linknum = Items[linknum].nextItem)
				{
					if (linknum == itemNum)
					{
						Items[Rooms[item->roomNumber].itemNumber].nextItem = item->nextItem;
						break;
					}
				}
			}
		}

		if (item == Lara.target)
			Lara.target = NULL;

		if (itemNum >= LevelItems)
		{
			item->nextItem = NextItemFree;
			NextItemFree = itemNum;
		}
		else
		{
			item->flags |= IFLAG_KILLED;
		}
	}
}*/

void __cdecl RemoveAllItemsInRoom(__int16 roomNumber, __int16 objectNumber)
{
	ROOM_INFO* room = &Rooms[roomNumber];
	__int16 currentItemNum = room->itemNumber;

	while (currentItemNum != NO_ITEM)
	{
		ITEM_INFO* item = &Items[currentItemNum];

		if (item->objectNumber == objectNumber)
		{
			RemoveActiveItem(currentItemNum);
			item->status = ITEM_INACTIVE;
			item->flags &= 0xC1;
		}

		currentItemNum = item->nextItem;
	}
}

void Inject_Items()
{
	INJECT(0x00440840, CreateItem);
	//INJECT(0x00440620, KillItem);
	//INJECT(0x00440DA0, ItemNewRoom);
}
