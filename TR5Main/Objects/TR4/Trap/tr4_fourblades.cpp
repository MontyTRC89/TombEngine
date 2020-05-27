#include "framework.h"
#include "tr4_fourblades.h"
#include "level.h"
#include "control.h"

void FourBladesControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	short frameNumber;

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;
		if (frameNumber <= 5 || frameNumber >= 58 || frameNumber >= 8 && frameNumber <= 54)
		{
			item->itemFlags[0] = 0;
		}
		else
		{
			if (frameNumber >= 6 && frameNumber <= 7)
			{
				item->itemFlags[3] = 20;
				item->itemFlags[0] = 30;
			}
			else
			{
				if (frameNumber >= 55 && frameNumber <= 57)
				{
					item->itemFlags[3] = 200;
					item->itemFlags[0] = 30;
				}
			}
		}

		AnimateItem(item);
	}
}