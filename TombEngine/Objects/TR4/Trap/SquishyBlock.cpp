#include "framework.h"
#include "Objects/TR4/Trap/SquishyBlock.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN::Entities::Traps
{	
	void ControlSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		short ang;
		short frame;

		if (!TriggerActive(&item))
			return;

		frame = item.Animation.FrameNumber - GetAnimData(item).frameBase;

		if (&item.TouchBits)
		{
			ang = (short)phd_atan(item.Pose.Position.z - LaraItem->Pose.Position.z, item.Pose.Position.x - LaraItem->Pose.Position.x) - item.Pose.Orientation.y;

			if (!frame && ang > 0xA000 && ang < 0xE000)
			{
				item.ItemFlags[0] = 9;
				LaraItem->HitPoints = 0;
				LaraItem->Pose.Orientation.y = item.Pose.Orientation.y - 0x4000;
			}
			else if (frame == 33 && ang > 0x2000 && ang < 0x6000)
			{
				item.ItemFlags[0] = 42;
				LaraItem->HitPoints = 0;
				LaraItem->Pose.Orientation.y = item.Pose.Orientation.y + 0x4000;
			}
		}

		if (!item.ItemFlags[0] || frame != item.ItemFlags[0])
			AnimateItem(&item);
	}

	void FallingSquishyBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TestBoundsCollide(&item, laraItem, coll->Setup.Radius) && TestCollision(&item, laraItem))
		{
			if (item.Animation.FrameNumber - GetAnimData(item).frameBase <= 8)
			{
				item.Animation.FrameNumber += 2;
				laraItem->HitPoints = 0;
				SetAnimation(laraItem, LA_BOULDER_DEATH);
				laraItem->Animation.Velocity.z = 0;
				laraItem->Animation.Velocity.y = 0;		
			}
			else if (laraItem->HitPoints > 0)
				ItemPushItem(&item, laraItem, coll, false, 1);
		}
	}

	void ControlFallingSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TriggerActive(&item))
		{
			if (item.ItemFlags[0] < 60)
			{
				SoundEffect(SFX_TR4_EARTHQUAKE_LOOP, &item.Pose);
				Camera.bounce = (item.ItemFlags[0] - 92) >> 1;
				item.ItemFlags[0]++;
			}
			else
			{
				if (item.Animation.FrameNumber - GetAnimData(item).frameBase == 8)
					Camera.bounce = -96;

				AnimateItem(&item);
			}
		}
	}
}
