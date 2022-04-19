#include "framework.h"
#include "tr4_laradouble.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/control/box.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"

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
			LaraItem->hitPoints = item->hitPoints;
		}

		AnimateItem(item);
	}
}