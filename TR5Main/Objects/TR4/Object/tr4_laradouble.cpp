#include "framework.h"
#include "tr4_laradouble.h"
#include "items.h"
#include "level.h"
#include "sound.h"
#include "box.h"
#include "lara.h"

void InitialiseLaraDouble(short itemNum)
{
	ClearItem(itemNum);
}

void LaraDoubleControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP, &item->pos, 0);

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