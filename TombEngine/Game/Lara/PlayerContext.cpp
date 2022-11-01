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

	bool PlayerContext::CanAFKPose()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		// Assess whether AFK pose is enabled.
		if (!g_GameFlow->HasAFKPose())
			return false;

		// Assess wade and hand status.
		if (player->Control.WaterStatus == WaterStatus::Wade &&
			player->Control.HandStatus == HandStatus::Free)
		{
			return false;
		}

		// Assess context.
		if (!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
			(player->Control.Weapon.GunType != LaraWeaponType::Flare || // Flare is not being handled.
				player->Flare.Life) &&
			player->Vehicle == NO_ITEM)									// Not in a vehicle.
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanTurn180()
	{
		auto* lara = GetLaraInfo(PlayerItemPtr);

		return (lara->Control.WaterStatus == WaterStatus::Wade || TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr));
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
		Context::GroundMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanRunBackward()
	{
		Context::GroundMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + ANGLE(180.0f),
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run backward state.
			false, false, false
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanWalkForward()
	{
		Context::GroundMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // Defined by walk forward state.
		};
		return this->TestGroundMovementSetup(contextSetup);
	}

	bool PlayerContext::CanWalkBackward()
	{
		Context::GroundMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + ANGLE(180.0f),
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

		// Assess wade status.
		if (player->Control.WaterStatus != WaterStatus::Wade)
			return false;

		if (TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
		{
			Context::GroundMovementSetup contextSetup =
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

		// Assess wade status.
		if (player->Control.WaterStatus != WaterStatus::Wade)
			return false;

		if (TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
		{
			Context::GroundMovementSetup contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y + ANGLE(180.0f),
				NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by walk backward state.
				false, false, false
			};
			return this->TestGroundMovementSetup(contextSetup);
		}

		// TODO: More specific test for wading in water.
		return this->CanWalkBackward();
	}

	bool PlayerContext::IsInNarrowSpace()
	{
		// HACK: coll->Setup.Radius is only set to LARA_RADIUS_CRAWL in lara_col functions, then reset by LaraAboveWater(),
		// meaning that for tests called in lara_as functions it will store the wrong radius. -- Sezz 2021.11.05
		static const std::vector<int> crouchStates = { LS_CROUCH_IDLE, LS_CROUCH_TURN_LEFT, LS_CROUCH_TURN_RIGHT };
		float radius = TestState(PlayerItemPtr->Animation.ActiveState, crouchStates) ? LARA_RADIUS_CRAWL : LARA_RADIUS;

		// Assess center point collision.
		auto pointCollCenter = GetCollision(PlayerItemPtr, 0, 0.0f, -LARA_HEIGHT / 2);
		if (abs(pointCollCenter.Position.Ceiling - pointCollCenter.Position.Floor) < LARA_HEIGHT ||	// Center space is narrow enough.
			abs(PlayerCollPtr->Middle.Ceiling - LARA_HEIGHT_CRAWL) < LARA_HEIGHT)					// Consider statics overhead detected by GetCollisionInfo().
		{
			return true;
		}

		// TODO: Check whether < or <= and > or >=.

		// Assess front point collision.
		auto pointCollFront = GetCollision(PlayerItemPtr, PlayerItemPtr->Pose.Orientation.y, radius, -PlayerCollPtr->Setup.Height);
		if (abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) < LARA_HEIGHT &&		  // Front space is narrow enough.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > LARA_HEIGHT_CRAWL &&	  // Front space not too narrow.
			abs(pointCollFront.Position.Floor - pointCollCenter.Position.Floor) <= CRAWL_STEPUP_HEIGHT && // Front floor is within upper/lower floor bounds.
			pointCollFront.Position.Floor != NO_HEIGHT)
		{
			return true;
		}

		// Assess back point collision.
		auto pointCollBack = GetCollision(PlayerItemPtr, PlayerItemPtr->Pose.Orientation.y, -radius, -PlayerCollPtr->Setup.Height);
		if (abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor) < LARA_HEIGHT &&			 // Back space is narrow enough.
			abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor) > LARA_HEIGHT_CRAWL &&	 // Back space not too narrow.
			abs(pointCollBack.Position.Floor - pointCollCenter.Position.Floor) <= CRAWL_STEPUP_HEIGHT && // Back floor is within upper/lower floor bounds.
			pointCollBack.Position.Floor != NO_HEIGHT)
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanCrouch()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		if (player->Control.WaterStatus != WaterStatus::Wade &&
			(player->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(PlayerItemPtr, player->Control.Weapon.GunType)))
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanCrouchToCrawl()
	{
		const auto& player = GetLaraInfo(PlayerItemPtr);

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
		static const float maxWaterHeight	 = -CLICK(1);
		static const float maxProbeDistance	 = SECTOR(1);
		static const float distanceIncrement = CLICK(1);

		const auto& player = GetLaraInfo(PlayerItemPtr);

		// 1. Check water depth.
		if (player->WaterSurfaceDist < maxWaterHeight)
			return false;

		// TODO: Extend point collision struct to also find water depths.
		// 2. Assess continuity of path.
		float distance = 0.0f;
		auto pointCollA = GetCollision(PlayerItemPtr);
		while (distance < maxProbeDistance)
		{
			distance += distanceIncrement;
			auto pointCollB = GetCollision(PlayerItemPtr, PlayerItemPtr->Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

			if (abs(pointCollA.Position.Floor - pointCollB.Position.Floor) > CRAWL_STEPUP_HEIGHT ||	 // Avoid floor height differences beyond crawl stepup threshold.
				abs(pointCollB.Position.Ceiling - pointCollB.Position.Floor) <= LARA_HEIGHT_CRAWL || // Avoid narrow spaces.
				pointCollB.Position.FloorSlope)														 // Avoid slopes.
			{
				return false;
			}

			pointCollA = pointCollB;
		}

		return true;
	}

	bool PlayerContext::CanCrawlForward()
	{
		Context::GroundMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl forward state.
		};
		return this->TestGroundMovementSetup(contextSetup, true);
	}

	bool PlayerContext::CanCrawlBackward()
	{
		Context::GroundMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + ANGLE(180.0f),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl backward state.
		};
		return this->TestGroundMovementSetup(contextSetup, true);
	}

	bool PlayerContext::CanGrabMonkeySwing()
	{
		static const float monkeySwingGrabTolerance = CLICK(0.5f);

		auto* player = GetLaraInfo(PlayerItemPtr);

		// 1. Check for monkey swing ceiling.
		if (!player->Control.CanMonkeySwing)
			return false;

		int vPos = PlayerItemPtr->Pose.Position.y - LARA_HEIGHT_MONKEY;
		auto pointColl = GetCollision(PlayerItemPtr);

		// 2. Assess collision with ceiling.
		if ((pointColl.Position.Ceiling - vPos) < 0 & PlayerCollPtr->CollisionType != CT_TOP && PlayerCollPtr->CollisionType != CT_TOP_FRONT)
			return false;

		// 3. Assess point collision.
		if ((pointColl.Position.Ceiling - vPos) <= monkeySwingGrabTolerance &&
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > LARA_HEIGHT_MONKEY)
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::CanMonkeyForward()
	{
		Context::MonkeyMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y,
			CLICK(1.25f), -CLICK(1.25f) // Defined by monkey forward state.
		};
		return TestMonkeyMovementSetup(contextSetup);
	}

	bool PlayerContext::CanMonkeyBackward()
	{
		Context::MonkeyMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + ANGLE(180.0f),
			CLICK(1.25f), -CLICK(1.25f) // Defined by monkey backward state.
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

	bool PlayerContext::CanPerformJump()
	{
		return !TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr);
	}

	bool PlayerContext::CanJumpUp()
	{
		static const Context::JumpSetup contextSetup =
		{
			0,
			0.0f,
			false
		};
		return TestJumpMovementSetup(contextSetup);
	}

	bool PlayerContext::CanJumpForward()
	{
		return this->TestDirectionalStandingJump(ANGLE(0.0f));
	}

	bool PlayerContext::CanJumpBackward()
	{
		return this->TestDirectionalStandingJump(ANGLE(180.0f));
	}

	bool PlayerContext::CanJumpLeft()
	{
		return this->TestDirectionalStandingJump(ANGLE(-90.0f));
	}

	bool PlayerContext::CanJumpRight()
	{
		return this->TestDirectionalStandingJump(ANGLE(90.0f));
	}

	bool PlayerContext::CanRunJumpForward()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		// Check run timer.
		if (player->Control.Count.Run < LARA_RUN_JUMP_TIME)
			return false;

		Context::JumpSetup contextSetup
		{
			PlayerItemPtr->Pose.Orientation.y,
			CLICK(1.5f)
		};
		return TestJumpMovementSetup(contextSetup);
	}
	
	bool PlayerContext::CanSprintJumpForward()
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		// Check for jump state dispatch.
		if (!HasStateDispatch(PlayerItemPtr, LS_JUMP_FORWARD))
			return false;

		// Check run timer.
		if (player->Control.Count.Run < LARA_SPRINT_JUMP_TIME)
			return false;

		Context::JumpSetup contextSetup
		{
			PlayerItemPtr->Pose.Orientation.y,
			CLICK(1.8f)
		};
		return TestJumpMovementSetup(contextSetup);
	}

	bool PlayerContext::CanSlideJumpForward()
	{
		return true;

		// TODO: Broken on diagonal slides?
		if (g_GameFlow->HasSlideExtended())
		{
			auto pointColl = GetCollision(PlayerItemPtr);

			short aspectAngle = GetLaraSlideDirection(PlayerItemPtr, PlayerCollPtr);
			short steepnessAngle = Geometry::GetSurfaceSteepnessAngle(pointColl.FloorTilt);
			return (abs(short(PlayerCollPtr->Setup.ForwardAngle - aspectAngle)) <= abs(steepnessAngle));
		}

		return true;
	}

	bool PlayerContext::CanCrawlspaceDive()
	{
		auto pointColl = GetCollision(PlayerItemPtr, PlayerCollPtr->Setup.ForwardAngle, PlayerCollPtr->Setup.Radius, -PlayerCollPtr->Setup.Height);
		return (abs(pointColl.Position.Ceiling - pointColl.Position.Floor) < LARA_HEIGHT || this->IsInNarrowSpace());
	}

	bool PlayerContext::TestSidestep(bool isGoingRight)
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		// TODO: Make specific condition for wading in water.
		if (player->Control.WaterStatus == WaterStatus::Wade &&
			TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
		{
			Context::GroundMovementSetup contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f)),
				NO_LOWER_BOUND, -CLICK(0.8f), // Upper bound defined by sidestep left/right states.
				false, false, false
			};
			return this->TestGroundMovementSetup(contextSetup);
		}
		else
		{
			Context::GroundMovementSetup contextSetup =
			{
				PlayerItemPtr->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f)),
				CLICK(0.8f), -CLICK(0.8f) // Defined by sidestep left/right states.
			};
			return this->TestGroundMovementSetup(contextSetup);
		}
	}

	bool PlayerContext::TestMonkeyShimmy(bool isGoingRight)
	{
		Context::MonkeyMovementSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f)),
			CLICK(0.5f), -CLICK(0.5f) // Defined by monkey shimmy left/right states.
		};
		return TestMonkeyMovementSetup(contextSetup);
	}

	bool PlayerContext::TestDirectionalStandingJump(short relativeHeadingAngle)
	{
		// Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr))
			return false;

		Context::JumpSetup contextSetup =
		{
			PlayerItemPtr->Pose.Orientation.y + relativeHeadingAngle,
			CLICK(0.85f)
		};
		return TestJumpMovementSetup(contextSetup);
	}

	Context::Vault PlayerContext::GetVaultUp2Steps()
	{
		static const Context::VaultSetup contextSetup =
		{
			-STEPUP_HEIGHT, -CLICK(2.5f), // Floor range.
			LARA_HEIGHT, -MAX_HEIGHT,	  // Space range.
			CLICK(1)
		};

		auto vaultContext = this->GetVault(contextSetup);
		vaultContext.TargetState = LS_VAULT_2_STEPS;

		if (!vaultContext.Success)
			return vaultContext;

		vaultContext.Success = HasStateDispatch(PlayerItemPtr, vaultContext.TargetState);
		vaultContext.Height += CLICK(2);
		vaultContext.SetBusyHands = true;
		vaultContext.DoLedgeSnap = true;
		vaultContext.SetJumpVelocity = false;
		return vaultContext;
	}

	bool PlayerContext::TestGroundMovementSetup(const Context::GroundMovementSetup& contextSetup, bool useCrawlSetup)
	{
		// HACK: PlayerCollPtr->Setup.Radius and PlayerCollPtr->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. -- Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : PlayerCollPtr->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : PlayerCollPtr->Setup.Height;

		int vPos = PlayerItemPtr->Pose.Position.y;
		int vPosTop = vPos - playerHeight;
		auto pointColl = GetCollision(PlayerItemPtr, contextSetup.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);

		// 1. Check for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		bool isSlopeDown  = contextSetup.TestSlopeDown  ? (pointColl.Position.FloorSlope && pointColl.Position.Floor > vPos) : false;
		bool isSlopeUp	  = contextSetup.TestSlopeUp	? (pointColl.Position.FloorSlope && pointColl.Position.Floor < vPos) : false;
		bool isDeathFloor = contextSetup.TestDeathFloor ? pointColl.Block->Flags.Death										 : false;

		// 2. Check for floor slope or death floor (if applicable).
		if (isSlopeDown || isSlopeUp || isDeathFloor)
			return false;

		// Raycast setup at upper floor bound.
		auto originA = GameVector(
			PlayerItemPtr->Pose.Position.x,
			(vPos + contextSetup.UpperFloorBound) - 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetA = GameVector(
			pointColl.Coordinates.x,
			(vPos + contextSetup.UpperFloorBound) - 1,
			pointColl.Coordinates.z,
			PlayerItemPtr->RoomNumber
		);

		// Raycast setup at lowest ceiling bound (player height).
		auto originB = GameVector(
			PlayerItemPtr->Pose.Position.x,
			vPosTop + 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetB = GameVector(
			pointColl.Coordinates.x,
			vPosTop + 1,
			pointColl.Coordinates.z,
			PlayerItemPtr->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&originA, &targetA) || !LOS(&originB, &targetB))
			return false;

		// 4. Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= contextSetup.LowerFloorBound &&	   // Floor is within lower floor bound.
			(pointColl.Position.Floor - vPos) >= contextSetup.UpperFloorBound &&	   // Floor is within upper floor bound.
			(pointColl.Position.Ceiling - vPos) < -playerHeight &&					   // Ceiling is within lowest ceiling bound (player height).
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > playerHeight) // Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::TestMonkeyMovementSetup(const Context::MonkeyMovementSetup& contextSetup)
	{
		// HACK: Have to make the height explicit for now (see comment in above function). -- Sezz 2022.07.28
		static const int playerHeight = LARA_HEIGHT_MONKEY;

		int vPos = PlayerItemPtr->Pose.Position.y;
		int vPosTop = vPos - playerHeight;
		auto pointColl = GetCollision(PlayerItemPtr, contextSetup.HeadingAngle, OFFSET_RADIUS(PlayerCollPtr->Setup.Radius));

		// 1. Check for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Check for ceiling slope.
		if (pointColl.Position.CeilingSlope)
			return false;

		// Raycast setup at highest floor bound (player base)
		auto originA = GameVector(
			PlayerItemPtr->Pose.Position.x,
			vPos - 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetA = GameVector(
			pointColl.Coordinates.x,
			vPos - 1,
			pointColl.Coordinates.z,
			PlayerItemPtr->RoomNumber
		);
		
		// Raycast setup at lower ceiling bound.
		auto originB = GameVector(
			PlayerItemPtr->Pose.Position.x,
			(vPosTop + contextSetup.LowerCeilingBound) + 1,
			PlayerItemPtr->Pose.Position.z,
			PlayerItemPtr->RoomNumber
		);
		auto targetB = GameVector(
			pointColl.Coordinates.x,
			(vPosTop + contextSetup.LowerCeilingBound) + 1,
			pointColl.Coordinates.z,
			PlayerItemPtr->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&originA, &targetA) || !LOS(&originB, &targetB))
			return false;

		// 4. Assess point collision.
		if (pointColl.BottomBlock->Flags.Monkeyswing &&								    // Ceiling is a monkey swing.
			(pointColl.Position.Ceiling - vPosTop) <= contextSetup.LowerCeilingBound &&	// Ceiling is within lower ceiling bound.
			(pointColl.Position.Ceiling - vPosTop) >= contextSetup.UpperCeilingBound &&	// Ceiling is within upper ceiling bound.
			(pointColl.Position.Floor - vPos) > 0 &&									// Floor is within highest floor bound (player base).
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > playerHeight)	// Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool PlayerContext::TestJumpMovementSetup(const Context::JumpSetup& contextSetup)
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		int vPos = PlayerItemPtr->Pose.Position.y;
		auto pointColl = GetCollision(PlayerItemPtr, contextSetup.HeadingAngle, contextSetup.Distance, -PlayerCollPtr->Setup.Height);

		// 1. Check for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr);
		bool isWading = contextSetup.TestWadeStatus ? (player->Control.WaterStatus == WaterStatus::Wade) : false;

		// 2. Check for swamp or wade status (if applicable).
		if (isSwamp || isWading)
			return false;

		// 3. Check for corner.
		if (TestLaraFacingCorner(PlayerItemPtr, contextSetup.HeadingAngle, contextSetup.Distance))
			return false;

		// 4. Assess point collision.
		if ((pointColl.Position.Floor - vPos) >= -STEPUP_HEIGHT &&											  // Floor is within highest floor bound.
			((pointColl.Position.Ceiling - vPos) < -(PlayerCollPtr->Setup.Height + (LARA_HEADROOM * 0.8f)) || // Ceiling is within lowest ceiling bound... 
				((pointColl.Position.Ceiling - vPos) < -PlayerCollPtr->Setup.Height &&								// OR ceiling is level with Lara's head...
					(pointColl.Position.Floor - vPos) >= CLICK(0.5f))))												// AND there is a drop below.
		{
			return true;
		}

		return false;
	}

	Context::Vault PlayerContext::GetVault(const Context::VaultSetup& contextSetup)
	{
		auto* player = GetLaraInfo(PlayerItemPtr);

		auto pointCollFront = GetCollision(PlayerItemPtr, PlayerCollPtr->NearestLedgeAngle, OFFSET_RADIUS(PlayerCollPtr->Setup.Radius), -PlayerCollPtr->Setup.Height);
		auto pointCollCenter = GetCollision(PlayerItemPtr, 0, 0, --PlayerCollPtr->Setup.Height / 2);

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, PlayerItemPtr);
		bool isSwampTooDeep = contextSetup.TestSwampDepth ? (isSwamp && player->WaterSurfaceDist < -CLICK(3)) : isSwamp;
		
		int vPos = isSwamp ? PlayerItemPtr->Pose.Position.y : pointCollCenter.Position.Floor; // HACK: Avoid cheese when in the midst of performing a step. Can be done better. @Sezz 2022.04.08	

		// 1. Check swamp depth (if applicable).
		if (isSwampTooDeep)
			return Context::Vault{ false };

		// NOTE: Where the point/room probe finds that
		// a) the "wall" in front is formed by a ceiling, or
		// b) the space between the floor and ceiling is a clamp (i.e. it is too narrow),
		// any potentially climbable floor in a room above will be missed. The following step considers this.

		// 2. Raise vertical position of point collision probe by increments of 1/8th blocks to find potential vault ledge.
		int vOffset = contextSetup.LowerFloorBound;
		while (((pointCollFront.Position.Ceiling - vPos) > -PlayerCollPtr->Setup.Height ||					 // Ceiling is below Lara's height...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.ClampMin/* ||	// OR clamp is too small...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.ClampMax*/) &&	// OR clamp is too large (future-proofing; not possible right now).
			vOffset > (contextSetup.UpperFloorBound - PlayerCollPtr->Setup.Height))							 // Offset is not too high.
		{
			pointCollFront = GetCollision(PlayerItemPtr, PlayerCollPtr->NearestLedgeAngle, OFFSET_RADIUS(PlayerCollPtr->Setup.Radius), vOffset);
			vOffset -= std::max<int>(CLICK(0.5f), contextSetup.ClampMin);
		}

		// 3. Check for walls.
		if (pointCollFront.Position.Floor == NO_HEIGHT)
			return Context::Vault{ false };

		// 4. Assess point collision.
		if ((pointCollFront.Position.Floor - vPos) < contextSetup.LowerFloorBound &&						 // Floor is within lower floor bound.
			(pointCollFront.Position.Floor - vPos) >= contextSetup.UpperFloorBound &&						 // Floor is within upper floor bound.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.ClampMin &&	 // Space is not too narrow..
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.ClampMax && // Space is not too wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= contextSetup.GapMin)	 // Gap is visually permissive.
		{
			return Context::Vault{ true, pointCollFront.Position.Floor };
		}

		return Context::Vault{ false };
	}
}
