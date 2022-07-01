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
				auto oldPos = laraItem->Pose.Position;

				if (ItemPushItem(bladeItem, laraItem, coll, 1, 1))
				{
					DoDamage(laraItem, bladeItem->ItemFlags[3]);

					auto dPos = oldPos - laraItem->Pose.Position;
 
					if ((dPos.x || dPos.y || dPos.z) && TriggerActive(bladeItem))
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
						laraItem->Pose.Position += dPos;
				}
			}
		}
	}
}
