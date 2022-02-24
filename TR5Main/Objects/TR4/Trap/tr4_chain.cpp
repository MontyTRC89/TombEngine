#include "framework.h"
#include "tr4_chain.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void ChainControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags)
		{
			item->ItemFlags[2] = 1;
			item->ItemFlags[3] = 75;

			if (TriggerActive(item))
			{
				*((int*)&item->ItemFlags[0]) = 0x787E;
				AnimateItem(item);
				return;
			}
		}
		else
		{
			item->ItemFlags[3] = 25;

			if (TriggerActive(item))
			{
				*((int*)&item->ItemFlags[0]) = 0x780;
				AnimateItem(item);
				return;
			}
		}

		*((int*)&item->ItemFlags[0]) = 0;
	}
}
