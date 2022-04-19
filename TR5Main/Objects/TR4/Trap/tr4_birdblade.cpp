#include "framework.h"
#include "tr4_birdblade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void BirdBladeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->ItemFlags[3] = 100;

		if (!TriggerActive(item))
		{
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			*((int*)&item->ItemFlags[0]) = 0;
		}
		else
		{
			int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			if (frameNumber <= 14 || frameNumber >= 31)
				*((int*)&item->ItemFlags[0]) = 0;
			else
				*((int*)&item->ItemFlags[0]) = 6;

			AnimateItem(item);
		}
	}
}
