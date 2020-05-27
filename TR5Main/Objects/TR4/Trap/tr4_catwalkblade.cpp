#include "framework.h"
#include "tr4_catwalkblade.h"
#include "level.h"
#include "control.h"

void CatwalkBladeControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		short frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd || frameNumber < 38)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 100;

		AnimateItem(item);
	}
}