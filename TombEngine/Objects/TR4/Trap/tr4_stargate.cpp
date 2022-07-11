#include "framework.h"
#include "tr4_stargate.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	short StargateBounds[24] =
	{
		-512, 512, -1024, 
		-896, -96, 96, 
		-512, 512, -128, 
		0, -96, 96, 
		-512, -384, -1024, 
		0, -96, 96, 
		384, 512, -1024, 
		0, -96, 96
	};

	void StargateControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->ItemFlags[3] = 50;

		if (TriggerActive(item))
		{
			int touchBits = 0x036DB600; 
			item->ItemFlags[0] = short(touchBits & 0xFFFF);
			item->ItemFlags[1] = short((touchBits >> 16) & 0xFFFF);

			SoundEffect(SFX_TR4_STARGATE_SWIRL, &item->Pose);
			AnimateItem(item);
		}
		else
			item->ItemFlags[0] = item->ItemFlags[1] = 0;
	}

	void StargateCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Status == ITEM_INVISIBLE)
			return;

		if (TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		{
			for (int i = 0; i < 8; i++)
			{
				GlobalCollisionBounds.X1 = StargateBounds[3 * i + 0];
				GlobalCollisionBounds.Y1 = StargateBounds[3 * i + 1];
				GlobalCollisionBounds.Z1 = StargateBounds[3 * i + 2];

				if (TestWithGlobalCollisionBounds(item, laraItem, coll))
					ItemPushItem(item, laraItem, coll, 0, 2);
			}

			int result = TestCollision(item, laraItem);
			if (result)
			{
				int flags = item->ItemFlags[0] | item->ItemFlags[1] << 16;
				result &= flags;

				if (result)
				{
					int j = 0;
					do
					{
						if (result & 1)
						{
							GlobalCollisionBounds.X1 = CreatureSpheres[j].x - CreatureSpheres[j].r - item->Pose.Position.x;
							GlobalCollisionBounds.Y1 = CreatureSpheres[j].y - CreatureSpheres[j].r - item->Pose.Position.y;
							GlobalCollisionBounds.Z1 = CreatureSpheres[j].z - CreatureSpheres[j].r - item->Pose.Position.z;
							GlobalCollisionBounds.X2 = CreatureSpheres[j].x + CreatureSpheres[j].r - item->Pose.Position.x;
							GlobalCollisionBounds.Y2 = CreatureSpheres[j].y + CreatureSpheres[j].r - item->Pose.Position.y;
							GlobalCollisionBounds.Z2 = CreatureSpheres[j].z + CreatureSpheres[j].r - item->Pose.Position.z;

							int oldX = LaraItem->Pose.Position.x;
							int oldY = LaraItem->Pose.Position.y;
							int oldZ = LaraItem->Pose.Position.z;

							if (ItemPushItem(item, laraItem, coll, flags & 1, 2))
							{
								if ((flags & 1) &&
									(oldX != LaraItem->Pose.Position.x ||
									oldY != LaraItem->Pose.Position.y ||
									oldZ != LaraItem->Pose.Position.z) &&
									TriggerActive(item))
								{
									DoBloodSplat((GetRandomControl() & 0x3F) + laraItem->Pose.Position.x - 32,
										(GetRandomControl() & 0x1F) + CreatureSpheres[j].y - 16,
										(GetRandomControl() & 0x3F) + laraItem->Pose.Position.z - 32,
										(GetRandomControl() & 3) + 2,
										2 * GetRandomControl(),
										laraItem->RoomNumber);

									DoDamage(laraItem, 100);
								}
							}
						}

						result /= 2;
						j++;
						flags /= 2;

					} while (result);
				}
			}
		}
	}
}
