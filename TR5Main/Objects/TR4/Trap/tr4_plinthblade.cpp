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
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
		else
		{
			int frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
				item->itemFlags[3] = 0;
			else
				item->itemFlags[3] = 200;

			AnimateItem(item);
		}
	}
}