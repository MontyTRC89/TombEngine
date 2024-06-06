#include "framework.h"
#include "tr4_plough.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::Traps
{
	void InitializePlough(short itemNumber)
	{
		auto & item = g_Level.Items[itemNumber];

		item.ItemFlags[4] = 1;
	}

	void PloughControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[3] = 50;

		if (TriggerActive(&item))
		{
			*((int*)&item.ItemFlags) = 0x3F000;
			AnimateItem(&item);
		}
		else
			*((int*)&item.ItemFlags) = 0;
	}
}
