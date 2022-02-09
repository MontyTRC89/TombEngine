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
	void BladeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->Status == ITEM_INVISIBLE)
			return;

		if (item->ItemFlags[3])
		{
			if (TestBoundsCollide(item, l, coll->Setup.Radius))
			{
				int oldX = LaraItem->Position.xPos;
				int oldY = LaraItem->Position.yPos;
				int oldZ = LaraItem->Position.zPos;

				int dx = 0;
				int dy = 0;
				int dz = 0;

				if (ItemPushItem(item, l, coll, 1, 1))
				{
					LaraItem->HitPoints -= item->ItemFlags[3];

					dx = oldX - LaraItem->Position.xPos;
					dy = oldY - LaraItem->Position.yPos;
					dz = oldZ - LaraItem->Position.zPos;

					if ((dx || dy || dz) && TriggerActive(item))
					{
						DoBloodSplat((GetRandomControl() & 0x3F) + l->Position.xPos - 32,
							l->Position.yPos - (GetRandomControl() & 0x1FF) - 256,
							(GetRandomControl() & 0x3F) + l->Position.zPos - 32,
							(GetRandomControl() & 3) + (item->ItemFlags[3] / 32) + 2,
							2 * GetRandomControl(),
							l->RoomNumber);
					}

					if (!coll->Setup.EnableObjectPush)
					{
						LaraItem->Position.xPos += dx;
						LaraItem->Position.yPos += dy;
						LaraItem->Position.zPos += dz;
					}
				}
			}
		}
	}
}