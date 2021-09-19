#include "framework.h"
#include "tr4_catwalkblade.h"
#include "level.h"
#include "control/control.h"
#include "animation.h"

void CatwalkBladeControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		short frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

		if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd || frameNumber < 38)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 100;

		AnimateItem(item);
	}
}