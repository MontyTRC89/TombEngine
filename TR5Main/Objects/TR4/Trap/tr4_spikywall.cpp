#include "framework.h"
#include "tr4_spikywall.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/effects/effects.h"

void ControlSpikyWall(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	// Move wall.
	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED)
	{
		int x = item->Position.xPos + phd_sin(item->Position.yRot);
		int z = item->Position.zPos + phd_cos(item->Position.yRot);
		auto probe = GetCollisionResult(x, item->Position.yPos, z, item->RoomNumber);

		if (probe.Position.Floor != item->Position.yPos)
		{
			item->Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		else
		{
			item->Position.xPos = x;
			item->Position.zPos = z;

			if (probe.RoomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item->Position, 0);
		}
	}

	if (item->TouchBits)
	{
		LaraItem->HitPoints -= 15;
		LaraItem->HitStatus = true;

		DoLotsOfBlood(LaraItem->Position.xPos, LaraItem->Position.yPos - CLICK(2), LaraItem->Position.zPos, 4, item->Position.yRot, LaraItem->RoomNumber, 3);
		item->TouchBits = 0;

		SoundEffect(56, &item->Position, 0);
	}
}
