#include "framework.h"
#include "tr4_laradouble.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/control/box.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"

void InitialiseLaraDouble(short itemNumber)
{
	ClearItem(itemNumber);
}

void LaraDoubleControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Position, 0);

	if (CreatureActive(itemNumber))
	{
		if (item->HitStatus)
			LaraItem->HitPoints = item->HitPoints;

		AnimateItem(item);
	}
}
