#include "framework.h"
#include "Objects/TR2/Trap/tr2_killerstatue.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseKillerStatue(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = 1;
}

void KillerStatueControl(short itemNumber)
{
	ITEM_INFO* item;
	int x, y, z;
	short d;

	item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->ActiveState == 1)
		item->TargetState = 2;
	else
		item->TargetState = 1;

	if ((item->TouchBits & 0x80) && item->ActiveState == 2)
	{
		LaraItem->HitStatus = 1;
		LaraItem->HitPoints -= 20;

		int x = LaraItem->Position.xPos + (GetRandomControl() - 16384) / 256;
		int z = LaraItem->Position.zPos + (GetRandomControl() - 16384) / 256;
		int y = LaraItem->Position.yPos - GetRandomControl() / 44;
		int d = (GetRandomControl() - 16384) / 8 + LaraItem->Position.yRot;
		DoBloodSplat(x, y, z, LaraItem->Velocity, d, LaraItem->RoomNumber);
	}

	AnimateItem(item);
}