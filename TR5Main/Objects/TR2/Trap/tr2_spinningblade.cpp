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
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
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
			int x = item->Position.xPos + SECTOR(3) * phd_sin(item->Position.yRot) / 2;
			int z = item->Position.zPos + SECTOR(3) * phd_cos(item->Position.yRot) / 2;

			int height = GetCollision(x, item->Position.yPos, z, item->RoomNumber).Position.Floor;
			if (height == NO_HEIGHT)
				item->Animation.TargetState = 1;
		}

		spinning = true;

		if (item->TouchBits)
		{
			LaraItem->HitStatus = true;
			LaraItem->HitPoints -= 100;

			DoLotsOfBlood(LaraItem->Position.xPos, LaraItem->Position.yPos - CLICK(2), LaraItem->Position.zPos, (short)(item->Animation.Velocity * 2), LaraItem->Position.yRot, LaraItem->RoomNumber, 2);
		}

		SoundEffect(231, &item->Position, 0);
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
	item->Position.yPos = probe.Position.Floor;

	if (probe.RoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	if (spinning && item->Animation.ActiveState == 1)
		item->Position.yRot += -ANGLE(180.0f);
}
