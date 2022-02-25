#include "framework.h"
#include "tr5_willowwisp.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"

void InitialiseLightingGuide(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 1;
	item->ActiveState = 1;
}
