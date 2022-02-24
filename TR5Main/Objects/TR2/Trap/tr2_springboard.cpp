#include "framework.h"
#include "Objects/TR2/Trap/tr2_springboard.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

void SpringBoardControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ActiveState == 0 && LaraItem->Position.yPos == item->Position.yPos &&
		LaraItem->Position.xPos / SECTOR(1) == item->Position.xPos / SECTOR(1) &&
		LaraItem->Position.zPos / SECTOR(1) == item->Position.zPos / SECTOR(1))
	{
		if (LaraItem->HitPoints <= 0)
			return;

		if (LaraItem->ActiveState == LS_WALK_BACK || LaraItem->ActiveState == LS_RUN_BACK)
			LaraItem->Velocity = -LaraItem->Velocity;

		LaraItem->VerticalVelocity = -240;
		LaraItem->Airborne = true;
		LaraItem->AnimNumber = LA_FALL_START;
		LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
		LaraItem->ActiveState = LS_JUMP_FORWARD;
		LaraItem->TargetState = LS_JUMP_FORWARD;

		item->TargetState = 1;
	}

	AnimateItem(item);
}
