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
			int x = item->Pose.Position.x + SECTOR(3) * sin(item->Pose.Orientation.GetY()) / 2;
			int z = item->Pose.Position.z + SECTOR(3) * cos(item->Pose.Orientation.GetY()) / 2;

			int height = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).Position.Floor;
			if (height == NO_HEIGHT)
				item->Animation.TargetState = 1;
		}

		spinning = true;

		if (item->TouchBits)
		{
			LaraItem->HitStatus = true;
			LaraItem->HitPoints -= 100;

			DoLotsOfBlood(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y - CLICK(2), LaraItem->Pose.Position.z, item->Animation.Velocity * 2, LaraItem->Pose.Orientation.GetY(), LaraItem->RoomNumber, 2);
		}

		SoundEffect(231, &item->Pose, 0);
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
		item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + Angle::DegToRad(-180.0f));
}
