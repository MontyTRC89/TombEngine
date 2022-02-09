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

	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED)
	{
		int y = item->Position.yPos + ((item->ItemFlags[0] == 1) ? 10 : 5);

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);

		if (GetFloorHeight(floor, item->Position.xPos, y, item->Position.zPos) < y + 1024)
		{
			item->Status = ITEM_DEACTIVATED;
			StopSoundEffect(147);
		}
		else
		{
			item->Position.yPos = y;

			if (roomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, roomNumber);

			SoundEffect(147, &item->Position, 0);
		}
	}

	if (item->TouchBits)
	{
		LaraItem->HitPoints -= 20;
		LaraItem->HitStatus = true;

		DoLotsOfBlood(LaraItem->Position.xPos, item->Position.yPos + 768, LaraItem->Position.zPos, 4, item->Position.yRot, LaraItem->RoomNumber, 3);
		item->TouchBits = 0;

		SoundEffect(56, &item->Position, 0);
	}

	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED && item->ItemFlags[0] == 1)
		AnimateItem(item);
}