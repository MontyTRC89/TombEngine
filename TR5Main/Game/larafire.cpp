#include "larafire.h"
#include "items.h"
#include <stdio.h>

void __cdecl SmashItem(__int16 itemNum)
{
	/*ITEM_INFO* item = &Items[itemNum];
	__int16 objectNumber = item->objectNumber;
	printf("SmashItem\n");
	if (objectNumber >= ID_SMASH_OBJECT1 && objectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNum);
	else if (objectNumber == ID_BELL_SWITCH)
	{
		if (item->status != ITEM_ACTIVE)
		{
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
		}
	}*/
}

void Inject_LaraFire()
{
	//INJECT(0x00453A90, SmashItem);
}