#include "framework.h"
#include "tr4_lara_double.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/control/box.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"

namespace TEN::Entities::TR4
{
	void InitialiseLaraDouble(short itemNumber)
	{
		ClearItem(itemNumber);
	}

	void LaraDoubleControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Pose, 0);

		if (CreatureActive(itemNumber))
		{
			if (item->HitStatus)
				LaraItem->HitPoints = item->HitPoints;

			AnimateItem(item);
		}
	}
}
