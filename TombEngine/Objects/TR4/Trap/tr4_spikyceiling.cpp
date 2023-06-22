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

void InitializeSpikyCeiling(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	item.ItemFlags[0] = item.TriggerFlags;
}

void ControlSpikyCeiling(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	// Move wall.
	if (TriggerActive(&item) && item.Status != ITEM_DEACTIVATED)
	{
		auto pos = Geometry::TranslatePoint(item.Pose.Position, item.Pose.Orientation.y, 0.0f, item.ItemFlags[0]);
		auto pointColl = GetCollision(pos.x, pos.y, pos.z, item.RoomNumber);

		if ((item.ItemFlags[0] > 0 && pointColl.Position.Floor < (pos.y + SECTOR(1))) ||
			(item.ItemFlags[0] < 0 && pointColl.Position.Ceiling > (pos.y )))
		{
			item.Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		else
		{
			item.Pose.Position.y = pos.y;

			if (pointColl.RoomNumber != item.RoomNumber)
				ItemNewRoom(itemNumber, pointColl.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item.Pose);
		}
	}

	if (item.TouchBits.TestAny())
	{
		DoDamage(LaraItem, 15);
		DoLotsOfBlood(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y + CLICK(3), LaraItem->Pose.Position.z, 4, item.Pose.Orientation.y, LaraItem->RoomNumber, 3);
		item.TouchBits.ClearAll();

		SoundEffect(SFX_TR4_LARA_GRABFEET, &item.Pose);
	}
}
