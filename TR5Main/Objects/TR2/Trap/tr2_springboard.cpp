#include "framework.h"
#include "Objects/TR2/Trap/tr2_springboard.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

void SpringBoardControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->currentAnimState == 0 && LaraItem->pos.yPos == item->pos.yPos &&
		LaraItem->pos.xPos / SECTOR(1) == item->pos.xPos / SECTOR(1) &&
		LaraItem->pos.zPos / SECTOR(1) == item->pos.zPos / SECTOR(1))
	{
		if (LaraItem->hitPoints <= 0)
			return;

		if (LaraItem->currentAnimState == LS_WALK_BACK || LaraItem->currentAnimState == LS_RUN_BACK)
			LaraItem->speed = -LaraItem->speed;

		LaraItem->fallspeed = -240;
		LaraItem->gravityStatus = true;
		LaraItem->animNumber = LA_FALL_START;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LS_JUMP_FORWARD;
		LaraItem->goalAnimState = LS_JUMP_FORWARD;

		item->goalAnimState = 1;
	}

	AnimateItem(item);
}