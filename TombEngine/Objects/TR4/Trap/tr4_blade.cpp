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
	void BladeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* bladeItem = &g_Level.Items[itemNumber];

		if (bladeItem->Status == ITEM_INVISIBLE)
			return;

		if (bladeItem->ItemFlags[3])
		{
			if (TestBoundsCollide(bladeItem, laraItem, coll->Setup.Radius))
			{
				int oldX = laraItem->Pose.Position.x;
				int oldY = laraItem->Pose.Position.y;
				int oldZ = laraItem->Pose.Position.z;

				int dx = 0;
				int dy = 0;
				int dz = 0;

				if (ItemPushItem(bladeItem, laraItem, coll, 1, 1))
				{
					DoDamage(laraItem, bladeItem->ItemFlags[3]);

					dx = oldX - laraItem->Pose.Position.x;
					dy = oldY - laraItem->Pose.Position.y;
					dz = oldZ - laraItem->Pose.Position.z;

					if ((dx || dy || dz) && TriggerActive(bladeItem))
					{
						DoBloodSplat(
							(GetRandomControl() & 0x3F) + laraItem->Pose.Position.x - 32,
							laraItem->Pose.Position.y - (GetRandomControl() & 0x1FF) - 256,
							(GetRandomControl() & 0x3F) + laraItem->Pose.Position.z - 32,
							(GetRandomControl() & 3) + (bladeItem->ItemFlags[3] / 32) + 2,
							2 * GetRandomControl(),
							laraItem->RoomNumber
						);
					}

					if (!coll->Setup.EnableObjectPush)
					{
						laraItem->Pose.Position.x += dx;
						laraItem->Pose.Position.y += dy;
						laraItem->Pose.Position.z += dz;
					}
				}
			}
		}
	}
}
