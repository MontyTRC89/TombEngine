#include "framework.h"
#include "tr5_willowwisp.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"

void InitialiseLightingGuide(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 1;
	item->Animation.ActiveState = 1;
}
