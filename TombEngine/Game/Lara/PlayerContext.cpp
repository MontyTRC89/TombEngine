#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Flow/ScriptInterfaceFlowHandler.h"
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

	PlayerContext::PlayerContext(ItemInfo* item, CollisionInfo* coll)
	{
		PlayerItemPtr = item;
		PlayerCollPtr = coll;
	}

	bool PlayerContext::CanTurnFast()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		if (player->Control.WaterStatus == WaterStatus::Dry &&
			((player->Control.HandStatus == HandStatus::WeaponReady && player->Control.Weapon.GunType != LaraWeaponType::Torch) ||
				(player->Control.HandStatus == HandStatus::WeaponDraw && player->Control.Weapon.GunType != LaraWeaponType::Flare)))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanRunForward()
	{
		Context::SetupGroundMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanRunBackward()
	{
		Context::SetupGroundMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + Angle::DegToRad(180.0f),
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run backward state.
			false, false, false
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanWalkForward()
	{
		Context::SetupGroundMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // Defined by walk forward state.
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanWalkBackward()
	{
		Context::SetupGroundMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + Angle::DegToRad(180.0f),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // Defined by walk backward state.
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanSidestepLeft()
	{
		return this->TestSidestep(false);
	}

	bool PlayerContext::CanSidestepRight()
	{
		return this->TestSidestep(true);
	}

	bool PlayerContext::CanWadeForward()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		if (player->Control.WaterStatus != WaterStatus::Wade)
			return false;

		if (TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
		{
			Context::SetupGroundMovement contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y,
				NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by wade forward state.
				false, false, false
			};
			return this->TestGroundMovementSetup(contextSetup);
		}

		// TODO: More specific test for wading in water.
		return this->CanRunForward();
	}

	bool PlayerContext::CanWadeBackward()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		if (player->Control.WaterStatus != WaterStatus::Wade)
			return false;

		if (TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
		{
			Context::SetupGroundMovement contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y + Angle::DegToRad(180.0f),
				NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by walk backward state.
				false, false, false
			};
			return this->TestGroundMovementSetup(contextSetup);
		}

		// TODO: More specific test for wading in water.
		return this->CanWalkBackward();
	}

	bool PlayerContext::CanCrouch()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		if (player->Control.WaterStatus != WaterStatus::Wade &&
			(player->Control.HandStatus == HandStatus::Free ||
				!IsStandingWeapon(PlayerItemPtr, player->Control.Weapon.GunType)))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanCrouchToCrawl()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		if (!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
			player->Control.HandStatus == HandStatus::Free &&			// Hands are free.
			(player->Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare. TODO: Should be allowed, but the flare animation bugs out right now. -- Sezz 2022.03.18
				player->Flare.Life))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanCrouchRoll()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		// 1. Assess water depth.
		if (player->WaterSurfaceDist < -CLICK(1)) // TODO: Demagic: LARA_CRAWL_WATER_HEIGHT_MAX
			return false;

		// 2. Assess continuity of path.
		float distance = 0.0f;
		auto probeA = GetCollision(PlayerItemPtr);
		while (distance < SECTOR(1))
		{
			distance += CLICK(1);
			auto probeB = GetCollision(PlayerItemPtr, PlayerItemPtr->Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

			if (abs(probeA.Position.Floor - probeB.Position.Floor) > CRAWL_STEPUP_HEIGHT ||	 // Avoid floor height differences beyond crawl stepup threshold.
				abs(probeB.Position.Ceiling - probeB.Position.Floor) <= LARA_HEIGHT_CRAWL || // Avoid narrow spaces.
				probeB.Position.FloorSlope)													 // Avoid slopes.
			{
				return false;
			}

			probeA = probeB;
		}

		return true;
	}

	bool PlayerContext::CanCrawlForward()
	{
		Context::SetupGroundMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl forward state.
		};
		return this->TestGroundMovementSetup(contextSetup, true);
	}

	bool PlayerContext::CanCrawlBackward()
	{
		Context::SetupGroundMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + Angle::DegToRad(180.0f),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl backward state.
		};
		return this->TestGroundMovementSetup(contextSetup, true);
	}

	bool PlayerContext::CanMonkeyForward()
	{
		Context::SetupMonkeyMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			int(CLICK(1.25f)), int(-CLICK(1.25f)) // Defined by monkey forward state.
		};
		return TestMonkeyMovementSetup(contextSetup);
	}

	bool PlayerContext::CanMonkeyBackward()
	{
		Context::SetupMonkeyMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + Angle::DegToRad(180.0f),
			int(CLICK(1.25f)), int(-CLICK(1.25f)) // Defined by monkey backward state.
		};
		return TestMonkeyMovementSetup(contextSetup);
	}

	bool PlayerContext::CanMonkeyShimmyLeft()
	{
		return this->TestMonkeyShimmy(false);
	}
	
	bool PlayerContext::CanMonkeyShimmyRight()
	{
		return this->TestMonkeyShimmy(true);
	}

	bool PlayerContext::CanJumpUp()
	{
		Context::SetupJump contextSetup =
		{
			0,
			0,
			false
		};
		return TestJumpMovementSetup(contextSetup);
	}

	bool PlayerContext::CanJumpForward()
	{
		return this->TestDirectionalStandingJump(Angle::DegToRad(0.0f));
	}

	bool PlayerContext::CanJumpBackward()
	{
		return this->TestDirectionalStandingJump(Angle::DegToRad(180.0f));
	}

	bool PlayerContext::CanJumpLeft()
	{
		return this->TestDirectionalStandingJump(Angle::DegToRad(-90.0f));
	}

	bool PlayerContext::CanJumpRight()
	{
		return this->TestDirectionalStandingJump(Angle::DegToRad(90.0f));
	}

	bool PlayerContext::CanRunJumpForward()
	{
		Context::SetupJump contextSetup
		{
			PlayerItemPtr->Pose.Orientation.y,
			(int)CLICK(1.5f)
		};
		return TestJumpMovementSetup(contextSetup);
	}

	bool PlayerContext::CanSlideJumpForward()
	{
		return true;

		// TODO: Broken on diagonal slides?
		if (g_GameFlow->HasSlideExtended())
		{
			auto probe = GetCollision(PlayerItemPtr);

			short directionAngle = GetLaraSlideDirection(PlayerItemPtr, PlayerCollPtr);
			short steepnessAngle = GetSurfaceSteepnessAngle(probe.FloorTilt);
			return (abs(short(PlayerCollPtr->Setup.ForwardAngle - directionAngle)) <= abs(steepnessAngle));
		}

		return true;
	}

	bool PlayerContext::CanCrawlspaceDive()
	{
		auto probe = GetCollision(PlayerItemPtr, PlayerCollPtr->Setup.ForwardAngle, PlayerCollPtr->Setup.Radius, -PlayerCollPtr->Setup.Height);

		if (abs(probe.Position.Ceiling - probe.Position.Floor) < LARA_HEIGHT ||
			TestLaraKeepLow(PlayerItemPtr, PlayerCollPtr))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::TestSidestep(bool goingRight)
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		// TODO: Make specific condition for wading in water.
		if (player->Control.WaterStatus == WaterStatus::Wade &&
			TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
		{
			Context::SetupGroundMovement contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y + (goingRight ? Angle::DegToRad(90.0f) : Angle::DegToRad(-90.0f)),
				NO_LOWER_BOUND, (int)-CLICK(0.8f), // Upper bound defined by sidestep left/right states.
				false, false, false
			};
			return this->TestGroundMovementSetup(contextSetup);
		}
		else
		{
			Context::SetupGroundMovement contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y + (goingRight ? Angle::DegToRad(90.0f) : Angle::DegToRad(-90.0f)),
				(int)CLICK(0.8f), (int)-CLICK(0.8f) // Defined by sidestep left/right states.
			};
			return this->TestGroundMovementSetup(contextSetup);
		}
	}

	bool PlayerContext::TestMonkeyShimmy(bool goingRight)
	{
		Context::SetupMonkeyMovement contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + (goingRight ? Angle::DegToRad(90.0f) : Angle::DegToRad(-90.0f)),
			int(CLICK(0.5f)), int(-CLICK(0.5f)) // Defined by monkey shimmy left/right states.
		};
		return TestMonkeyMovementSetup(contextSetup);
	}

	bool PlayerContext::TestDirectionalStandingJump(float angle)
	{
		if (TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
			return false;

		Context::SetupJump testSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + angle,
			(int)CLICK(0.85f)
		};
		return TestJumpMovementSetup(testSetup);
	}

	bool PlayerContext::TestGroundMovementSetup(Context::SetupGroundMovement contextSetup, bool useCrawlSetup)
	{
		// HACK: PlayerCollPtr->Setup.Radius and PlayerCollPtr->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. -- Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : PlayerCollPtr->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : PlayerCollPtr->Setup.Height;

		int yPos = PlayerItemPtr->Pose.Position.y;
		int yPosTop = yPos - playerHeight;
		auto probe = GetCollision(PlayerItemPtr, contextSetup.Angle, OFFSET_RADIUS(playerRadius), -playerHeight);

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
			PlayerItemPtr->Pose.Position.x,
			(yPos + contextSetup.UpperFloorBound) - 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetA = GameVector(
			probe.Coordinates.x,
			(yPos + contextSetup.UpperFloorBound) - 1,
			probe.Coordinates.z,
			PlayerItemPtr->RoomNumber
		);

		// Raycast setup at lowest ceiling bound (player height).
		auto originB = GameVector(
			PlayerItemPtr->Pose.Position.x,
			yPosTop + 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetB = GameVector(
			probe.Coordinates.x,
			yPosTop + 1,
			probe.Coordinates.z,
			PlayerItemPtr->RoomNumber
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

	bool PlayerContext::TestMonkeyMovementSetup(Context::SetupMonkeyMovement contextSetup)
	{
		// HACK: Have to make the height explicit for now (see comment in above function). -- Sezz 2022.07.28
		int playerHeight = LARA_HEIGHT_MONKEY;

		int yPos = PlayerItemPtr->Pose.Position.y;
		int yPosTop = yPos - playerHeight;
		auto probe = GetCollision(PlayerItemPtr, contextSetup.Angle, OFFSET_RADIUS(PlayerCollPtr->Setup.Radius));

		// 1. Check for wall.
		if (probe.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Check for ceiling slope.
		if (probe.Position.CeilingSlope)
			return false;

		// Raycast setup at highest floor bound (player base)
		auto originA = GameVector(
			PlayerItemPtr->Pose.Position.x,
			yPos - 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetA = GameVector(
			probe.Coordinates.x,
			yPos - 1,
			probe.Coordinates.z,
			PlayerItemPtr->RoomNumber
		);
		
		// Raycast setup at lower ceiling bound.
		auto originB = GameVector(
			PlayerItemPtr->Pose.Position.x,
			(yPosTop + contextSetup.LowerCeilingBound) + 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetB = GameVector(
			probe.Coordinates.x,
			(yPosTop + contextSetup.LowerCeilingBound) + 1,
			probe.Coordinates.z,
			PlayerItemPtr->RoomNumber
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

	bool PlayerContext::TestJumpMovementSetup(Context::SetupJump contextSetup)
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		int yPos = PlayerItemPtr->Pose.Position.y;
		auto probe = GetCollision(PlayerItemPtr, contextSetup.Angle, contextSetup.Distance, -PlayerCollPtr->Setup.Height);

		// 1. Check for wall.
		if (probe.Position.Floor == NO_HEIGHT)
			return false;

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr);
		bool isWading = contextSetup.CheckWadeStatus ? (player->Control.WaterStatus == WaterStatus::Wade) : false;

		// 2. Check for swamp or wade status (if applicable).
		if (isSwamp || isWading)
			return false;

		// 3. Check for corner.
		if (TestLaraFacingCorner(PlayerItemPtr, contextSetup.Angle, contextSetup.Distance))
			return false;

		// 4. Assess point probe collision.
		if ((probe.Position.Floor - yPos) >= -STEPUP_HEIGHT &&											  // Floor is within highest floor bound.
			((probe.Position.Ceiling - yPos) < -(PlayerCollPtr->Setup.Height + (LARA_HEADROOM * 0.8f)) || // Ceiling is within lowest ceiling bound... 
				((probe.Position.Ceiling - yPos) < -PlayerCollPtr->Setup.Height &&								// OR ceiling is level with Lara's head
					(probe.Position.Floor - yPos) >= CLICK(0.5f))))												// AND there is a drop below.
		{
			return true;
		}

		return false;
	}
}
