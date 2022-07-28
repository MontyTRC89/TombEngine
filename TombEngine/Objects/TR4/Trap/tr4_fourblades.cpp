#include "framework.h"
#include "tr4_fourblades.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void FourBladesControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
		{
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			*((int*)&item->ItemFlags[0]) = 0;
		}
		else
		{
			int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
			if (frameNumber <= 5 ||
				frameNumber >= 58 ||
				frameNumber >= 8 && frameNumber <= 54)
			{
				*((int*)&item->ItemFlags[0]) = 0;
			}
			else
			{
				if (frameNumber >= 6 && frameNumber <= 7)
				{
					item->ItemFlags[3] = 20;
					*((int*)&item->ItemFlags[0]) = 30;
				}
				else if (frameNumber >= 55 && frameNumber <= 57)
				{
					item->ItemFlags[3] = 200;
					*((int*)&item->ItemFlags[0]) = 30;
				}
			}

			AnimateItem(item);
		}
	}
}
