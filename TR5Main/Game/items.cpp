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

void Inject_Items()
{
	//INJECT(0x00440DA0, ItemNewRoom);
}
