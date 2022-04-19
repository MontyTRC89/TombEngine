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

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
}

void SpinningBladeControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	bool spinning = false;

	if (item->currentAnimState == 2)
	{
		if (item->goalAnimState != 1)
		{
			int x = item->pos.xPos + WALL_SIZE * 3 * phd_sin(item->pos.yRot) / 2;
			int z = item->pos.zPos + WALL_SIZE * 3 * phd_cos(item->pos.yRot) / 2;

			short roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
			int height = GetFloorHeight(floor, x, item->pos.yPos, z);

			if (height == NO_HEIGHT)
				item->goalAnimState = 1;
		}

		spinning = true;

		if (item->touchBits)
		{
			LaraItem->hitStatus = true;
			LaraItem->hitPoints -= 100;

			DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - STEP_SIZE * 2, LaraItem->pos.zPos, (short)(item->speed * 2), LaraItem->pos.yRot, LaraItem->roomNumber, 2);
		}

		SoundEffect(231, &item->pos, 0);
	}
	else
	{
		if (TriggerActive(item))
			item->goalAnimState = 2;
		spinning = false;
	}

	AnimateItem(item);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	item->floor = item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (spinning && item->currentAnimState == 1)
		item->pos.yRot += -ANGLE(180);
}