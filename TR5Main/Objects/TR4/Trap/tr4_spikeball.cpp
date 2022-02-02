#include "framework.h"
#include "tr4_spikeball.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void SpikeballControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (TriggerActive(item))
		{
			int frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

			if ((frameNumber <= 14 || frameNumber >= 24) && (frameNumber < 138 || frameNumber > 140))
			{
				if (frameNumber < 141)
					*((int*)&item->itemFlags[0]) = 0;
				else
				{
					item->itemFlags[3] = 50;
					*((int*)&item->itemFlags[0]) = 0x7FF800;
				}
			}
			else
			{
				item->itemFlags[3] = 150;
				*((int*)&item->itemFlags[0]) = 0x7FF800;
			}

			AnimateItem(item);
		}
		else
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			*((int*)&item->itemFlags[0]) = 0;
		}
	}
}
