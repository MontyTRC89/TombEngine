#include "framework.h"
#include "tr5_fallingceiling.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/control/control.h"
#include "Game/animation.h"

void FallingCeilingControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ActiveState)
	{
		if (item->ActiveState == 1 && item->TouchBits)
		{
			LaraItem->HitPoints -= 300;
			LaraItem->HitStatus = true;
		}
	}
	else
	{
		item->TargetState = 1;
		item->Airborne = true;;
	}

	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
		RemoveActiveItem(itemNumber);
	else
	{
		auto probe = GetCollisionResult(item);

		item->Floor = probe.Position.Floor;

		if (probe.RoomNumber != item->RoomNumber)
			ItemNewRoom(itemNumber, probe.RoomNumber);

		if (item->ActiveState == 1)
		{
			if (item->Position.yPos >= item->Floor)
			{
				item->Position.yPos = item->Floor;
				item->Airborne = false;
				item->TargetState = 2;
				item->VerticalVelocity = 0;
			}
		}
	}
}
