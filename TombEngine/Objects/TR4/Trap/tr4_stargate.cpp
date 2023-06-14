#include "framework.h"
#include "Objects/TR4/Trap/tr4_stargate.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using std::vector;

namespace TEN::Entities::Traps
{
	constexpr auto STARGATE_HARM_DAMAGE = 100;

	const vector<unsigned int> StargateHarmJoints = { 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25 };
	const vector<Vector3i> StargateBounds =
	{
		Vector3i(-CLICK(2), CLICK(2), -SECTOR(2)), 
		Vector3i(-896, -96, 96),
		Vector3i(-CLICK(2), CLICK(2), -128),
		Vector3i(0, -96, 96),
		Vector3i(-CLICK(2), -384, -SECTOR(2)),
		Vector3i(0, -96, 96),
		Vector3i(384, CLICK(2), -SECTOR(2)),
		Vector3i(0, -96, 96)
	};

	void StargateControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			SoundEffect(SFX_TR4_STARGATE_SWIRL, &item->Pose);
			AnimateItem(item);
		}
	}

	void StargateCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Status == ITEM_INVISIBLE)
			return;

		// TODO: Border collision.
		/*for (auto& bounds : StargateBounds)
		{
			GlobalCollisionBounds.X1 = bounds.x;
			GlobalCollisionBounds.Y1 = bounds.y;
			GlobalCollisionBounds.Z1 = bounds.z;

			if (TestWithGlobalCollisionBounds(item, laraItem, coll))
				ItemPushItem(item, laraItem, coll, false, 2);
		}*/

		if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
			return;

		if (TestCollision(item, laraItem) &&
			TriggerActive(item) &&
			item->Animation.FrameNumber > GetAnimData(item).frameBase + 20 && // Hardcoded frame range.
			item->Animation.FrameNumber < GetAnimData(item).frameBase + 60)
		{
			// Blades deal damage cumulatively.
			for (int i = 0; i < StargateHarmJoints.size(); i++)
			{
				if (item->TouchBits.Test(StargateHarmJoints[i]))
				{
					DoDamage(laraItem, STARGATE_HARM_DAMAGE);
					DoBloodSplat(
						(GetRandomControl() & 0x3F) + laraItem->Pose.Position.x - 32,
						(GetRandomControl() & 0x1F) + CreatureSpheres[i].y - 16,
						(GetRandomControl() & 0x3F) + laraItem->Pose.Position.z - 32,
						(GetRandomControl() & 3) + 2,
						GetRandomControl() * 2,
						laraItem->RoomNumber);
				}
			}
		}
	}
}
