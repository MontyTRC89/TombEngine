#include "framework.h"
#include "tr4_birdblade.h"
#include "level.h"
#include "control.h"

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
			short frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

			if (frameNumber <= 14 || frameNumber >= 31)
				*((int*)&item->itemFlags[0]) = 0;
			else
				*((int*)&item->itemFlags[0]) = 6;

			AnimateItem(item);
		}
	}
}