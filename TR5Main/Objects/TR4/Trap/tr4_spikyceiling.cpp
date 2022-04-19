#include "framework.h"
#include "tr4_spikyceiling.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"

void ControlSpikyCeiling(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->status != ITEM_DEACTIVATED)
	{
		int y = item->pos.yPos + ((item->itemFlags[0] == 1) ? 10 : 5);

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);

		if (GetFloorHeight(floor, item->pos.xPos, y, item->pos.zPos) < y + 1024)
		{
			item->status = ITEM_DEACTIVATED;
			StopSoundEffect(147);
		}
		else
		{
			item->pos.yPos = y;

			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNumber, roomNumber);

			SoundEffect(147, &item->pos, 0);
		}
	}

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 20;
		LaraItem->hitStatus = true;

		DoLotsOfBlood(LaraItem->pos.xPos, item->pos.yPos + 768, LaraItem->pos.zPos, 4, item->pos.yRot, LaraItem->roomNumber, 3);
		item->touchBits = 0;

		SoundEffect(56, &item->pos, 0);
	}

	if (TriggerActive(item) && item->status != ITEM_DEACTIVATED && item->itemFlags[0] == 1)
		AnimateItem(item);
}