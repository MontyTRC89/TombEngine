#include "framework.h"
#include "tr4_spikyceiling.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"

void ControlSpikyCeiling(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED)
	{
		int y = item->Position.yPos + ((item->ItemFlags[0] == 1) ? 10 : 5);
		auto probe = GetCollisionResult(item->Position.xPos, y, item->Position.zPos, item->RoomNumber);

		if (probe.Position.Floor < (y + SECTOR(1)))
		{
			item->Status = ITEM_DEACTIVATED;
			StopSoundEffect(147);
		}
		else
		{
			item->Position.yPos = y;

			if (probe.RoomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			SoundEffect(147, &item->Position, 0);
		}
	}

	if (item->TouchBits)
	{
		LaraItem->HitPoints -= 20;
		LaraItem->HitStatus = true;

		DoLotsOfBlood(LaraItem->Position.xPos, item->Position.yPos + CLICK(3), LaraItem->Position.zPos, 4, item->Position.yRot, LaraItem->RoomNumber, 3);
		item->TouchBits = 0;

		SoundEffect(56, &item->Position, 0);
	}

	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED && item->ItemFlags[0] == 1)
		AnimateItem(item);
}
