#include "framework.h"
#include "tr4_plough.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void PloughControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		item->itemFlags[3] = 50;

		if (TriggerActive(item))
		{
			*((int*)&item->itemFlags) = 0x3F000;
			AnimateItem(item);
		}
		else
		{
			*((int*)&item->itemFlags) = 0;
		}
	}
}