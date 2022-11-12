#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;

namespace TEN::Entities::Player::Context
{
	// -----------------------------
	// CONTEXT ASSESSMENT FUNCTIONS
	// For State Control & Collision
	// -----------------------------

	bool CanPerformStep(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		if (abs(coll->Middle.Floor) > 0 &&
			(coll->Middle.Floor <= STEPUP_HEIGHT ||					// Within lower floor bound...
				player.Control.WaterStatus == WaterStatus::Wade) &&		// OR Lara is wading.
			coll->Middle.Floor >= -STEPUP_HEIGHT &&					// Within upper floor bound.
			coll->Middle.Floor != NO_HEIGHT)
		{
			return true;
		}

		return false;
	}

	bool CanStepUp(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto lowerFloorBound = -CLICK(1.0f / 2);
		static constexpr auto upperFloorBound = -STEPUP_HEIGHT;

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);

		// Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= lowerFloorBound && // Floor height is within lower floor bound.
			(pointColl.Position.Floor - vPos) >= upperFloorBound)	// Floor height is within upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStepDown(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto lowerFloorBound = STEPUP_HEIGHT;
		static constexpr auto upperFloorBound = CLICK(1.0f / 2);

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);

		if ((pointColl.Position.Floor - vPos) <= lowerFloorBound && // Floor height is within lower floor bound.
			(pointColl.Position.Floor - vPos) >= upperFloorBound)	// Floor height is within upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanFall(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto lowerFloorBound = STEPUP_HEIGHT;

		const auto& player = *GetLaraInfo(item);

		// 1. Check wade status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);

		// 2. Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= lowerFloorBound)
			return false;

		return true;
	}

	bool CanSlide(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto floorBound = STEPUP_HEIGHT;

		// 1. Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			return false;

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);

		// 2. Assess point collision.
		if (abs(pointColl.Position.Floor - vPos) <= floorBound && // Floor height is within upper/lower floor bound.
			pointColl.Position.FloorSlope)						  // Floor is a slope.
		{
			return true;
		}

		return false;
	}

	bool CanAFKPose(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// 1. Check whether AFK pose is enabled.
		if (!g_GameFlow->HasAFKPose())
			return false;

		// 2. Check AFK pose timer.
		if (player.Control.Count.Pose < LARA_POSE_TIME)
			return false;

		// 3. Check hand and water status.
		if (player.Control.HandStatus != HandStatus::Free ||
			player.Control.WaterStatus == WaterStatus::Wade)
		{
			return false;
		}

		// 4. Assess context.
		if (!(IsHeld(In::Flare) || IsHeld(In::DrawWeapon)) &&		   // Avoid unsightly concurrent actions.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare.
				player.Flare.Life) &&
			player.Vehicle == NO_ITEM)								   // Not in a vehicle.
		{
			return true;
		}

		return false;
	}

	bool CanTurn180(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& lara = *GetLaraInfo(item);

		return (lara.Control.WaterStatus == WaterStatus::Wade || TestEnvironment(ENV_FLAG_SWAMP, item));
	}

	bool CanTurnFast(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		if (player.Control.WaterStatus == WaterStatus::Dry &&
			((player.Control.HandStatus == HandStatus::WeaponReady && player.Control.Weapon.GunType != LaraWeaponType::Torch) ||
				(player.Control.HandStatus == HandStatus::WeaponDraw && player.Control.Weapon.GunType != LaraWeaponType::Flare)))
		{
			return true;
		}

		return false;
	}

	bool CanRunForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::GroundMovementSetup
		{
			item->Pose.Orientation.y,
			INFINITY, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};
		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool CanRunBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::GroundMovementSetup
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			INFINITY, -STEPUP_HEIGHT, // Defined by run backward state.
			false, false, false
		};
		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool CanWalkForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::GroundMovementSetup
		{
			item->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // Defined by walk forward state.
		};
		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool CanWalkBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::GroundMovementSetup
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // Defined by walk backward state.
		};
		return TestGroundMovementSetup(item, coll, contextSetup);
	}

	bool CanSidestepLeft(ItemInfo* item, CollisionInfo* coll)
	{
		return TestSidestep(item, coll, false);
	}

	bool CanSidestepRight(ItemInfo* item, CollisionInfo* coll)
	{
		return TestSidestep(item, coll, true);
	}

	bool CanWadeForward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// Check water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		// Swamp case.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			auto contextSetup = Context::GroundMovementSetup
			{
				item->Pose.Orientation.y,
				INFINITY, -STEPUP_HEIGHT, // Defined by wade forward state.
				false, false, false
			};
			return TestGroundMovementSetup(item, coll, contextSetup);
		}
		// Regular case.
		else
			return CanRunForward(item, coll); // TODO: More specific test for wading in water.
	}

	bool CanWadeBackward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// Check water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		// Swamp case.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			auto contextSetup = Context::GroundMovementSetup
			{
				short(item->Pose.Orientation.y + ANGLE(180.0f)),
				INFINITY, -STEPUP_HEIGHT, // Defined by walk backward state.
				false, false, false
			};
			return TestGroundMovementSetup(item, coll, contextSetup);
		}
		// Regular case.
		else
			return CanWalkBackward(item, coll); // TODO: More specific test for wading in water.
	}

	bool IsInNarrowSpace(ItemInfo* item, CollisionInfo* coll)
	{
		static const std::vector<int> crouchStates = { LS_CROUCH_IDLE, LS_CROUCH_TURN_LEFT, LS_CROUCH_TURN_RIGHT };

		// HACK: coll->Setup.Radius is only set to LARA_RADIUS_CRAWL in lara_col functions, then reset by LaraAboveWater(),
		// meaning that for tests called in lara_as functions it will store the wrong radius. -- Sezz 2021.11.05
		float radius = TestState(item->Animation.ActiveState, crouchStates) ? LARA_RADIUS_CRAWL : LARA_RADIUS;

		// Assess center point collision.
		auto pointCollCenter = GetCollision(item, 0, 0.0f, -LARA_HEIGHT / 2);
		if (abs(pointCollCenter.Position.Ceiling - pointCollCenter.Position.Floor) < LARA_HEIGHT ||	// Center space is narrow enough.
			abs(coll->Middle.Ceiling - LARA_HEIGHT_CRAWL) < LARA_HEIGHT)							// Consider statics overhead detected by GetCollisionInfo().
		{
			return true;
		}

		// TODO: Check whether < or <= and > or >=.

		// Assess front point collision.
		auto pointCollFront = GetCollision(item, item->Pose.Orientation.y, radius, -coll->Setup.Height);
		if (abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) < LARA_HEIGHT &&		  // Front space is narrow enough.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > LARA_HEIGHT_CRAWL &&	  // Front space not too narrow.
			abs(pointCollFront.Position.Floor - pointCollCenter.Position.Floor) <= CRAWL_STEPUP_HEIGHT && // Front floor is within upper/lower floor bounds.
			pointCollFront.Position.Floor != NO_HEIGHT)
		{
			return true;
		}

		// Assess back point collision.
		auto pointCollBack = GetCollision(item, item->Pose.Orientation.y, -radius, -coll->Setup.Height);
		if (abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor) < LARA_HEIGHT &&			 // Back space is narrow enough.
			abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor) > LARA_HEIGHT_CRAWL &&	 // Back space not too narrow.
			abs(pointCollBack.Position.Floor - pointCollCenter.Position.Floor) <= CRAWL_STEPUP_HEIGHT && // Back floor is within upper/lower floor bounds.
			pointCollBack.Position.Floor != NO_HEIGHT)
		{
			return true;
		}

		return false;
	}

	bool CanCrouch(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		if (player.Control.WaterStatus != WaterStatus::Wade &&
			(player.Control.HandStatus == HandStatus::Free || !IsStandingWeapon(item, player.Control.Weapon.GunType)))
		{
			return true;
		}

		return false;
	}

	bool CanCrouchToCrawl(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		if (!(IsHeld(In::Flare) || IsHeld(In::DrawWeapon)) &&		   // Avoid unsightly concurrent actions.
			player.Control.HandStatus == HandStatus::Free &&		   // Hands are free.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare. TODO: Should be allowed, but the flare animation bugs out right now. -- Sezz 2022.03.18
				player.Flare.Life))
		{
			return true;
		}

		return false;
	}

	bool CanCrouchRoll(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto maxWaterHeight = -CLICK(1);
		static constexpr auto maxProbeDist	 = BLOCK(1);
		static constexpr auto distIncrement	 = BLOCK(1.0f / 4);

		const auto& player = *GetLaraInfo(item);

		// 1. Check whether crouch roll is enabled.
		if (!g_GameFlow->HasCrouchRoll())
			return false;

		// 2. Check water depth.
		if (player.WaterSurfaceDist < maxWaterHeight)
			return false;

		// TODO: Extend point collision struct to also find water depths.
		// 3. Assess continuity of path.
		float distance = 0.0f;
		auto pointCollA = GetCollision(item);
		while (distance < maxProbeDist)
		{
			distance += distIncrement;
			auto pointCollB = GetCollision(item, item->Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

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

	bool CanCrawlForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::GroundMovementSetup
		{
			item->Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl forward state.
		};
		return TestGroundMovementSetup(item, coll, contextSetup, true);
	}

	bool CanCrawlBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::GroundMovementSetup
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl backward state.
		};
		return TestGroundMovementSetup(item, coll, contextSetup, true);
	}

	bool CanPerformMonkeyStep(ItemInfo* item, CollisionInfo* coll)
	{
		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
		auto pointColl = GetCollision(item);

		// 1. Check for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Assess point collision.
		if ((pointColl.Position.Ceiling - vPosTop) <= MONKEY_STEPUP_HEIGHT && // Ceiling height is within lower ceiling bound.
			(pointColl.Position.Ceiling - vPosTop) >= -MONKEY_STEPUP_HEIGHT)  // Ceiling height is within upper ceiling bound.
		{
			return true;
		}

		return false;
	}

	bool CanFallFromMonkeySwing(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto ceilingBound = CLICK(5.0f / 4);

		auto& player = *GetLaraInfo(item);

		// 1. Check for monkey swing ceiling.
		if (!player.Control.CanMonkeySwing)
			return true;

		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
		auto pointColl = GetCollision(item);

		// 2. Check for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return true;

		// 3. Check ceiling slope if overhang climb is disabled.
		if (pointColl.Position.CeilingSlope && !g_GameFlow->HasOverhangClimb())
			return true;

		// 4. Assess point collision.
		if ((pointColl.Position.Ceiling - vPosTop) > ceilingBound || // Ceiling height is below lower bound.
			(pointColl.Position.Ceiling - vPosTop) < -ceilingBound)	 // Ceiling height is beyond upper bound.
		{
			return true;
		}

		return false;
	}

	bool CanGrabMonkeySwing(ItemInfo* item, CollisionInfo* coll)
	{
		static const float grabHeightTolerance = CLICK(1.0f / 2);

		const auto& player = *GetLaraInfo(item);

		// 1. Check for monkey swing ceiling.
		if (!player.Control.CanMonkeySwing)
			return false;

		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
		auto pointColl = GetCollision(item);

		// 2. Assess collision with ceiling.
		if ((pointColl.Position.Ceiling - vPosTop) < 0 &&
			coll->CollisionType != CollisionType::CT_TOP &&
			coll->CollisionType != CollisionType::CT_TOP_FRONT)
		{
			return false;
		}

		// 3. Assess point collision.
		if ((pointColl.Position.Ceiling - vPosTop) <= grabHeightTolerance &&
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > LARA_HEIGHT_MONKEY)
		{
			return true;
		}

		return false;
	}

	bool CanMonkeyForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::MonkeyMovementSetup
		{
			item->Pose.Orientation.y,
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // Defined by monkey forward state.
		};
		return TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool CanMonkeyBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::MonkeyMovementSetup
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // Defined by monkey backward state.
		};
		return TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool CanMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll)
	{
		return TestMonkeyShimmy(item, coll, false);
	}
	
	bool CanMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll)
	{
		return TestMonkeyShimmy(item, coll, true);
	}

	bool CanLand(ItemInfo* item, CollisionInfo* coll)
	{
		// 1. Check airborne status and Y velocity.
		if (!item->Animation.IsAirborne || item->Animation.Velocity.y < 0.0f)
			return false;

		// 2. Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			return true;

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item);

		// 3. Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= item->Animation.Velocity.y)
			return true;

		return false;
	}

	bool CanPerformJump(ItemInfo* item, CollisionInfo* coll)
	{
		return !TestEnvironment(ENV_FLAG_SWAMP, item);
	}

	bool CanJumpUp(ItemInfo* item, CollisionInfo* coll)
	{
		static const auto contextSetup = Context::JumpSetup
		{
			0,
			0.0f,
			false
		};
		return TestJumpSetup(item, coll, contextSetup);
	}

	bool CanJumpForward(ItemInfo* item, CollisionInfo* coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(0.0f));
	}

	bool CanJumpBackward(ItemInfo* item, CollisionInfo* coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(180.0f));
	}

	bool CanJumpLeft(ItemInfo* item, CollisionInfo* coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(-90.0f));
	}

	bool CanJumpRight(ItemInfo* item, CollisionInfo* coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(90.0f));
	}

	bool CanRunJumpForward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// Check run timer.
		if (player.Control.Count.Run < LARA_RUN_JUMP_TIME)
			return false;

		auto contextSetup = Context::JumpSetup
		{
			item->Pose.Orientation.y,
			CLICK(3.0f / 2)
		};
		return TestJumpSetup(item, coll, contextSetup);
	}
	
	bool CanSprintJumpForward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// Check for jump state dispatch.
		if (!HasStateDispatch(item, LS_JUMP_FORWARD))
			return false;

		// Check run timer.
		if (player.Control.Count.Run < LARA_SPRINT_JUMP_TIME)
			return false;

		auto contextSetup = Context::JumpSetup
		{
			item->Pose.Orientation.y,
			CLICK(1.8f)
		};
		return TestJumpSetup(item, coll, contextSetup);
	}

	bool CanSlideJumpForward(ItemInfo* item, CollisionInfo* coll)
	{
		// TODO: Get back to this project. -- Sezz 2022.11.11
		return true;

		// Check whether extended slide mechanics are enabled.
		if (!g_GameFlow->HasSlideExtended())
			return true;
		
		// TODO: Broken on diagonal slides?

		auto pointColl = GetCollision(item);

		short aspectAngle = GetLaraSlideHeadingAngle(item, coll);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(Geometry::GetFloorNormal(pointColl.FloorTilt));
		return (abs(short(coll->Setup.ForwardAngle - aspectAngle)) <= abs(slopeAngle));
	}

	bool CanCrawlspaceDive(ItemInfo* item, CollisionInfo* coll)
	{
		auto pointColl = GetCollision(item, coll->Setup.ForwardAngle, coll->Setup.Radius, -coll->Setup.Height);
		return (abs(pointColl.Position.Ceiling - pointColl.Position.Floor) < LARA_HEIGHT || IsInNarrowSpace(item, coll));
	}

	bool TestSidestep(ItemInfo* item, CollisionInfo* coll, bool isGoingRight)
	{
		const auto& player = *GetLaraInfo(item);

		// TODO: Make specific condition for wading in water.
		if (player.Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			auto contextSetup = Context::GroundMovementSetup
			{
				short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				INFINITY, -CLICK(5.0f / 4), // Upper bound defined by sidestep left/right states.
				false, false, false
			};
			return TestGroundMovementSetup(item, coll, contextSetup);
		}
		else
		{
			auto contextSetup = Context::GroundMovementSetup
			{
				short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				CLICK(5.0f / 4), -CLICK(5.0f / 4) // Defined by sidestep left/right states.
			};
			return TestGroundMovementSetup(item, coll, contextSetup);
		}
	}

	bool TestMonkeyShimmy(ItemInfo* item, CollisionInfo* coll, bool isGoingRight)
	{
		auto contextSetup = Context::MonkeyMovementSetup
		{
			short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
			CLICK(1.0f / 2), -CLICK(1.0f / 2) // Defined by monkey shimmy left/right states.
		};
		return TestMonkeyMovementSetup(item, coll, contextSetup);
	}

	bool TestDirectionalStandingJump(ItemInfo* item, CollisionInfo* coll, short relativeHeadingAngle)
	{
		// Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			return false;

		auto contextSetup = Context::JumpSetup
		{
			short(item->Pose.Orientation.y + relativeHeadingAngle),
			CLICK(0.85f)
		};
		return TestJumpSetup(item, coll, contextSetup);
	}

	Context::Vault GetVaultUp2Steps(ItemInfo* item, CollisionInfo* coll)
	{
		static const auto contextSetup = Context::VaultSetup
		{
			-STEPUP_HEIGHT, -CLICK(5.0f / 2), // Floor range.
			LARA_HEIGHT, INFINITY,			  // Space range.
			CLICK(1)
		};
		auto vaultContext = GetVault(item, coll, contextSetup);
		vaultContext.TargetState = LS_VAULT_2_STEPS;

		if (!vaultContext.Success)
			return vaultContext;

		vaultContext.Success = HasStateDispatch(item, vaultContext.TargetState);
		vaultContext.Height += CLICK(2);
		vaultContext.SetBusyHands = true;
		vaultContext.DoLedgeSnap = true;
		vaultContext.SetJumpVelocity = false;
		return vaultContext;
	}

	bool TestGroundMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::GroundMovementSetup& contextSetup, bool useCrawlSetup)
	{
		// HACK: coll->Setup.Radius and coll->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. -- Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

		int vPos = item->Pose.Position.y;
		int vPosTop = vPos - playerHeight;
		auto pointColl = GetCollision(item, contextSetup.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);

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
			item->Pose.Position.x,
			(vPos + contextSetup.UpperFloorBound) - 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetA = GameVector(
			pointColl.Coordinates.x,
			(vPos + contextSetup.UpperFloorBound) - 1,
			pointColl.Coordinates.z,
			item->RoomNumber
		);

		// Raycast setup at lowest ceiling bound (player height).
		auto originB = GameVector(
			item->Pose.Position.x,
			vPosTop + 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetB = GameVector(
			pointColl.Coordinates.x,
			vPosTop + 1,
			pointColl.Coordinates.z,
			item->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&originA, &targetA) || !LOS(&originB, &targetB))
			return false;

		// 4. Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= contextSetup.LowerFloorBound &&	   // Floor height is within lower floor bound.
			(pointColl.Position.Floor - vPos) >= contextSetup.UpperFloorBound &&	   // Floor height is within upper floor bound.
			(pointColl.Position.Ceiling - vPos) < -playerHeight &&					   // Ceiling height is within lowest ceiling bound (player height).
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > playerHeight) // Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool TestMonkeyMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::MonkeyMovementSetup& contextSetup)
	{
		// HACK: Have to make the height explicit for now (see comment in above function). -- Sezz 2022.07.28
		static const int playerHeight = LARA_HEIGHT_MONKEY;

		int vPos = item->Pose.Position.y;
		int vPosTop = vPos - playerHeight;
		auto pointColl = GetCollision(item, contextSetup.HeadingAngle, OFFSET_RADIUS(coll->Setup.Radius));

		// 1. Check for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Check for ceiling slope.
		if (pointColl.Position.CeilingSlope)
			return false;

		// Raycast setup at highest floor bound (player base).
		auto originA = GameVector(
			item->Pose.Position.x,
			vPos - 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetA = GameVector(
			pointColl.Coordinates.x,
			vPos - 1,
			pointColl.Coordinates.z,
			item->RoomNumber
		);
		
		// Raycast setup at lower ceiling bound.
		auto originB = GameVector(
			item->Pose.Position.x,
			(vPosTop + contextSetup.LowerCeilingBound) + 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto targetB = GameVector(
			pointColl.Coordinates.x,
			(vPosTop + contextSetup.LowerCeilingBound) + 1,
			pointColl.Coordinates.z,
			item->RoomNumber
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

	bool TestJumpSetup(ItemInfo* item, CollisionInfo* coll, const Context::JumpSetup& contextSetup)
	{
		const auto& player = *GetLaraInfo(item);

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item, contextSetup.HeadingAngle, contextSetup.Distance, -coll->Setup.Height);

		// 1. Check for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
		bool isWading = contextSetup.TestWadeStatus ? (player.Control.WaterStatus == WaterStatus::Wade) : false;

		// 2. Check for swamp or wade status (if applicable).
		if (isSwamp || isWading)
			return false;

		// 3. Check for corner.
		if (TestLaraFacingCorner(item, contextSetup.HeadingAngle, contextSetup.Distance))
			return false;

		// 4. Assess point collision.
		if ((pointColl.Position.Floor - vPos) >= -STEPUP_HEIGHT &&									 // Floor is within highest floor bound.
			((pointColl.Position.Ceiling - vPos) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) || // Ceiling is within lowest ceiling bound... 
				((pointColl.Position.Ceiling - vPos) < -coll->Setup.Height &&							// OR ceiling is level with Lara's head...
					(pointColl.Position.Floor - vPos) >= CLICK(1.0f / 2))))								// AND there is a drop below.
		{
			return true;
		}

		return false;
	}

	Context::Vault GetVault(ItemInfo* item, CollisionInfo* coll, const Context::VaultSetup& contextSetup)
	{
		const auto& player = *GetLaraInfo(item);

		auto pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), -coll->Setup.Height);
		auto pointCollCenter = GetCollision(item, 0, 0, -coll->Setup.Height / 2);

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
		bool isSwampTooDeep = contextSetup.TestSwampDepth ? (isSwamp && player.WaterSurfaceDist < -CLICK(3)) : isSwamp;
		
		int vPos = isSwamp ? item->Pose.Position.y : pointCollCenter.Position.Floor; // HACK: Avoid cheese when in the midst of performing a step. Can be done better. @Sezz 2022.04.08	

		// 1. Check swamp depth (if applicable).
		if (isSwampTooDeep)
			return Context::Vault{ false };

		// NOTE: Where the point collision probe finds that
		// a) the "wall" in front is formed by a ceiling, or
		// b) the space between the floor and ceiling is too narrow,
		// any potentially climbable floor in a room above will be missed. The following step considers this.

		// 2. Raise vertical position of point collision probe by increments of 1/8th blocks to find potential vault ledge.
		int vOffset = contextSetup.LowerFloorBound;
		while (((pointCollFront.Position.Ceiling - vPos) > -coll->Setup.Height ||					 // Ceiling is below Lara's height...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.SpaceMin/* ||	// OR space is too small...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.ClampMax*/) &&	// OR space is too large (future-proofing; not possible right now).
			vOffset > (contextSetup.UpperFloorBound - coll->Setup.Height))							 // Offset is not too high.
		{
			pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), vOffset);
			vOffset -= std::max(CLICK(1.0f / 2), contextSetup.SpaceMin);
		}

		// 3. Check for wall.
		if (pointCollFront.Position.Floor == NO_HEIGHT)
			return Context::Vault{ false };

		// 4. Assess point collision.
		if ((pointCollFront.Position.Floor - vPos) < contextSetup.LowerFloorBound &&						 // Floor is within lower floor bound.
			(pointCollFront.Position.Floor - vPos) >= contextSetup.UpperFloorBound &&						 // Floor is within upper floor bound.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.SpaceMin &&	 // Space is not too narrow..
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.SpaceMax && // Space is not too wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= contextSetup.GapMin)	 // Gap is visually permissive.
		{
			return Context::Vault{ true, pointCollFront.Position.Floor };
		}

		return Context::Vault{ false };
	}
}
