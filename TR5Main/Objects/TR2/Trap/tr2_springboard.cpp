#include "framework.h"
#include "tr2_springboard.h"
#include "level.h"
#include "lara.h"
#include "control.h"

void SpringBoardControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->currentAnimState == 0 && LaraItem->pos.yPos == item->pos.yPos &&
		(LaraItem->pos.xPos >> WALL_SHIFT) == (item->pos.xPos >> WALL_SHIFT) &&
		(LaraItem->pos.zPos >> WALL_SHIFT) == (item->pos.zPos >> WALL_SHIFT))
	{
		if (LaraItem->hitPoints <= 0)
			return;

		if (LaraItem->currentAnimState == STATE_LARA_WALK_BACK || LaraItem->currentAnimState == STATE_LARA_RUN_BACK)
			LaraItem->speed = -LaraItem->speed;

		LaraItem->fallspeed = -240;
		LaraItem->gravityStatus = true;
		LaraItem->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = STATE_LARA_JUMP_FORWARD;
		LaraItem->goalAnimState = STATE_LARA_JUMP_FORWARD;

		item->goalAnimState = 1;
	}

	AnimateItem(item);
}