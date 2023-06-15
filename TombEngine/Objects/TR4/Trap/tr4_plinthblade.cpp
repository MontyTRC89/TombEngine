#include "framework.h"
#include "tr4_plinthblade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void PlinthBladeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
		{
			item->Animation.FrameNumber = 0;
		}
		else
		{
			if (TestLastFrame(item))
			{
				item->ItemFlags[3] = 0;
			}
			else
			{
				item->ItemFlags[3] = 200;
			}

			AnimateItem(item);
		}
	}
}
