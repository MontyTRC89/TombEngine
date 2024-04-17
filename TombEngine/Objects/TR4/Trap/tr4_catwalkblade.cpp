#include "framework.h"
#include "tr4_catwalkblade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/Animation/Animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void CatwalkBladeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
		{
			item->Animation.FrameNumber = 0;
		}
		else
		{
			int frameNumber = item->Animation.FrameNumber;

			if (TestLastFrame(item) || frameNumber < 38)
			{
				item->ItemFlags[3] = 0;
			}
			else
			{
				item->ItemFlags[3] = 100;
			}

			AnimateItem(item);
		}
	}
}
