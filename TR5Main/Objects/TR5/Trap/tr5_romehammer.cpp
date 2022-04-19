#include "framework.h"
#include "tr5_romehammer.h"
#include "Game/items.h"
#include "Specific/level.h"

void InitialiseRomeHammer(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->itemFlags[0] = 2;
	item->itemFlags[3] = 250;
}