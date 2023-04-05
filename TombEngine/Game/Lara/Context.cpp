#include "framework.h"
#include "Game/Lara/Context.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/ContextData.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Internal/TEN/Flow/FlowHandler.h"

namespace TEN::Entities::Player::Context
{
	static bool TestLedgeClimbSetup(ItemInfo& item, CollisionInfo& coll, const LedgeClimbSetupData& setupData)
	{
		constexpr auto ABS_FLOOR_BOUND = CLICK(1);

		// Get point collision.
		auto pointCollCenter = GetCollision(&item);
		auto pointCollFront = GetCollision(&item, setupData.HeadingAngle, OFFSET_RADIUS(coll.Setup.Radius), -(LARA_HEIGHT_STRETCH + CLICK(0.5f)));

		int vPosTop = item.Pose.Position.y - LARA_HEIGHT_STRETCH;
		int relFloorHeight = abs(pointCollFront.Position.Floor - vPosTop);
		int floorToCeilHeight = abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor);
		int gapHeight = abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor);

		// 1. Test for slippery slope (if applicable).
		// TODO: This check fails for no reason.
		bool isSlipperySlope = setupData.TestSlipperySlope ? pointCollFront.Position.FloorSlope : false;
		if (isSlipperySlope)
			return false;

		// 2. Test for object blocking ledge.
		TestForObjectOnLedge(&item, &coll);
		if (coll.HitStatic)
			return false;

		// 3. Test for valid ledge.
		if (!TestValidLedge(&item, &coll))
			return false;

		// 4. Assess point collision.
		if (relFloorHeight <= ABS_FLOOR_BOUND &&				// Floor height is within lower/upper floor bounds.
			floorToCeilHeight > setupData.FloorToCeilingMin &&	// Floor-to-ceiling height isn't too narrow.
			floorToCeilHeight <= setupData.FloorToCeilingMax && // Floor-to-ceiling height isn't too wide.
			gapHeight >= setupData.GapHeightMin)				// Gap height is permissive.
		{
			return true;
		}

		return false;
	}

	bool CanSwingOnLedge(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto UPPER_FLOOR_BOUND = 0;
		constexpr auto LOWER_CEIL_BOUND	 = CLICK(1.5f);

		auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetCollision(&item, item.Pose.Orientation.y, BLOCK(0.25f));
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - LARA_HEIGHT_REACH);

		// 1. Test for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		// 2. Assess point collision.
		if (relFloorHeight >= UPPER_FLOOR_BOUND && // Floor height is below upper floor bound.
			relCeilHeight <= LOWER_CEIL_BOUND)	   // Ceiling height is above lower ceiling bound.
		{
			return true;
		}

		return false;
	}

	bool CanPerformLedgeJump(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto LEDGE_HEIGHT_MIN = CLICK(2);

		// 1. Check if ledge jumps are enabled.
		if (!g_GameFlow->HasLedgeJumps())
			return false;

		// Ray collision setup at minimum ledge height.
		auto origin = GameVector(
			item.Pose.Position.x,
			(item.Pose.Position.y - LARA_HEIGHT_STRETCH) + LEDGE_HEIGHT_MIN,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target = GameVector(
			Geometry::TranslatePoint(origin.ToVector3i(), item.Pose.Orientation.y, OFFSET_RADIUS(coll.Setup.Radius)),
			item.RoomNumber);

		// 2. Assess level geometry ray collision.
		if (LOS(&origin, &target))
			return false;

		// TODO: Assess static object geometry ray collision.

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - LARA_HEIGHT_STRETCH);

		// 3. Assess point collision.
		if (relCeilHeight >= -coll.Setup.Height) // Ceiling height is below upper ceiling bound.
			return false;

		return true;
	}

	bool CanPerformLedgeHandstand(ItemInfo& item, CollisionInfo& coll)
	{
		auto setupData = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT, -MAX_HEIGHT,
			CLICK(3),
			false
		};

		return TestLedgeClimbSetup(item, coll, setupData);
	}

	bool CanClimbLedgeToCrouch(ItemInfo& item, CollisionInfo& coll)
	{
		auto setupData = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,
			(int)CLICK(0.6f),
			true
		};

		return TestLedgeClimbSetup(item, coll, setupData);
	}

	bool CanClimbLedgeToStand(ItemInfo& item, CollisionInfo& coll)
	{
		auto setupData = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT, -MAX_HEIGHT,
			CLICK(1),
			false
		};

		return TestLedgeClimbSetup(item, coll, setupData);
	}

	bool CanLedgeShimmyLeft(ItemInfo& item, CollisionInfo& coll)
	{
		return TestLaraHangSideways(&item, &coll, ANGLE(-90.0f));
	}

	bool CanLedgeShimmyRight(ItemInfo& item, CollisionInfo& coll)
	{
		return TestLaraHangSideways(&item, &coll, ANGLE(90.0f));
	}

	bool CanWallShimmyUp(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto WALL_STEP_HEIGHT = -CLICK(1);

		auto& player = GetLaraInfo(item);

		// 1. Test if wall is climbable.
		if (!player.Control.CanClimbLadder)
			return false;

		// Get point collision.
		auto pointCollCenter = GetCollision(&item);
		auto pointCollLeft = GetCollision(&item, item.Pose.Orientation.y - ANGLE(90.0f), OFFSET_RADIUS(coll.Setup.Radius));
		auto pointCollRight = GetCollision(&item, item.Pose.Orientation.y + ANGLE(90.0f), OFFSET_RADIUS(coll.Setup.Radius));

		int vPosTop = item.Pose.Position.y - LARA_HEIGHT_STRETCH;
		int relCeilHeightCenter = pointCollCenter.Position.Ceiling - vPosTop;
		int relCeilHeightLeft = pointCollCenter.Position.Ceiling - vPosTop;
		int relCeilHeightRight = pointCollCenter.Position.Ceiling - vPosTop;

		// 2. Assess point collision.
		if (relCeilHeightCenter <= WALL_STEP_HEIGHT &&
			relCeilHeightLeft <= WALL_STEP_HEIGHT &&
			relCeilHeightRight <= WALL_STEP_HEIGHT)
		{
			return true;
		}

		return false;
	}

	// TODO!!
	bool CanWallShimmyDown(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto WALL_STEP_HEIGHT = CLICK(1);

		auto& player = GetLaraInfo(item);

		auto pointCollCenter = GetCollision(&item);

		int relFloorHeight = pointCollCenter.Position.Floor - item.Pose.Position.y;
		// Left and right.

		// 1. Check if wall is climbable.
		if (!player.Control.CanClimbLadder)
			return false;

		// 2. Assess point collision.
		if (relFloorHeight >= WALL_STEP_HEIGHT)
			return true;

		return false;
	}
}
