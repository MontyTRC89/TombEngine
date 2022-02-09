#include "framework.h"
#include "tr4_plinthblade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void PlinthBladeControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (!TriggerActive(item))
		{
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
		else
		{
			int frameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				item->ItemFlags[3] = 0;
			else
				item->ItemFlags[3] = 200;

			AnimateItem(item);
		}
	}
}