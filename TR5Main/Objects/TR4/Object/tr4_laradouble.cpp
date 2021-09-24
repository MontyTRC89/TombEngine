#include "framework.h"
#include "tr4_laradouble.h"
#include "items.h"
#include "level.h"
#include "Sound/sound.h"
#include "control/box.h"
#include "lara.h"
#include "animation.h"
#include "item.h"
void InitialiseLaraDouble(short itemNum)
{
	ClearItem(itemNum);
}

void LaraDoubleControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->pos, 0);

	if (CreatureActive(itemNum))
	{
		if (item->hitStatus)
		{
			LaraItem->hitPoints += item->hitPoints - 1000;
		}

		item->hitPoints = 1000;

		AnimateItem(item);
	}
}