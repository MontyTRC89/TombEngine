#include "framework.h"
#include "tr4_jean_yves.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	enum JeanYvesState
	{

	};

	enum JeanYvesAnim
	{

	};

	void InitialiseJeanYves(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* objectInfo = &Objects[item->ObjectNumber];

		item->Animation.TargetState = 1;
		item->Animation.ActiveState = 1;
		item->Animation.AnimNumber = objectInfo->animIndex;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	void JeanYvesControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags >= Lara.HighestLocation)
		{
			int state = 0;

			if (GetRandomControl() & 3)
				state = (GetRandomControl() & 1) + 1;
			else
				state = 3 * (GetRandomControl() & 1);

			item->Animation.TargetState = (((byte)(item->Animation.ActiveState) - 1) & 0xC) + state + 1;
			AnimateItem(item);
		}
		else
		{
			if (Lara.HighestLocation > 3)
				Lara.HighestLocation = 3;

			int state = (GetRandomControl() & 3) + 4 * Lara.HighestLocation;
			int animNumber = Objects[item->ObjectNumber].animIndex + state;
			state++;

			item->Animation.AnimNumber = animNumber;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = state;
			item->Animation.TargetState = state;
			item->TriggerFlags = Lara.HighestLocation;

			AnimateItem(item);
		}
	}
}
