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

// NOTES:
// item.ItemFlags[0]: use dynamic motion.
// item.ItemFlags[1]: ??
// item.ItemFlags[4]: heading angle.

namespace TEN::Entities::Traps
{	
	enum SquishyBlockState
	{
		SQUISHY_BLOCK_STATE_MOVE = 0,
		SQUISHY_BLOCK_STATE_COLLIDE_LEFT = 1,
		SQUISHY_BLOCK_STATE_COLLIDE_RIGHT = 2,
		SQUISHY_BLOCK_STATE_BAKED_MOTION = 3
	};

	enum SquishyBlockAnim
	{
		SQUISHY_BLOCK_ANIM_MOVE = 0,
		SQUISHY_BLOCK_ANIM_IMPACT_BACK = 1,
		SQUISHY_BLOCK_ANIM_IMPACT_FRONT = 2,
		SQUISHY_BLOCK_ANIM_BAKED_MOTION = 3
	};

	void InitializeSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!item.TriggerFlags)
			SetAnimation(item, SQUISHY_BLOCK_ANIM_BAKED_MOTION);

		item.HitPoints = NOT_TARGETABLE;
		item.ItemFlags[0] = item.TriggerFlags;
		item.ItemFlags[1] = 0;
		item.ItemFlags[4] = 0;
	}

	static bool IsNextSectorValid(const ItemInfo& item, const Vector3& dir)
	{
		auto projectedPos = Geometry::TranslatePoint(item.Pose.Position, dir, BLOCK(0.5f));
		auto pointColl = GetCollision(item.Pose.Position, item.RoomNumber, dir, BLOCK(0.5f));
		int height = GameBoundingBox(&item).GetHeight();

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

		if (relCeilHeight <= height)
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

	void ControlSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& object = Objects[item.ObjectNumber];

		short& something0 = item.ItemFlags[0];
		short& someAngle = item.ItemFlags[1];
		short& headingAngle = item.ItemFlags[4];

		if (!TriggerActive(&item))
			return;

		if (!item.TriggerFlags)
		{
			if (item.Animation.ActiveState != SQUISHY_BLOCK_STATE_BAKED_MOTION)
				SetAnimation(item, SQUISHY_BLOCK_ANIM_BAKED_MOTION);
		}
		else
		{
			if (item.Animation.ActiveState == SQUISHY_BLOCK_ANIM_BAKED_MOTION)
				SetAnimation(item, SQUISHY_BLOCK_STATE_MOVE);

			something0 = item.TriggerFlags;

			if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_MOVE)
			{
				auto forwardDir = EulerAngles(0, item.Pose.Orientation.y + headingAngle, 0).ToDirection();

				auto pointColl = GetCollision(item.Pose.Position, item.RoomNumber, forwardDir, BLOCK(0.5f));

				if (pointColl.RoomNumber != item.RoomNumber)
					ItemNewRoom(itemNumber, pointColl.RoomNumber);

				if (!IsNextSectorValid(item, forwardDir))
				{
					switch (headingAngle)
					{
					default:
					case ANGLE(0.0f):
						SetAnimation(item, SQUISHY_BLOCK_ANIM_IMPACT_FRONT);
						break;

					case ANGLE(-180.0f):
						SetAnimation(item, SQUISHY_BLOCK_ANIM_IMPACT_BACK);
						break;
					}
				}
				else
				{
					float dist = Lerp(item.TriggerFlags / 4, item.TriggerFlags, something0);
					item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, forwardDir, dist);
				}
			}
			else
			{
				if ((item.Animation.FrameNumber - GetAnimData(item).frameBase) == 19)
				{
					if (item.HitPoints != NOT_TARGETABLE && item.HitPoints)
					{
						someAngle = item.HitPoints;
						item.HitPoints = NOT_TARGETABLE;
					}

					headingAngle += ANGLE(180.0f);
					item.Pose.Orientation.y += ANGLE(someAngle);
				}				
			}
		}

		if (LaraItem->HitPoints != 0)
			AnimateItem(&item);		
	}

	void ControlFallingSquishyBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		short& something0 = item.ItemFlags[0];

		if (TriggerActive(&item))
		{
			if (something0 < 60)
			{
				SoundEffect(SFX_TR4_EARTHQUAKE_LOOP, &item.Pose);
				Camera.bounce = (something0 - 92) >> 1;
				something0++;
			}
			else
			{
				if ((item.Animation.FrameNumber - GetAnimData(item).frameBase) == 8)
					Camera.bounce = -96;

				AnimateItem(&item);
			}
		}
	}

	void CollideSquishyBlock(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TestBoundsCollide(&item, playerItem, coll->Setup.Radius) && TestCollision(&item, playerItem))
		{
			if (playerItem->HitPoints > 0)
				ItemPushItem(&item, playerItem, coll, false, 1);
		}

		if (ItemPushItem(&item, playerItem, coll, false, 1))
		{
			if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_BAKED_MOTION)
			{
				int frameNumber = item.Animation.FrameNumber - GetAnimData(item).frameBase;
				if (!frameNumber || frameNumber == 33)
					DoDamage(playerItem, INT_MAX);
			}
			else if (item.Animation.ActiveState == SQUISHY_BLOCK_STATE_COLLIDE_RIGHT ||
				item.Animation.ActiveState == SQUISHY_BLOCK_STATE_COLLIDE_LEFT)
			{
				DoDamage(playerItem, INT_MAX);
			}
		}	
	}

	void CollideFallingSquishyBlock(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (TestBoundsCollide(&item, playerItem, coll->Setup.Radius) && TestCollision(&item, playerItem))
		{
			if ((item.Animation.FrameNumber - GetAnimData(item).frameBase) <= 8)
			{
				item.Animation.FrameNumber += 2;

				DoDamage(playerItem, INT_MAX);
				SetAnimation(playerItem, LA_BOULDER_DEATH);
				playerItem->Animation.Velocity.y = 0.0f;	
				playerItem->Animation.Velocity.z = 0.0f;	
			}
			else if (playerItem->HitPoints > 0)
			{
				ItemPushItem(&item, playerItem, coll, false, 1);
			}
		}
	}
}
