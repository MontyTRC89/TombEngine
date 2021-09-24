#include "framework.h"
#include "tr4_plough.h"
#include "level.h"
#include "control/control.h"
#include "animation.h"
#include "item.h"
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