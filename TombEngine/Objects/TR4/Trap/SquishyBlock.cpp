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
#include "Game/Setup.h"
#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN::Entities::Traps
{	
	enum SquishyBlockState
	{
		SQUISHY_BLOCK_STATE_MOVE = 0,
		SQUISHY_BLOCK_STATE_COLLIDE = 1,
	};

	enum SquishyBlockAnim
	{
		SQUISHY_BLOCK_ANIM_MOVE = 0,
		SQUISHY_BLOCK_ANIM_COLLIDE = 1,
	};


	void InitializeSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[1] = item.TriggerFlags * 2;
	}

	void ControlSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		short ang;
		short frame;

		if (!TriggerActive(&item))
			return;

		frame = item.Animation.FrameNumber - GetAnimData(item).frameBase;
		int forwardVel = item.ItemFlags[1];
		auto bounds = GameBoundingBox(&item);
		auto pointColl = GetCollision(&item, item.Pose.Orientation.y, (forwardVel >= 0) ? bounds.Z2 : bounds.Z1, bounds.Y2);

		int upperFloorBound = item.Pose.Position.y;
		int lowerCeilBound = item.Pose.Position.y + bounds.Y1;

		if (&item.TouchBits)
		{
			ang = (short)phd_atan(item.Pose.Position.x - LaraItem->Pose.Position.x, item.Pose.Position.z - LaraItem->Pose.Position.z) - item.Pose.Orientation.y;

			 if ((item.Animation.ActiveState == SQUISHY_BLOCK_ANIM_COLLIDE &&  (ang > -24586 && ang < -8206)))
			{
				//item.ItemFlags[0] = 9;
				//LaraItem->HitPoints = 0;
				//LaraItem->Pose.Orientation.y = item.Pose.Orientation.y - 0x4000;
			}
		}

		if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_MOVE)
		{
				
			if (pointColl.RoomNumber != item.RoomNumber)
				ItemNewRoom(itemNumber, pointColl.RoomNumber);

			if (pointColl.Block->IsWall(item.Pose.Position.x, item.Pose.Position.z) ||
				pointColl.Block->Stopper ||
				pointColl.Position.Floor < upperFloorBound ||
				pointColl.Position.Ceiling > lowerCeilBound)
			{
					item.Pose.Orientation.y += ANGLE(180.0f);
					item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + SQUISHY_BLOCK_ANIM_COLLIDE;
					item.Animation.FrameNumber = GetAnimData(item).frameBase;
					item.Animation.ActiveState = SQUISHY_BLOCK_STATE_COLLIDE;
					item.Animation.TargetState = SQUISHY_BLOCK_STATE_COLLIDE;
			}
			else
				item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, item.Pose.Orientation.y, forwardVel);							
		}
					
			if (!item.ItemFlags[0])
			AnimateItem(&item);		
	}

	void SquishyBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TestBoundsCollide(&item, laraItem, coll->Setup.Radius) && TestCollision(&item, laraItem))
		{
			if (item.Animation.ActiveState == SQUISHY_BLOCK_ANIM_COLLIDE)
			{
				laraItem->HitPoints = 0;
				laraItem->Animation.Velocity.z = 0;
				laraItem->Animation.Velocity.y = 0;
				item.ItemFlags[0] = 1;
			}
		}
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
