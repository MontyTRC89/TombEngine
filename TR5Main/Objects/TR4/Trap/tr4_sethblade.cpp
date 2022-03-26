#include "framework.h"
#include "tr4_sethblade.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void InitialiseSethBlade(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
		item->Animation.TargetState = 2;
		item->Animation.ActiveState = 2;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
		item->ItemFlags[2] = abs(item->TriggerFlags);
	}

	void SethBladeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		*((int*)&item->ItemFlags) = 0;

		if (TriggerActive(item))
		{
			if (item->Animation.ActiveState == 2)
			{
				if (item->ItemFlags[2] > 1)
					item->ItemFlags[2]--;
				else if (item->ItemFlags[2] == 1)
				{
					item->Animation.TargetState = 1;
					item->ItemFlags[2] = 0;
				}
				else if (item->ItemFlags[2] == 0)
				{
					if (item->TriggerFlags > 0)
						item->ItemFlags[2] = item->TriggerFlags;
				}
			}
			else
			{
				int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].FrameBase;

				if (frameNumber >= 0 && frameNumber <= 6)
				{
					*((int*)&item->ItemFlags) = -1;
					item->ItemFlags[3] = 1000;
				}
				else if (frameNumber >= 7 && frameNumber <= 15)
				{
					*((int*)&item->ItemFlags) = 448;
					item->ItemFlags[3] = 1000;
				}
				else
				{
					*((int*)&item->ItemFlags) = 0;
					item->ItemFlags[3] = 1000;
				}
			}

			AnimateItem(item);
		}
	}
}
