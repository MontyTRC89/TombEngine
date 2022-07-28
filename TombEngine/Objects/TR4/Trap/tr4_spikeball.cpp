#include "framework.h"
#include "tr4_spikeball.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void SpikeballControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			if ((frameNumber <= 14 || frameNumber >= 24) &&
				(frameNumber < 138 || frameNumber > 140))
			{
				if (frameNumber < 141)
					*((int*)&item->ItemFlags[0]) = 0;
				else
				{
					item->ItemFlags[3] = 50;
					*((int*)&item->ItemFlags[0]) = 0x7FF800;
				}
			}
			else
			{
				item->ItemFlags[3] = 150;
				*((int*)&item->ItemFlags[0]) = 0x7FF800;
			}

			AnimateItem(item);
		}
		else
		{
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			*((int*)&item->ItemFlags[0]) = 0;
		}
	}
}
