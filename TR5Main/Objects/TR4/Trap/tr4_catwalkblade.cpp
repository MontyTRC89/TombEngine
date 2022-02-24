#include "framework.h"
#include "tr4_catwalkblade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void CatwalkBladeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		else
		{
			int frameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd || frameNumber < 38)
				item->ItemFlags[3] = 0;
			else
				item->ItemFlags[3] = 100;

			AnimateItem(item);
		}
	}
}
