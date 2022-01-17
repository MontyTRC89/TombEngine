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

		if (item->status == ITEM_INVISIBLE)
			return;

		if (item->itemFlags[3])
		{
			if (TestBoundsCollide(item, l, coll->Setup.Radius))
			{
				int oldX = LaraItem->pos.xPos;
				int oldY = LaraItem->pos.yPos;
				int oldZ = LaraItem->pos.zPos;

				int dx = 0;
				int dy = 0;
				int dz = 0;

				if (ItemPushItem(item, l, coll, 1, 1))
				{
					LaraItem->hitPoints -= item->itemFlags[3];

					dx = oldX - LaraItem->pos.xPos;
					dy = oldY - LaraItem->pos.yPos;
					dz = oldZ - LaraItem->pos.zPos;

					if ((dx || dy || dz) && TriggerActive(item))
					{
						DoBloodSplat((GetRandomControl() & 0x3F) + l->pos.xPos - 32,
							l->pos.yPos - (GetRandomControl() & 0x1FF) - 256,
							(GetRandomControl() & 0x3F) + l->pos.zPos - 32,
							(GetRandomControl() & 3) + (item->itemFlags[3] / 32) + 2,
							2 * GetRandomControl(),
							l->roomNumber);
					}

					if (!coll->Setup.EnableObjectPush)
					{
						LaraItem->pos.xPos += dx;
						LaraItem->pos.yPos += dy;
						LaraItem->pos.zPos += dz;
					}
				}
			}
		}
	}
}