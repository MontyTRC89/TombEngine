#include "framework.h"
#include "Objects/TR2/Trap/tr2_spinningblade.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseSpinningBlade(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = 1;
}

void SpinningBladeControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	bool spinning = false;

	if (item->Animation.ActiveState == 2)
	{
		if (item->Animation.TargetState != 1)
		{
			int x = item->Pose.Position.x + SECTOR(3) * phd_sin(item->Pose.Orientation.y) / 2;
			int z = item->Pose.Position.z + SECTOR(3) * phd_cos(item->Pose.Orientation.y) / 2;

			int height = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).Position.Floor;
			if (height == NO_HEIGHT)
				item->Animation.TargetState = 1;
		}

		spinning = true;

		if (item->TouchBits)
		{
			DoDamage(LaraItem, 100);
			DoLotsOfBlood(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y - CLICK(2), LaraItem->Pose.Position.z, (short)(item->Animation.Velocity * 2), LaraItem->Pose.Orientation.y, LaraItem->RoomNumber, 2);
		}

		SoundEffect(SFX_TR2_ROLLING_BLADE, &item->Pose);
	}
	else
	{
		if (TriggerActive(item))
			item->Animation.TargetState = 2;

		spinning = false;
	}

	AnimateItem(item);

	auto probe = GetCollision(item);

	item->Floor = probe.Position.Floor;
	item->Pose.Position.y = probe.Position.Floor;

	if (probe.RoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	if (spinning && item->Animation.ActiveState == 1)
		item->Pose.Orientation.y += -ANGLE(180.0f);
}
