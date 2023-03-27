#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"

using namespace TEN::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Player::Context
{
	// -----------------------------
	// CONTEXT ASSESSMENT FUNCTIONS
	// For State Control & Collision
	// -----------------------------

	bool CanPerformStep(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND = -STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(*item);

		// Get point collision.
		auto pointColl = GetCollision(item);
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;

		// 1. Test for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		// 2. Test if player is already aligned with floor.
		if (relFloorHeight == 0)
			return false;

		// 3. Assess point collision and player status.
		if ((relFloorHeight <= LOWER_FLOOR_BOUND ||					// Floor height is higher than lower floor bound...
				player.Control.WaterStatus == WaterStatus::Wade) && // OR player is wading.
			relFloorHeight >= UPPER_FLOOR_BOUND)					// Floor height is lower than upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStepUp(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = -CLICK(0.5f);
		constexpr auto UPPER_FLOOR_BOUND = -STEPUP_HEIGHT;

		// Get point collision.
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;

		// Assess point collision.
		if (relFloorHeight <= LOWER_FLOOR_BOUND && // Floor height is higher than lower floor bound.
			relFloorHeight >= UPPER_FLOOR_BOUND)   // Floor height is lower than upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStepDown(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND = CLICK(0.5f);

		// Get point collision.
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;

		// Assess point collision.
		if (relFloorHeight <= LOWER_FLOOR_BOUND && // Floor height is higher than lower floor bound.
			relFloorHeight >= UPPER_FLOOR_BOUND)   // Floor height is lower than upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStrikeAFKPose(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = GetLaraInfo(*item);

		// 1. Check if AFK pose is enabled.
		if (!g_GameFlow->HasAFKPose())
			return false;

		// 2. Test AFK pose timer.
		if (player.Control.Count.Pose < LARA_POSE_TIME)
			return false;

		// 3. Test player hand and water status.
		if (player.Control.HandStatus != HandStatus::Free ||
			player.Control.WaterStatus == WaterStatus::Wade)
		{
			return false;
		}

		// 4. Assess player status.
		if (!(IsHeld(In::Flare) || IsHeld(In::DrawWeapon)) &&		   // Avoid unsightly concurrent actions.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Player not handling flare...
				player.Flare.Life) &&								   // OR flare is still active.
			player.Vehicle == NO_ITEM)								   // Player not in a vehicle.
		{
			return true;
		}

		return false;
	}

	bool CanTurn180(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = GetLaraInfo(*item);

		// Assess player status.
		if (player.Control.WaterStatus == WaterStatus::Wade || // Player is wading...
			TestEnvironment(ENV_FLAG_SWAMP, item))			   // OR player is in swamp.
		{
			return true;
		}

		return false;
	}

	bool CanTurnFast(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = GetLaraInfo(*item);

		// Assess player status.
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
		auto setupData = Context::LandMovementSetupData
		{
			item->Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};

		return Context::TestGroundSetup(item, coll, setupData);
	}

	bool CanRunBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // Defined by run backward state.
			false, false, false
		};

		return Context::TestGroundSetup(item, coll, setupData);
	}

	bool CanWalkForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			item->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // Defined by walk forward state.
		};

		return Context::TestGroundSetup(item, coll, setupData);
	}

	bool CanWalkBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // Defined by walk backward state.
		};

		return Context::TestGroundSetup(item, coll, setupData);
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
		const auto& player = GetLaraInfo(*item);

		// 1. Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		// 2.1. Assess context in swamp case.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			auto contextSetup = Context::LandMovementSetupData
			{
				item->Pose.Orientation.y,
				-MAX_HEIGHT, -STEPUP_HEIGHT, // Defined by wade forward state.
				false, false, false
			};

			return Context::TestGroundSetup(item, coll, contextSetup);
		}

		// 2.2. Assess context in regular case.
		return CanRunForward(item, coll); // TODO: More specific test for wading in water.
	}

	bool CanWadeBackward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// 1. Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		// 2.1. Assess context in swamp case.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			auto contextSetup = Context::LandMovementSetupData
			{
				short(item->Pose.Orientation.y + ANGLE(180.0f)),
				-MAX_HEIGHT, -STEPUP_HEIGHT, // Defined by walk backward state.
				false, false, false
			};

			return Context::TestGroundSetup(item, coll, contextSetup);
		}

		// 2.2. Assess context in regular case.
		return CanWalkBackward(item, coll); // TODO: More specific test for wading in water.
	}

	bool CanSlide(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto FLOOR_BOUND = STEPUP_HEIGHT;

		// 1. Check if player is in swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			return false;

		// Get point collision.
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;

		// 2. Assess point collision.
		if (abs(relFloorHeight) <= FLOOR_BOUND && // Floor height is within upper/lower floor bounds.
			pointColl.Position.FloorSlope)		  // Floor is a slippery slope.
		{
			return true;
		}

		return false;
	}

	bool CanSteerOnSlide(ItemInfo* item, CollisionInfo* coll)
	{
		return g_GameFlow->HasSlideExtended();
	}

	bool IsInNarrowSpace(ItemInfo* item, CollisionInfo* coll)
	{
		static const auto crouchStates = std::vector<int>
		{
			LS_CROUCH_IDLE,
			LS_CROUCH_TURN_LEFT,
			LS_CROUCH_TURN_RIGHT
		};

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
		const auto& player = GetLaraInfo(*item);

		// Assess player status.
		if (player.Control.WaterStatus != WaterStatus::Wade &&
			(player.Control.HandStatus == HandStatus::Free ||
				!IsStandingWeapon(item, player.Control.Weapon.GunType)))
		{
			return true;
		}

		return false;
	}

	bool CanCrouchToCrawl(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = GetLaraInfo(*item);

		// Assess player status.
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
		constexpr auto FLOOR_BOUND					  = CRAWL_STEPUP_HEIGHT;
		constexpr auto FLOOR_TO_CEIL_HEIGHT_DELTA_MAX = LARA_HEIGHT_CRAWL;
		constexpr auto WATER_HEIGHT_MAX				  = -CLICK(1);
		constexpr auto PROBE_DIST_MAX				  = BLOCK(1);
		constexpr auto STEP_DIST					  = BLOCK(0.25f);

		const auto& player = GetLaraInfo(*item);

		// 1. Check if crouch roll is enabled.
		if (!g_GameFlow->HasCrouchRoll())
			return false;

		// 2. Test water depth.
		if (player.WaterSurfaceDist < WATER_HEIGHT_MAX)
			return false;

		// TODO: Extend point collision struct to also find water depths.
		float distance = 0.0f;
		auto pointColl0 = GetCollision(item);

		// 3. Test continuity of path.
		while (distance < PROBE_DIST_MAX)
		{
			// Get point collision.
			distance += STEP_DIST;
			auto pointColl1 = GetCollision(item, item->Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

			int floorHeightDelta = abs(pointColl0.Position.Floor - pointColl1.Position.Floor);
			int floorToCeilHeight = abs(pointColl1.Position.Ceiling - pointColl1.Position.Floor);

			// Assess point collision.
			if (floorHeightDelta > FLOOR_BOUND ||					   // Avoid floor height delta beyond crawl stepup threshold.
				floorToCeilHeight <= FLOOR_TO_CEIL_HEIGHT_DELTA_MAX || // Avoid narrow spaces.
				pointColl1.Position.FloorSlope)						   // Avoid slippery floor slopes.
			{
				return false;
			}

			pointColl0 = std::move(pointColl1);
		}

		return true;
	}

	bool CanCrawlForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::LandMovementSetupData
		{
			item->Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl forward state.
		};
		return Context::TestGroundSetup(item, coll, contextSetup, true);
	}

	bool CanCrawlBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // Defined by crawl backward state.
		};
		return Context::TestGroundSetup(item, coll, contextSetup, true);
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
		static constexpr auto ceilingBound = CLICK(5 / 4.0f);

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
		static const float grabHeightTolerance = CLICK(1 / 2.0f);

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
		auto contextSetup = Context::MonkeySwingSetup
		{
			item->Pose.Orientation.y,
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // Defined by monkey forward state.
		};
		return Context::TestMonkeySwingSetup(item, coll, contextSetup);
	}

	bool CanMonkeyBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::MonkeySwingSetup
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // Defined by monkey backward state.
		};
		return Context::TestMonkeySwingSetup(item, coll, contextSetup);
	}

	bool CanMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll)
	{
		return Context::TestMonkeyShimmy(item, coll, false);
	}
	
	bool CanMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll)
	{
		return Context::TestMonkeyShimmy(item, coll, true);
	}

	bool CanSwingOnLedge(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = *GetLaraInfo(item);

		int vPos = item->Pose.Position.y;
		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_REACH;
		auto pointColl = GetCollision(item, item->Pose.Orientation.y, BLOCK(1.0f / 4));

		// 1. Check for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		// 2. Assess point collision.
		if ((pointColl.Position.Floor - vPos) >= 0 &&
			(pointColl.Position.Ceiling - vPosTop) <= CLICK(3 / 2.0f))
		{
			return true;
		}

		return false;
	}

	bool CanPerformLedgeJump(ItemInfo* item, CollisionInfo* coll)
	{
		static constexpr auto minLedgeHeight = CLICK(2);

		// 1. Check whether ledge jumps are enabled.
		if (!g_GameFlow->HasLedgeJumps())
			return false;

		// Raycast setup at minimum ledge height.
		auto origin = GameVector(
			item->Pose.Position.x,
			(item->Pose.Position.y - LARA_HEIGHT_STRETCH) + minLedgeHeight,
			item->Pose.Position.z,
			item->RoomNumber);
		auto target = GameVector(
			Geometry::TranslatePoint(origin.ToVector3i(), item->Pose.Orientation.y, OFFSET_RADIUS(coll->Setup.Radius)),
			item->RoomNumber);

		// 2. Assess level geometry ray collision.
		if (LOS(&origin, &target))
			return false;

		// TODO: Assess static object geometry ray collision.

		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_STRETCH;
		auto pointColl = GetCollision(item);

		// 3. Assess point collision.
		if ((pointColl.Position.Ceiling - vPosTop) >= -coll->Setup.Height) // Ceiling isn't too low.
			return false;

		return true;
	}

	bool CanClimbLedgeToCrouch(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::LedgeClimbSetup
		{
			item->Pose.Orientation.y,
			LARA_HEIGHT_CRAWL, LARA_HEIGHT, // Space range.
			CLICK(1),
			true
		};
		return Context::TestLedgeClimbSetup(item, coll, contextSetup);
	}

	bool CanClimbLedgeToStand(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::LedgeClimbSetup
		{
			item->Pose.Orientation.y,
			LARA_HEIGHT, -INFINITY, // Space range.
			CLICK(1),
			false
		};
		return Context::TestLedgeClimbSetup(item, coll, contextSetup);
	}

	bool CanLedgeShimmyLeft(ItemInfo* item, CollisionInfo* coll)
	{
		return TestLaraHangSideways(item, coll, ANGLE(-90.0f));
	}

	bool CanLedgeShimmyRight(ItemInfo* item, CollisionInfo* coll)
	{
		return TestLaraHangSideways(item, coll, ANGLE(90.0f));
	}

	bool CanWallShimmyUp(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto wallStepHeight = CLICK(1);

		auto& player = *GetLaraInfo(item);

		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_STRETCH;
		auto pointCollCenter = GetCollision(item);
		auto pointCollLeft = GetCollision(item, item->Pose.Orientation.y - ANGLE(90.0f), coll->Setup.Radius);
		auto pointCollRight = GetCollision(item, item->Pose.Orientation.y + ANGLE(90.0f), coll->Setup.Radius);

		// 1. Check whether wall is climbable.
		if (!player.Control.CanClimbLadder)
			return false;

		// 2. Assess point collision.
		if ((pointCollCenter.Position.Ceiling - vPosTop) <= -wallStepHeight &&
			(pointCollLeft.Position.Ceiling - vPosTop) <= -wallStepHeight &&
			(pointCollRight.Position.Ceiling - vPosTop) <= -wallStepHeight)
		{
			return true;
		}

		return false;
	}

	// TODO!!
	bool CanWallShimmyDown(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto wallStepHeight = CLICK(1);

		auto& player = *GetLaraInfo(item);

		int vPos = item->Pose.Position.y;
		auto pointCollCenter = GetCollision(item);
		// Left and right.

		// 1. Check whether wall is climbable.
		if (!player.Control.CanClimbLadder)
			return false;

		// 2. Assess point collision.
		if ((pointCollCenter.Position.Floor - vPos) >= wallStepHeight)
			return true;

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
		return Context::TestJumpSetup(item, coll, contextSetup);
	}

	bool CanJumpForward(ItemInfo* item, CollisionInfo* coll)
	{
		return Context::TestDirectionalStandingJump(item, coll, ANGLE(0.0f));
	}

	bool CanJumpBackward(ItemInfo* item, CollisionInfo* coll)
	{
		return Context::TestDirectionalStandingJump(item, coll, ANGLE(180.0f));
	}

	bool CanJumpLeft(ItemInfo* item, CollisionInfo* coll)
	{
		return Context::TestDirectionalStandingJump(item, coll, ANGLE(-90.0f));
	}

	bool CanJumpRight(ItemInfo* item, CollisionInfo* coll)
	{
		return Context::TestDirectionalStandingJump(item, coll, ANGLE(90.0f));
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
			CLICK(3 / 2.0f)
		};
		return Context::TestJumpSetup(item, coll, contextSetup);
	}
	
	bool CanSprintJumpForward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// 1. Check whether sprint jump is enabled.
		if (!g_GameFlow->HasSprintJump())
			return false;

		// 2. Check for jump state dispatch.
		if (!HasStateDispatch(item, LS_JUMP_FORWARD))
			return false;

		// 3. Check run timer.
		if (player.Control.Count.Run < LARA_SPRINT_JUMP_TIME)
			return false;

		// 4. Assess context.
		auto contextSetup = Context::JumpSetup
		{
			item->Pose.Orientation.y,
			CLICK(1.8f)
		};
		return Context::TestJumpSetup(item, coll, contextSetup);
	}

	bool CanPerformSlideJump(ItemInfo* item, CollisionInfo* coll)
	{
		// TODO: Get back to this project. -- Sezz 2022.11.11
		return true;

		// Check whether extended slide mechanics are enabled.
		if (!g_GameFlow->HasSlideExtended())
			return true;
		
		// TODO: Broken on diagonal slides?

		auto pointColl = GetCollision(item);

		short aspectAngle = GetLaraSlideHeadingAngle(item, coll);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(GetSurfaceNormal(pointColl.FloorTilt, true));
		return (abs(short(coll->Setup.ForwardAngle - aspectAngle)) <= abs(slopeAngle));
	}

	bool CanCrawlspaceDive(ItemInfo* item, CollisionInfo* coll)
	{
		auto pointColl = GetCollision(item, coll->Setup.ForwardAngle, coll->Setup.Radius, -coll->Setup.Height);
		return (abs(pointColl.Position.Ceiling - pointColl.Position.Floor) < LARA_HEIGHT || IsInNarrowSpace(item, coll));
	}

	bool CanDismountTightrope(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		auto pointColl = GetCollision(item);

		if (player.Control.Tightrope.CanDismount &&			   // Dismount is allowed.
			pointColl.Position.Floor == item->Pose.Position.y) // Floor is level with player.
		{
			return true;
		}

		return false;
	}

	Context::Vault GetVaultUp2Steps(ItemInfo* item, CollisionInfo* coll)
	{
		static const auto contextSetup = Context::VaultSetup
		{
			-STEPUP_HEIGHT, -CLICK(5 / 2.0f), // Floor range.
			LARA_HEIGHT, INFINITY,			  // Space range.
			CLICK(1)
		};
		auto context = GetVaultBase(item, coll, contextSetup);
		context.TargetState = LS_VAULT_2_STEPS;

		if (!context.Success)
			return context;

		context.Success = HasStateDispatch(item, context.TargetState);
		context.Height += CLICK(2);
		context.SetBusyHands = true;
		context.DoLedgeSnap = true;
		context.SetJumpVelocity = false;
		return context;
	}

	Context::WaterClimbOut GetWaterClimbOutDownStep(ItemInfo* item, CollisionInfo* coll)
	{
		auto contextSetup = Context::WaterClimbOutSetup
		{
			CLICK(5 / 4.0f) - 4, CLICK(1 / 2.0f),
			LARA_HEIGHT, -INFINITY,
			CLICK(1),
		};

		auto context = Context::GetWaterClimbOutBase(item, coll, contextSetup);
		context.Height -= CLICK(1);
		return context;
	}

	bool TestSidestep(ItemInfo* item, CollisionInfo* coll, bool isGoingRight)
	{
		const auto& player = *GetLaraInfo(item);

		// TODO: Make specific condition for wading in water.
		if (player.Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			auto contextSetup = Context::LandMovementSetupData
			{
				short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				-MAX_HEIGHT, -(int)CLICK(1.25f), // Upper bound defined by sidestep left/right states.
				false, false, false
			};
			return Context::TestGroundSetup(item, coll, contextSetup);
		}
		else
		{
			auto contextSetup = Context::LandMovementSetupData
			{
				short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				(int)CLICK(1.25f), -(int)CLICK(1.25f) // Defined by sidestep left/right states.
			};
			return Context::TestGroundSetup(item, coll, contextSetup);
		}
	}

	bool TestMonkeyShimmy(ItemInfo* item, CollisionInfo* coll, bool isGoingRight)
	{
		auto contextSetup = Context::MonkeySwingSetup
		{
			short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
			CLICK(0.5f), -CLICK(0.5f) // Defined by monkey shimmy left/right states.
		};
		return Context::TestMonkeySwingSetup(item, coll, contextSetup);
	}

	bool TestDirectionalStandingJump(ItemInfo* item, CollisionInfo* coll, short relHeadingAngle)
	{
		// Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			return false;

		auto contextSetup = Context::JumpSetup
		{
			short(item->Pose.Orientation.y + relHeadingAngle),
			CLICK(0.85f)
		};
		return Context::TestJumpSetup(item, coll, contextSetup);
	}

	bool TestGroundSetup(ItemInfo* item, CollisionInfo* coll, const Context::LandMovementSetupData& contextSetup, bool useCrawlSetup)
	{
		// HACK: coll->Setup.Radius and coll->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. -- Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

		auto pointColl = GetCollision(item, contextSetup.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);
		int vPos = item->Pose.Position.y;
		int vPosTop = vPos - playerHeight;

		int relFloorHeight = pointColl.Position.Floor - vPos;
		int relCeilingHeight = pointColl.Position.Ceiling - vPos;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 1. Test for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		bool isSlipperySlopeDown = contextSetup.TestSlipperySlopeBelow ? (pointColl.Position.FloorSlope && (pointColl.Position.Floor > vPos)) : false;
		bool isSlipperySlopeUp	 = contextSetup.TestSlipperySlopeAbove ? (pointColl.Position.FloorSlope && (pointColl.Position.Floor < vPos)) : false;
		bool isDeathFloor		 = contextSetup.TestDeathFloor		   ? pointColl.Block->Flags.Death										  : false;

		// 2. Check for slippery floor slope or death floor (if applicable).
		if (isSlipperySlopeDown || isSlipperySlopeUp || isDeathFloor)
			return false;

		// Raycast setup at upper floor bound.
		auto originA = GameVector(
			item->Pose.Position.x,
			(vPos + contextSetup.UpperFloorBound) - 1,
			item->Pose.Position.z,
			item->RoomNumber);
		auto targetA = GameVector(
			pointColl.Coordinates.x,
			(vPos + contextSetup.UpperFloorBound) - 1,
			pointColl.Coordinates.z,
			item->RoomNumber);

		// Raycast setup at lowest ceiling bound (player height).
		auto originB = GameVector(
			item->Pose.Position.x,
			vPosTop + 1,
			item->Pose.Position.z,
			item->RoomNumber);
		auto targetB = GameVector(
			pointColl.Coordinates.x,
			vPosTop + 1,
			pointColl.Coordinates.z,
			item->RoomNumber);

		// 3. Assess level geometry ray collision.
		if (!LOS(&originA, &targetA) || !LOS(&originB, &targetB))
			return false;

		// TODO: Assess static object geometry ray collision.

		// 4. Assess point collision.
		if (relFloorHeight <= contextSetup.LowerFloorBound && // Floor height is within lower floor bound.
			relFloorHeight >= contextSetup.UpperFloorBound && // Floor height is within upper floor bound.
			relCeilingHeight < -playerHeight &&				  // Ceiling height is lower than player height.
			floorToCeilHeight > playerHeight)				  // Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool TestMonkeySwingSetup(ItemInfo* item, CollisionInfo* coll, const Context::MonkeySwingSetup& contextSetup)
	{
		// HACK: Have to make the height explicit for now (see comment in above function). -- Sezz 2022.07.28
		static const int playerHeight = LARA_HEIGHT_MONKEY;

		int vPos = item->Pose.Position.y;
		int vPosTop = vPos - playerHeight;
		auto pointColl = GetCollision(item, contextSetup.HeadingAngle, OFFSET_RADIUS(coll->Setup.Radius));

		// 1. Test for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Test for ceiling slippery slope.
		if (pointColl.Position.CeilingSlope)
			return false;

		// Ray collision setup at highest floor bound (player base).
		auto origin0 = GameVector(
			item->Pose.Position.x,
			vPos - 1,
			item->Pose.Position.z,
			item->RoomNumber);
		auto target0 = GameVector(
			pointColl.Coordinates.x,
			vPos - 1,
			pointColl.Coordinates.z,
			item->RoomNumber);
		
		// Raycast setup at lower ceiling bound.
		auto origin1 = GameVector(
			item->Pose.Position.x,
			(vPosTop + contextSetup.LowerCeilingBound) + 1,
			item->Pose.Position.z,
			item->RoomNumber);
		auto target1 = GameVector(
			pointColl.Coordinates.x,
			(vPosTop + contextSetup.LowerCeilingBound) + 1,
			pointColl.Coordinates.z,
			item->RoomNumber);

		// 3. Assess level geometry ray collision.
		if (!LOS(&origin0, &target0) || !LOS(&origin1, &target1))
			return false;

		// TODO: Assess static object geometry ray collision.
		
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

	bool TestLedgeClimbSetup(ItemInfo* item, CollisionInfo* coll, const Context::LedgeClimbSetup& contextSetup)
	{
		constexpr auto climbHeightTolerance = CLICK(1);
		
		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_STRETCH;
		auto pointCollCenter = GetCollision(item);
		auto pointCollFront = GetCollision(item, contextSetup.HeadingAngle, coll->Setup.Radius, -(LARA_HEIGHT_STRETCH + CLICK(1 / 2.0f)));

		// 1. Check for slope (if applicable).
		bool isSlope = contextSetup.CheckSlope ? pointCollFront.Position.FloorSlope : false;
		if (isSlope)
			return false;

		// 2. Check for object.
		TestForObjectOnLedge(item, coll);
		if (coll->HitStatic)
			return false;

		// 3. Check for valid ledge.
		if (!TestValidLedge(item, coll))
			return false;

		// 4. Assess point collision.
		if (abs(pointCollFront.Position.Floor - vPosTop) <= climbHeightTolerance &&							 // Ledge height is climbable.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.SpaceMin &&  // Space isn't too narrow.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.SpaceMax && // Space isn't too wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= contextSetup.GapMin)	 // Gap is visually permissive.
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
					(pointColl.Position.Floor - vPos) >= CLICK(1 / 2.0f))))								// AND there is a drop below.
		{
			return true;
		}

		return false;
	}

	Context::Vault GetVaultBase(ItemInfo* item, CollisionInfo* coll, const Context::VaultSetup& contextSetup)
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
		// any potentially climbable floor in a room above will be missed. The following loop hacks around this floordata limitation.

		// 2. Raise vertical position of point collision probe by increments of 1/8th blocks to find potential vault ledge.
		int vOffset = contextSetup.LowerFloorBound;
		while (((pointCollFront.Position.Ceiling - vPos) > -coll->Setup.Height ||					 // Ceiling is below Lara's height...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.SpaceMin/* ||	// OR space is too small...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.ClampMax*/) &&	// OR space is too large (future-proofing; not possible right now).
			vOffset > (contextSetup.UpperFloorBound - coll->Setup.Height))							 // Offset is not too high.
		{
			pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), vOffset);
			vOffset -= std::max(CLICK(1 / 2.0f), contextSetup.SpaceMin);
		}

		// 3. Check for wall.
		if (pointCollFront.Position.Floor == NO_HEIGHT)
			return Context::Vault{ false };

		// 4. Assess point collision.
		if ((pointCollFront.Position.Floor - vPos) < contextSetup.LowerFloorBound &&						 // Floor is within lower floor bound.
			(pointCollFront.Position.Floor - vPos) >= contextSetup.UpperFloorBound &&						 // Floor is within upper floor bound.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.SpaceMin &&	 // Space is not too narrow.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.SpaceMax && // Space is not too wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= contextSetup.GapMin)	 // Gap is visually permissive.
		{
			return Context::Vault{ true, pointCollFront.Position.Floor };
		}

		return Context::Vault{ false };
	}

	Context::WaterClimbOut GetWaterClimbOutBase(ItemInfo* item, CollisionInfo* coll, Context::WaterClimbOutSetup contextSetup)
	{
		int vPos = item->Pose.Position.y;
		auto pointCollCenter = GetCollision(item);
		auto pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), -(coll->Setup.Height + CLICK(1)));

		// Assess point collision.
		if ((pointCollFront.Position.Floor - vPos) <= contextSetup.LowerFloorBound &&						 // Floor is within lower floor bound.
			(pointCollFront.Position.Floor - vPos) > contextSetup.UpperFloorBound &&	//check				 // Floor is within upper floor bound.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > contextSetup.ClampMin &&	 // Space is not too narrow.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= contextSetup.ClampMax && // Space is not to wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= contextSetup.GapMin)	 // Gap is visually permissive.
		{
			return Context::WaterClimbOut{ true, pointCollFront.Position.Floor };
		}

		return Context::WaterClimbOut{ false };
	}
}
