#include "framework.h"
#include "tr4_fourblades.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void FourBladesControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (!TriggerActive(item))
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			*((int*)&item->itemFlags[0]) = 0;
		}
		else
		{
			int frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
			if (frameNumber <= 5 || frameNumber >= 58 || frameNumber >= 8 && frameNumber <= 54)
			{
				*((int*)&item->itemFlags[0]) = 0;
			}
			else
			{
				if (frameNumber >= 6 && frameNumber <= 7)
				{
					item->itemFlags[3] = 20;
					*((int*)&item->itemFlags[0]) = 30;
				}
				else if (frameNumber >= 55 && frameNumber <= 57)
				{
					item->itemFlags[3] = 200;
					*((int*)&item->itemFlags[0]) = 30;
				}
			}

			AnimateItem(item);
		}
	}
}