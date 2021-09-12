#include "framework.h"
#include "tr4_chain.h"
#include "level.h"
#include "control.h"

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
				item->itemFlags[0] = 30846;
				AnimateItem(item);
				return;
			}
		}
		else
		{
			item->itemFlags[3] = 25;

			if (TriggerActive(item))
			{
				item->itemFlags[0] = 1920;
				AnimateItem(item);
				return;
			}
		}

		item->itemFlags[0] = 0;
	}
}