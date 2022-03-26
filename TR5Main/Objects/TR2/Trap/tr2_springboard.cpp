#include "framework.h"
#include "Objects/TR2/Trap/tr2_springboard.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

void SpringBoardControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Animation.ActiveState == 0 && LaraItem->Position.yPos == item->Position.yPos &&
		LaraItem->Position.xPos / SECTOR(1) == item->Position.xPos / SECTOR(1) &&
		LaraItem->Position.zPos / SECTOR(1) == item->Position.zPos / SECTOR(1))
	{
		if (LaraItem->HitPoints <= 0)
			return;

		if (LaraItem->Animation.ActiveState == LS_WALK_BACK || LaraItem->Animation.ActiveState == LS_RUN_BACK)
			LaraItem->Animation.Velocity = -LaraItem->Animation.Velocity;

		LaraItem->Animation.VerticalVelocity = -240;
		LaraItem->Animation.Airborne = true;
		LaraItem->Animation.AnimNumber = LA_FALL_START;
		LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].FrameBase;
		LaraItem->Animation.ActiveState = LS_JUMP_FORWARD;
		LaraItem->Animation.TargetState = LS_JUMP_FORWARD;

		item->Animation.TargetState = 1;
	}

	AnimateItem(item);
}
