#include "framework.h"
#include "tr4_spikywall.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/effects/effects.h"

void ControlSpikyWall(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	/* Move wall */
	if (TriggerActive(item) && item->status != ITEM_DEACTIVATED)
	{
		int x = item->pos.xPos + phd_sin(item->pos.yRot);
		int z = item->pos.zPos + phd_cos(item->pos.yRot);

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);

		if (GetFloorHeight(floor, x, item->pos.yPos, z) != item->pos.yPos)
		{
			item->status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		else
		{
			item->pos.xPos = x;
			item->pos.zPos = z;
			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNum, roomNumber);
			SoundEffect(SFX_TR4_ROLLING_BALL, &item->pos, 0);
		}
	}

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 15;
		LaraItem->hitStatus = true;

		DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - 512, LaraItem->pos.zPos, 4, item->pos.yRot, LaraItem->roomNumber, 3);
		item->touchBits = 0;

		SoundEffect(56, &item->pos, 0);
	}
}