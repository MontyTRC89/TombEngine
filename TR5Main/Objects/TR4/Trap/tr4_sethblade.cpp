#include "framework.h"
#include "tr4_sethblade.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void InitialiseSethBlade(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		item->animNumber = Objects[item->objectNumber].animIndex + 1;
		item->goalAnimState = 2;
		item->currentAnimState = 2;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->itemFlags[2] = abs(item->triggerFlags);
	}

	void SethBladeControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		*((int*)&item->itemFlags) = 0;

		if (TriggerActive(item))
		{
			if (item->currentAnimState == 2)
			{
				if (item->itemFlags[2] > 1)
				{
					item->itemFlags[2]--;
				}
				else if (item->itemFlags[2] == 1)
				{
					item->goalAnimState = 1;
					item->itemFlags[2] = 0;
				}
				else if (item->itemFlags[2] == 0)
				{
					if (item->triggerFlags > 0)
					{
						item->itemFlags[2] = item->triggerFlags;
					}
				}
			}
			else
			{
				int frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

				if (frameNumber >= 0 && frameNumber <= 6)
				{
					*((int*)&item->itemFlags) = -1;
					item->itemFlags[3] = 1000;
				}
				else if (frameNumber >= 7 && frameNumber <= 15)
				{
					*((int*)&item->itemFlags) = 448;
					item->itemFlags[3] = 1000;
				}
				else
				{
					*((int*)&item->itemFlags) = 0;
					item->itemFlags[3] = 1000;
				}
			}

			AnimateItem(item);
		}
	}
}