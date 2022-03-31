#include "framework.h"
#include "tr4_blade.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/Lara/lara.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	void BladeCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
	{
		auto* bladeItem = &g_Level.Items[itemNumber];

		if (bladeItem->Status == ITEM_INVISIBLE)
			return;

		if (bladeItem->ItemFlags[3])
		{
			if (TestBoundsCollide(bladeItem, laraItem, coll->Setup.Radius))
			{
				int oldX = laraItem->Position.xPos;
				int oldY = laraItem->Position.yPos;
				int oldZ = laraItem->Position.zPos;

				int dx = 0;
				int dy = 0;
				int dz = 0;

				if (ItemPushItem(bladeItem, laraItem, coll, 1, 1))
				{
					laraItem->HitPoints -= bladeItem->ItemFlags[3];

					dx = oldX - laraItem->Position.xPos;
					dy = oldY - laraItem->Position.yPos;
					dz = oldZ - laraItem->Position.zPos;

					if ((dx || dy || dz) && TriggerActive(bladeItem))
					{
						DoBloodSplat(
							(GetRandomControl() & 0x3F) + laraItem->Position.xPos - 32,
							laraItem->Position.yPos - (GetRandomControl() & 0x1FF) - 256,
							(GetRandomControl() & 0x3F) + laraItem->Position.zPos - 32,
							(GetRandomControl() & 3) + (bladeItem->ItemFlags[3] / 32) + 2,
							2 * GetRandomControl(),
							laraItem->RoomNumber
						);
					}

					if (!coll->Setup.EnableObjectPush)
					{
						laraItem->Position.xPos += dx;
						laraItem->Position.yPos += dy;
						laraItem->Position.zPos += dz;
					}
				}
			}
		}
	}
}
