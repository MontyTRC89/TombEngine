#include "framework.h"
#include "Objects/TR5/Trap/tr5_fallingceiling.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

void FallingCeilingControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Animation.ActiveState)
	{
		if (item->Animation.ActiveState == 1 && item->TouchBits)
			DoDamage(LaraItem, 300);
	}
	else
	{
		item->Animation.TargetState = 1;
		item->Animation.IsAirborne = true;;
	}

	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
		RemoveActiveItem(itemNumber);
	else
	{
		auto probe = GetCollision(item);

		item->Floor = probe.Position.Floor;

		if (probe.RoomNumber != item->RoomNumber)
			ItemNewRoom(itemNumber, probe.RoomNumber);

		if (item->Animation.ActiveState == 1)
		{
			if (item->Pose.Position.y >= item->Floor)
			{
				item->Pose.Position.y = item->Floor;
				item->Animation.TargetState = 2;
				item->Animation.IsAirborne = false;
				item->Animation.Velocity.y = 0.0f;
			}
		}
	}
}
