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
		SQUISHY_BLOCK_STATE_COLLIDE_LEFT = 1,
		SQUISHY_BLOCK_STATE_COLLIDE_RIGHT = 2,
		SQUISHY_BLOCK_STATE_ORIGINAL = 3,
	};

	enum SquishyBlockAnim
	{
		SQUISHY_BLOCK_ANIM_MOVE = 0,
		SQUISHY_BLOCK_ANIM_COLLIDE_LEFT = 1,
		SQUISHY_BLOCK_ANIM_COLLIDE_RIGHT = 2,
		SQUISHY_BLOCK_ANIM_ORIGINAL = 3,
	};

	void InitializeSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!item.TriggerFlags)
		{
			SetAnimation(item, SQUISHY_BLOCK_ANIM_ORIGINAL);
			item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + SQUISHY_BLOCK_ANIM_ORIGINAL;
			item.Animation.FrameNumber = GetAnimData(item).frameBase;
			item.Animation.ActiveState = SQUISHY_BLOCK_STATE_ORIGINAL;
			item.Animation.TargetState = SQUISHY_BLOCK_STATE_ORIGINAL;
		}

			item.ItemFlags[0] = item.TriggerFlags;
			item.ItemFlags[4] = ANGLE(0.0f);
			item.ItemFlags[1] = 0;
			item.HitPoints = NOT_TARGETABLE;
	}

	void ControlSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		if (!TriggerActive(&item))
			return;

		if (!item.TriggerFlags)
		{
			if (item.Animation.ActiveState != SQUISHY_BLOCK_STATE_ORIGINAL)
			{
				item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + SQUISHY_BLOCK_ANIM_ORIGINAL;
				item.Animation.FrameNumber = GetAnimData(item).frameBase;
				item.Animation.ActiveState = SQUISHY_BLOCK_STATE_ORIGINAL;
				item.Animation.TargetState = SQUISHY_BLOCK_STATE_ORIGINAL;
			}
		}
		else
		{
			if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_ORIGINAL)
			{
				item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + SQUISHY_BLOCK_ANIM_MOVE;
				item.Animation.FrameNumber = GetAnimData(item).frameBase;
				item.Animation.ActiveState = SQUISHY_BLOCK_STATE_MOVE;
				item.Animation.TargetState = SQUISHY_BLOCK_STATE_MOVE;
			}

			item.ItemFlags[0] = item.TriggerFlags;

			if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_MOVE)
			{

				auto forwardDir = EulerAngles(0, item.Pose.Orientation.y + item.ItemFlags[4], 0).ToDirection();

				auto pointColl = GetCollision(item.Pose.Position, item.RoomNumber, forwardDir, BLOCK(0.5f));

				if (pointColl.RoomNumber != item.RoomNumber)
					ItemNewRoom(itemNumber, pointColl.RoomNumber);

				if (!IsNextSectorValid(item, forwardDir))
				{
					if (item.ItemFlags[4] == ANGLE(180))
					{
						item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + SQUISHY_BLOCK_ANIM_COLLIDE_LEFT;
						item.Animation.FrameNumber = GetAnimData(item).frameBase;
						item.Animation.ActiveState = SQUISHY_BLOCK_STATE_COLLIDE_LEFT;
						item.Animation.TargetState = SQUISHY_BLOCK_STATE_COLLIDE_LEFT;
					}
					else if (item.ItemFlags[4] == ANGLE(0))
					{
						item.Animation.AnimNumber = Objects[item.ObjectNumber].animIndex + SQUISHY_BLOCK_ANIM_COLLIDE_RIGHT;
						item.Animation.FrameNumber = GetAnimData(item).frameBase;
						item.Animation.ActiveState = SQUISHY_BLOCK_STATE_COLLIDE_RIGHT;
						item.Animation.TargetState = SQUISHY_BLOCK_STATE_COLLIDE_RIGHT;
					}
				}
				else
					item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, forwardDir, Lerp(item.TriggerFlags / 4, item.TriggerFlags, item.ItemFlags[0]));
			}
			else
			{
				if (item.Animation.FrameNumber - GetAnimData(item).frameBase == 19)
				{
					if (item.HitPoints != NOT_TARGETABLE && item.HitPoints)
					{
						item.ItemFlags[1] = item.HitPoints;
						item.HitPoints = NOT_TARGETABLE;
					}

					item.ItemFlags[4] = item.ItemFlags[4] + ANGLE(180.0f);
					item.Pose.Orientation.y += ANGLE(item.ItemFlags[1]);
				}				
			}
		}
			if (LaraItem->HitPoints)
			AnimateItem(&item);		
	}
	
	bool IsNextSectorValid(const ItemInfo& item, const Vector3& dir)
	{
		auto projectedPos = Geometry::TranslatePoint(item.Pose.Position, dir, BLOCK(0.5f));
		auto pointColl = GetCollision(item.Pose.Position, item.RoomNumber, dir, BLOCK(0.5f));
		auto bounds = GameBoundingBox(&item);
		int itemHeight = bounds.GetHeight();
		
		// Test for wall.
		if (pointColl.Block->IsWall(projectedPos.x, projectedPos.z))
			return false;

		// Test for slippery slope.
		if (pointColl.Position.FloorSlope)
			return false;

		// Flat floor.
		if ((abs(pointColl.FloorTilt.x) == 0 && abs(pointColl.FloorTilt.y) == 0))
		{
			// Test for step.
			int relFloorHeight = abs(pointColl.Position.Floor - item.Pose.Position.y);
			if (relFloorHeight >= CLICK(1) && item.Pose.Position.y >= pointColl.Position.Floor)
				return false;
		}
		// Sloped floor.
		else
		{
			// Half block.
			int relFloorHeight = abs(pointColl.Position.Floor - item.Pose.Position.y);
			if (relFloorHeight > CLICK(1))
				return false;

			short slopeAngle = ANGLE(0.0f);
			if (pointColl.FloorTilt.x > 0)
			{
				slopeAngle = ANGLE(-90.0f);
			}
			else if (pointColl.FloorTilt.x < 0)
			{
				slopeAngle = ANGLE(90.0f);
			}

			if (pointColl.FloorTilt.y > 0 && pointColl.FloorTilt.y > abs(pointColl.FloorTilt.x))
			{
				slopeAngle = ANGLE(180.0f);
			}
			else if (pointColl.FloorTilt.y < 0 && -pointColl.FloorTilt.y > abs(pointColl.FloorTilt.x))
			{
				slopeAngle = ANGLE(0.0f);
			}

			short dirAngle = phd_atan(dir.z, dir.x);
			short alignAngle = Geometry::GetShortestAngle(slopeAngle, dirAngle);

			// Test if slope aspect is aligned with direction.
			if (alignAngle != 0 && alignAngle != ANGLE(180.0f))
				return false;
		}

		// Check for diagonal split.
		if (pointColl.Position.DiagonalStep)
			return false;

		// Test ceiling height.
		int relCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);
		
		if (relCeilHeight <= itemHeight)
			return false;

		// Check for blocked grey box.
		if (g_Level.Boxes[pointColl.Block->Box].flags & BLOCKABLE)
		{
			if (g_Level.Boxes[pointColl.Block->Box].flags & BLOCKED)
				return false;
		}

		// Check for inaccessible sector.
		if (pointColl.Block->Box == NO_ZONE)
			return false;

		// Check for stopper flag.
		if (pointColl.Block->Stopper)
			return false;

		return true;
	}

	void SquishyBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TestBoundsCollide(&item, laraItem, coll->Setup.Radius) && TestCollision(&item, laraItem))
		{
			if (laraItem->HitPoints > 0)
				ItemPushItem(&item, laraItem, coll, false, 1);			
		}

		if (ItemPushItem(&item, laraItem, coll, false, 1))
		{
			if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_ORIGINAL)
			{
				auto frame = item.Animation.FrameNumber - GetAnimData(item).frameBase;
				if (!frame || frame == 33)
					LaraItem->HitPoints = 0;
			}
			else if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_COLLIDE_RIGHT ||
					item.Animation.ActiveState == SQUISHY_BLOCK_STATE_COLLIDE_LEFT)
			{
				LaraItem->HitPoints = 0;
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
