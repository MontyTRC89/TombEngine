#include "framework.h"
#include "Objects/TR4/Trap/tr4_spikywall.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

void InitializeSpikyWall(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	item.ItemFlags[0] = item.TriggerFlags;
}

void ControlSpikyWall(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	// Move wall.
	if (TriggerActive(&item) && item.Status != ITEM_DEACTIVATED)
	{
		auto pos = Geometry::TranslatePoint(item.Pose.Position, item.Pose.Orientation.y, item.ItemFlags[0]);
		auto pointColl = GetCollision(pos.x, pos.y, pos.z, item.RoomNumber);

		if (pointColl.Position.Floor != item.Pose.Position.y)
		{
			item.Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		else
		{
			item.Pose.Position.x = pos.x;
			item.Pose.Position.z = pos.z;

			if (pointColl.RoomNumber != item.RoomNumber)
				ItemNewRoom(itemNumber, pointColl.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item.Pose);
		}
	}

	if (item.TouchBits.TestAny())
	{
		DoDamage(LaraItem, 15);
		DoLotsOfBlood(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y - CLICK(2), LaraItem->Pose.Position.z, 4, item.Pose.Orientation.y, LaraItem->RoomNumber, 3);
		item.TouchBits.ClearAll();

		SoundEffect(SFX_TR4_LARA_GRABFEET, &item.Pose);
	}
}
