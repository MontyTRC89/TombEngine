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

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->activeState = 1;
}

void KillerStatueControl(short itemNumber)
{
	ITEM_INFO* item;
	int x, y, z;
	short d;

	item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->activeState == 1)
		item->targetState = 2;
	else
		item->targetState = 1;

	if ((item->touchBits & 0x80) && item->activeState == 2)
	{
		LaraItem->hitStatus = 1;
		LaraItem->hitPoints -= 20;

		int x = LaraItem->pos.xPos + (GetRandomControl() - 16384) / 256;
		int z = LaraItem->pos.zPos + (GetRandomControl() - 16384) / 256;
		int y = LaraItem->pos.yPos - GetRandomControl() / 44;
		int d = (GetRandomControl() - 16384) / 8 + LaraItem->pos.yRot;
		DoBloodSplat(x, y, z, LaraItem->Velocity, d, LaraItem->roomNumber);
	}

	AnimateItem(item);
}