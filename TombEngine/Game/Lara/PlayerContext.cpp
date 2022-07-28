#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/PlayerContextStructs.h"

namespace TEN::Entities::Player
{
	PlayerContext::PlayerContext()
	{
	}

	bool PlayerContext::CanRunForward(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanRunBack(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run back state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWalkForward(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // Defined by walk forward state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWalkBack(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // Defined by walk back state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}


	bool PlayerContext::CanSidestepLeft(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y - ANGLE(90.0f),
			int(CLICK(0.8f)), int(-CLICK(0.8f)) // Defined by sidestep left state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanSidestepLeftSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y - ANGLE(90.0f),
			NO_LOWER_BOUND, int(-CLICK(0.8f)), // Defined by sidestep left state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanSidestepRight(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(90.0f),
			int(CLICK(0.8f)), int(-CLICK(0.8f)) // Defined by step sideright state state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanSidestepRightSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(90.0f),
			NO_LOWER_BOUND, int(-CLICK(0.8f)), // Defined by step right state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWadeForwardSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by wade forward state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWalkBackSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextGroundMovementSetup contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by walk back state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanTurnFast(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		if (lara->Control.WaterStatus == WaterStatus::Dry &&
			((lara->Control.HandStatus == HandStatus::WeaponReady && lara->Control.Weapon.GunType != LaraWeaponType::Torch) ||
				(lara->Control.HandStatus == HandStatus::WeaponDraw && lara->Control.Weapon.GunType != LaraWeaponType::Flare)))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::TestGroundMovementSetup(ItemInfo* item, CollisionInfo* coll, ContextGroundMovementSetup contextSetup, bool useCrawlSetup)
	{
		// HACK: coll->Setup.Radius and coll->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. @Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

		int yPos = item->Pose.Position.y;
		auto probe = GetCollision(item, contextSetup.Angle, OFFSET_RADIUS(playerRadius), -playerHeight);

		// 1. Check for wall.
		if (probe.Position.Floor == NO_HEIGHT)
			return false;

		bool isSlopeDown = contextSetup.CheckSlopeDown ? (probe.Position.FloorSlope && probe.Position.Floor > yPos) : false;
		bool isSlopeUp = contextSetup.CheckSlopeUp ? (probe.Position.FloorSlope && probe.Position.Floor < yPos) : false;
		bool isDeathFloor = contextSetup.CheckDeathFloor ? probe.Block->Flags.Death : false;

		// 2. Check for slope or death floor (if applicable).
		if (isSlopeDown || isSlopeUp || isDeathFloor)
			return false;

		auto origin1 = GameVector(
			item->Pose.Position.x,
			yPos + contextSetup.UpperFloorBound - 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto target1 = GameVector(
			probe.Coordinates.x,
			yPos + contextSetup.UpperFloorBound - 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		auto origin2 = GameVector(
			item->Pose.Position.x,
			yPos - playerHeight + 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto target2 = GameVector(
			probe.Coordinates.x,
			probe.Coordinates.y + 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&origin1, &target1) || !LOS(&origin2, &target2))
			return false;

		// 4. Assess point probe collision.
		if ((probe.Position.Floor - yPos) <= contextSetup.LowerFloorBound &&   // Floor is within lower floor bound.
			(probe.Position.Floor - yPos) >= contextSetup.UpperFloorBound &&   // Floor is within upper floor bound.
			(probe.Position.Ceiling - yPos) < -playerHeight &&				   // Ceiling is within lowest ceiling bound (i.e. player's height).
			abs(probe.Position.Ceiling - probe.Position.Floor) > playerHeight) // Space is not too narrow.
		{
			return true;
		}

		return false;
	}
}
