#include "framework.h"
#include "tr4_spikeball.h"
#include "level.h"
#include "control.h"

namespace TEN::Entities::TR4
{
	void SpikeballControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (TriggerActive(item))
		{
			short frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

			if ((frameNumber <= 14 || frameNumber >= 24) && (frameNumber < 138 || frameNumber > 140))
			{
				if (frameNumber < 141)
					*item->itemFlags = 0;
				else
				{
					item->itemFlags[3] = 50;
					*item->itemFlags = 0x7FF800;
				}
			}
			else
			{
				item->itemFlags[3] = 150;
				*item->itemFlags = 0x7FF800;
			}

			AnimateItem(item);
		}
		else
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			*item->itemFlags = 0;
		}
	}
}
