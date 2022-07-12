#include "framework.h"
#include "Objects/TR2/Trap/tr2_springboard.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

void SpringBoardControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Animation.ActiveState == 0 && LaraItem->Pose.Position.y == item->Pose.Position.y &&
		LaraItem->Pose.Position.x / SECTOR(1) == item->Pose.Position.x / SECTOR(1) &&
		LaraItem->Pose.Position.z / SECTOR(1) == item->Pose.Position.z / SECTOR(1))
	{
		if (LaraItem->HitPoints <= 0)
			return;

		if (LaraItem->Animation.ActiveState == LS_WALK_BACK || LaraItem->Animation.ActiveState == LS_RUN_BACK)
			LaraItem->Animation.Velocity = -LaraItem->Animation.Velocity;

		LaraItem->Animation.AnimNumber = LA_FALL_START;
		LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
		LaraItem->Animation.ActiveState = LS_JUMP_FORWARD;
		LaraItem->Animation.TargetState = LS_JUMP_FORWARD;
		LaraItem->Animation.IsAirborne = true;
		LaraItem->Animation.VerticalVelocity = -240;

		item->Animation.TargetState = 1;
	}

	AnimateItem(item);
}
