#include "framework.h"
#include "tr4_chain.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void ChainControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->triggerFlags)
		{
			item->itemFlags[2] = 1;
			item->itemFlags[3] = 75;

			if (TriggerActive(item))
			{
				*((int*)&item->itemFlags[0]) = 0x787E;
				AnimateItem(item);
				return;
			}
		}
		else
		{
			item->itemFlags[3] = 25;

			if (TriggerActive(item))
			{
				*((int*)&item->itemFlags[0]) = 0x780;
				AnimateItem(item);
				return;
			}
		}

		*((int*)&item->itemFlags[0]) = 0;
	}
}