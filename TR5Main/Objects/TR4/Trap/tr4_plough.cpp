#include "framework.h"
#include "tr4_plough.h"
#include "level.h"
#include "control/control.h"
#include "animation.h"

void PloughControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	item->itemFlags[3] = 50;
	if (TriggerActive(item))
	{
		item->itemFlags[0] = 258048;
		AnimateItem(item);
	}
	else
	{
		item->itemFlags[0] = 0;
	}
}