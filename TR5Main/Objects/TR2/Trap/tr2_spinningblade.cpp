#include "framework.h"
#include "Objects/TR2/Trap/tr2_spinningblade.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseSpinningBlade(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = 1;
}

void SpinningBladeControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	bool spinning = false;

	if (item->ActiveState == 2)
	{
		if (item->TargetState != 1)
		{
			int x = item->Position.xPos + WALL_SIZE * 3 * phd_sin(item->Position.yRot) / 2;
			int z = item->Position.zPos + WALL_SIZE * 3 * phd_cos(item->Position.yRot) / 2;

			short roomNumber = item->RoomNumber;
			FLOOR_INFO* floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
			int height = GetFloorHeight(floor, x, item->Position.yPos, z);

			if (height == NO_HEIGHT)
				item->TargetState = 1;
		}

		spinning = true;

		if (item->TouchBits)
		{
			LaraItem->HitStatus = true;
			LaraItem->HitPoints -= 100;

			DoLotsOfBlood(LaraItem->Position.xPos, LaraItem->Position.yPos - STEP_SIZE * 2, LaraItem->Position.zPos, (short)(item->Velocity * 2), LaraItem->Position.yRot, LaraItem->RoomNumber, 2);
		}

		SoundEffect(231, &item->Position, 0);
	}
	else
	{
		if (TriggerActive(item))
			item->TargetState = 2;
		spinning = false;
	}

	AnimateItem(item);

	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
	item->Floor = item->Position.yPos = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (spinning && item->ActiveState == 1)
		item->Position.yRot += -ANGLE(180);
}