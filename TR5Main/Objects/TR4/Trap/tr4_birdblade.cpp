#include "framework.h"
#include "tr4_birdblade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void BirdBladeControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		item->itemFlags[3] = 100;

		if (!TriggerActive(item))
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			*((int*)&item->itemFlags[0]) = 0;
		}
		else
		{
			int frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

			if (frameNumber <= 14 || frameNumber >= 31)
				*((int*)&item->itemFlags[0]) = 0;
			else
				*((int*)&item->itemFlags[0]) = 6;

			AnimateItem(item);
		}
	}
}