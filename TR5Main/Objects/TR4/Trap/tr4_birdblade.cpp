#include "framework.h"
#include "tr4_birdblade.h"
#include "level.h"
#include "control.h"
#include "animation.h"

void BirdBladeControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	item->itemFlags[3] = 100;
	if (!TriggerActive(item))
	{
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		short frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

		if (frameNumber <= 14 || frameNumber >= 31)
			item->itemFlags[0] = 0;
		else
			item->itemFlags[0] = 6;

		AnimateItem(item);
	}
}