#include "framework.h"
#include "tr4_jeanyves.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

void InitialiseJeanYves(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];
	
	item->TargetState = 1;
	item->ActiveState = 1;
	item->AnimNumber = obj->animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
}

void JeanYvesControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags >= Lara.highestLocation)
	{
		short state = 0;

		if (GetRandomControl() & 3)
			state = (GetRandomControl() & 1) + 1;
		else
			state = 3 * (GetRandomControl() & 1);

		item->TargetState = (((byte)(item->ActiveState) - 1) & 0xC) + state + 1;
		AnimateItem(item);
	}
	else
	{
		if (Lara.highestLocation > 3)
			Lara.highestLocation = 3;

		short state = (GetRandomControl() & 3) + 4 * Lara.highestLocation;
		short animNumber = Objects[item->ObjectNumber].animIndex + state;
		state++;

		item->TargetState = item->ActiveState = state;
		item->AnimNumber = animNumber;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TriggerFlags = Lara.highestLocation;

		AnimateItem(item);
	}
}