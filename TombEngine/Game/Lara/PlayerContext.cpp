#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/PlayerContextStructs.h"
#include "Specific/input.h"

using namespace TEN::Input;

namespace TEN::Entities::Player
{
	PlayerContext::PlayerContext()
	{
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

	bool PlayerContext::CanRunForward(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanRunBack(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run back state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWalkForward(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // Defined by walk forward state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWalkBack(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // Defined by walk back state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}


	bool PlayerContext::CanSidestepLeft(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y - ANGLE(90.0f),
			int(CLICK(0.8f)), int(-CLICK(0.8f)) // Defined by sidestep left state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanSidestepLeftSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y - ANGLE(90.0f),
			NO_LOWER_BOUND, int(-CLICK(0.8f)), // Defined by sidestep left state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanSidestepRight(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(90.0f),
			int(CLICK(0.8f)), int(-CLICK(0.8f)) // Defined by sidestep right state state.
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanSidestepRightSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(90.0f),
			NO_LOWER_BOUND, int(-CLICK(0.8f)), // Defined by sidestep right state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWadeForwardSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by wade forward state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanWalkBackSwamp(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by walk back state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanCrouch(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		if (lara->Control.WaterStatus != WaterStatus::Wade &&
			(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(item, lara->Control.Weapon.GunType)))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanCrouchToCrawl(ItemInfo* item)
	{
		auto* lara = GetLaraInfo(item);

		if (!(TrInput & (IN_FLARE | IN_DRAW)) &&					  // Avoid unsightly concurrent actions.
			lara->Control.HandStatus == HandStatus::Free &&			  // Hands are free.
			(lara->Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare. TODO: Should be allowed, but the flare animation bugs out right now. @Sezz 2022.03.18
				lara->Flare.Life))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanCrouchRoll(ItemInfo* item, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(item);

		// 1. Assess water depth.
		if (lara->WaterSurfaceDist < -CLICK(1)) // TODO: Demagic: LARA_CRAWL_WATER_HEIGHT_MAX
			return false;

		// 2. Assess continuity of path.
		int distance = 0;
		auto probeA = GetCollision(item);
		while (distance < SECTOR(1))
		{
			distance += CLICK(1);
			auto probeB = GetCollision(item, item->Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

			if (abs(probeA.Position.Floor - probeB.Position.Floor) > CRAWL_STEPUP_HEIGHT ||	 // Avoid floor differences beyond crawl stepup threshold.
				abs(probeB.Position.Ceiling - probeB.Position.Floor) <= LARA_HEIGHT_CRAWL || // Avoid narrow spaces.
				probeB.Position.FloorSlope)													 // Avoid slopes.
			{
				return false;
			}

			probeA = probeB;
		}

		return true;
	}

	bool PlayerContext::CanCrawlForward(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl forward state.
		};

		return PlayerContext::TestGroundMovementSetup(item, coll, contextSetup, true);
	}

	bool PlayerContext::CanCrawlBack(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupGroundMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl back state.
		};

		return PlayerContext::TestGroundMovementSetup(item, coll, contextSetup, true);
	}

	bool PlayerContext::CanMonkeyForward(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupMonkeyMovement contextSetup =
		{
			item->Pose.Orientation.y,
			int(CLICK(1.25f)), int(-CLICK(1.25f)) // Defined by monkey forward state.
		};

		return TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanMonkeyBack(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupMonkeyMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(180.0f),
			int(CLICK(1.25f)), int(-CLICK(1.25f)) // Defined by monkey back state.
		};

		return PlayerContext::TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupMonkeyMovement contextSetup =
		{
			item->Pose.Orientation.y - ANGLE(90.0f),
			int(CLICK(0.5f)), int(-CLICK(0.5f)) // Defined by monkey shimmy left state.
		};

		return PlayerContext::TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::CanMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll)
	{
		ContextSetupMonkeyMovement contextSetup =
		{
			item->Pose.Orientation.y + ANGLE(90.0f),
			int(CLICK(0.5f)), int(-CLICK(0.5f)) // Defined by monkey shimmy right state.
		};

		return PlayerContext::TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool PlayerContext::TestGroundMovementSetup(ItemInfo* item, CollisionInfo* coll, ContextSetupGroundMovement contextSetup, bool useCrawlSetup)
	{
		// HACK: coll->Setup.Radius and coll->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. -- Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

		int yPos = item->Pose.Position.y;
		int yPosTop = yPos - playerHeight;
		auto probe = GetCollision(item, contextSetup.Angle, OFFSET_RADIUS(playerRadius), -playerHeight);

		// 1. Check for wall.
		if (probe.Position.Floor == NO_HEIGHT)
			return false;

		bool isSlopeDown  = contextSetup.CheckSlopeDown  ? (probe.Position.FloorSlope && probe.Position.Floor > yPos) : false;
		bool isSlopeUp	  = contextSetup.CheckSlopeUp	 ? (probe.Position.FloorSlope && probe.Position.Floor < yPos) : false;
		bool isDeathFloor = contextSetup.CheckDeathFloor ? probe.Block->Flags.Death									  : false;

		// 2. Check for floor slope or death floor (if applicable).
		if (isSlopeDown || isSlopeUp || isDeathFloor)
			return false;

		// Raycast setup at upper floor bound.
		auto originA = GameVector(
			item->Pose.Position.x,
			(yPos + contextSetup.UpperFloorBound) - 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetA = GameVector(
			probe.Coordinates.x,
			(yPos + contextSetup.UpperFloorBound) - 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		// Raycast setup at lowest ceiling bound (player height).
		auto originB = GameVector(
			item->Pose.Position.x,
			yPosTop + 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetB = GameVector(
			probe.Coordinates.x,
			yPosTop + 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&originA, &targetA) || !LOS(&originB, &targetB))
			return false;

		// 4. Assess point probe collision.
		if ((probe.Position.Floor - yPos) <= contextSetup.LowerFloorBound &&   // Floor is within lower floor bound.
			(probe.Position.Floor - yPos) >= contextSetup.UpperFloorBound &&   // Floor is within upper floor bound.
			(probe.Position.Ceiling - yPos) < -playerHeight &&				   // Ceiling is within lowest ceiling bound (player height).
			abs(probe.Position.Ceiling - probe.Position.Floor) > playerHeight) // Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::TestMonkeyMovementSetup(ItemInfo* item, CollisionInfo* coll, ContextSetupMonkeyMovement contextSetup)
	{
		// HACK: Have to make the height explicit for now (see comment in above function). -- Sezz 2022.07.28
		int playerHeight = LARA_HEIGHT_MONKEY;

		int yPos = item->Pose.Position.y;
		int yPosTop = yPos - playerHeight;
		auto probe = GetCollision(item, contextSetup.Angle, OFFSET_RADIUS(coll->Setup.Radius));

		// 1. Check for wall.
		if (probe.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Check for ceiling slope.
		if (probe.Position.CeilingSlope)
			return false;

		// Raycast setup at highest floor bound (player base)
		auto originA = GameVector(
			item->Pose.Position.x,
			yPos - 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetA = GameVector(
			probe.Coordinates.x,
			yPos - 1,
			probe.Coordinates.z,
			item->RoomNumber
		);
		
		// Raycast setup at lower ceiling bound.
		auto originB = GameVector(
			item->Pose.Position.x,
			(yPosTop + contextSetup.LowerCeilingBound) + 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetB = GameVector(
			probe.Coordinates.x,
			(yPosTop + contextSetup.LowerCeilingBound) + 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&originA, &targetA) || !LOS(&originB, &targetB))
			return false;

		// 4. Assess point probe collision.
		if (probe.BottomBlock->Flags.Monkeyswing &&								    // Ceiling is monkey swing.
			(probe.Position.Ceiling - yPosTop) <= contextSetup.LowerCeilingBound &&	// Ceiling is within lower ceiling bound.
			(probe.Position.Ceiling - yPosTop) >= contextSetup.UpperCeilingBound &&	// Ceiling is within upper ceiling bound.
			(probe.Position.Floor - yPos) > 0 &&									// Floor is within highest floor bound (player base).
			abs(probe.Position.Ceiling - probe.Position.Floor) > playerHeight)		// Space is not too narrow.
		{
			return true;
		}

		return false;
	}
}
