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
		if ((relFloorHeight <= LOWER_FLOOR_BOUND ||					// Floor height is above lower floor bound...
				player.Control.WaterStatus == WaterStatus::Wade) && // OR player is wading.
			relFloorHeight >= UPPER_FLOOR_BOUND)					// Floor height is below upper floor bound.
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
		if (relFloorHeight <= LOWER_FLOOR_BOUND && // Floor height is above lower floor bound.
			relFloorHeight >= UPPER_FLOOR_BOUND)   // Floor height is below than upper floor bound.
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
		if (relFloorHeight <= LOWER_FLOOR_BOUND && // Floor height is above lower floor bound.
			relFloorHeight >= UPPER_FLOOR_BOUND)   // Floor height is below upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStrikeAFKPose(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = GetLaraInfo(*item);

		// 1. Check if AFK posing is enabled.
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
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Defined by run forward state.
			false, true, false
		};

		return Context::TestLandMovementSetup(item, coll, setupData);
	}

	bool CanRunBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Defined by run backward state.
			false, false, false
		};

		return Context::TestLandMovementSetup(item, coll, setupData);
	}

	bool CanWalkForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			item->Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // NOTE: Defined by walk forward state.
		};

		return Context::TestLandMovementSetup(item, coll, setupData);
	}

	bool CanWalkBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // NOTE: Defined by walk backward state.
		};

		return Context::TestLandMovementSetup(item, coll, setupData);
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

		auto setupData = Context::LandMovementSetupData
		{
			item->Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Defined by wade forward state.
			false, false, false
		};

		// 2. Assess context.
		return Context::TestLandMovementSetup(item, coll, setupData);
	}

	bool CanWadeBackward(ItemInfo* item, CollisionInfo* coll)
	{
		const auto& player = *GetLaraInfo(item);

		// 1. Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Defined by walk backward state.
			false, false, false
		};

		// 2. Assess context.
		return Context::TestLandMovementSetup(item, coll, setupData);
	}

	bool CanSlide(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto ABS_FLOOR_BOUND = STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(*item);

		// 1. Test player water status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		// Get point collision.
		auto pointColl = GetCollision(item, 0, 0, -coll->Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;

		// 2. Assess point collision.
		if (abs(relFloorHeight) <= ABS_FLOOR_BOUND && // Floor height is within upper/lower floor bounds.
			pointColl.Position.FloorSlope)			  // Floor is a slippery slope.
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
		static const auto CROUCH_STATES = std::vector<int>
		{
			LS_CROUCH_IDLE,
			LS_CROUCH_TURN_LEFT,
			LS_CROUCH_TURN_RIGHT
		};

		// HACK: coll->Setup.Radius is only set to LARA_RADIUS_CRAWL in lara_col functions, then reset by LaraAboveWater(),
		// meaning that for tests called in lara_as functions it will store the wrong radius. -- Sezz 2021.11.05
		float radius = TestState(item->Animation.ActiveState, CROUCH_STATES) ? LARA_RADIUS_CRAWL : LARA_RADIUS;

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
		if (player.Control.WaterStatus != WaterStatus::Wade &&			 // Player is wading.
			(player.Control.HandStatus == HandStatus::Free ||			 // Player hands are free...
				!IsStandingWeapon(item, player.Control.Weapon.GunType))) // OR player is wielding a non-standing weapon.
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
		auto setupData = Context::LandMovementSetupData
		{
			item->Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Defined by crawl forward state.
		};

		return Context::TestLandMovementSetup(item, coll, setupData, true);
	}

	bool CanCrawlBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Defined by crawl backward state.
		};

		return Context::TestLandMovementSetup(item, coll, setupData, true);
	}

	bool CanPerformMonkeyStep(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto LOWER_CEIL_BOUND = MONKEY_STEPUP_HEIGHT;
		constexpr auto UPPER_CEIL_BOUND = -MONKEY_STEPUP_HEIGHT;

		// Get point collision.
		auto pointColl = GetCollision(item);
		int relCeilHeight = pointColl.Position.Ceiling - (item->Pose.Position.y - LARA_HEIGHT_MONKEY);

		// 1. Test for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return false;

		// 2. Assess point collision.
		if (relCeilHeight <= LOWER_CEIL_BOUND && // Ceiling height is above lower ceiling bound.
			relCeilHeight >= UPPER_CEIL_BOUND)	 // Ceiling height is below upper ceiling bound.
		{
			return true;
		}

		return false;
	}

	bool CanFallFromMonkeySwing(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto ABS_CEIL_BOUND = CLICK(1.25f);

		auto& player = GetLaraInfo(*item);

		// 1. Check for monkey swing ceiling.
		if (!player.Control.CanMonkeySwing)
			return true;

		// Get point collision.
		auto pointColl = GetCollision(item);
		int relCeilHeight = pointColl.Position.Ceiling - (item->Pose.Position.y - LARA_HEIGHT_MONKEY);

		// 2. Test for wall.
		if (pointColl.Position.Ceiling == NO_HEIGHT)
			return true;

		// 3. Test for slippery ceiling slope and check if overhang climb is disabled.
		if (pointColl.Position.CeilingSlope && !g_GameFlow->HasOverhangClimb())
			return true;

		// 4. Assess point collision.
		if (abs(relCeilHeight) < ABS_CEIL_BOUND) // Ceiling height if within lower/upper ceiling bound.
			return true;

		return false;
	}

	bool CanGrabMonkeySwing(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto GRAB_HEIGHT_TOLERANCE = CLICK(0.5f);

		const auto& player = GetLaraInfo(*item);

		// 1. Check for monkey swing ceiling.
		if (!player.Control.CanMonkeySwing)
			return false;

		auto pointColl = GetCollision(item);
		int relCeilHeight = pointColl.Position.Ceiling - (item->Pose.Position.y - LARA_HEIGHT_MONKEY);

		// 2. Assess collision with ceiling.
		if (relCeilHeight < 0 &&
			coll->CollisionType != CollisionType::CT_TOP &&
			coll->CollisionType != CollisionType::CT_TOP_FRONT)
		{
			return false;
		}

		// 3. Assess point collision.
		if (relCeilHeight <= GRAB_HEIGHT_TOLERANCE &&
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > LARA_HEIGHT_MONKEY)
		{
			return true;
		}

		return false;
	}

	bool CanMonkeyForward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::MonkeySwingSetupData
		{
			item->Pose.Orientation.y,
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Defined by monkey forward state.
		};

		return Context::TestMonkeySwingSetup(item, coll, setupData);
	}

	bool CanMonkeyBackward(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::MonkeySwingSetupData
		{
			short(item->Pose.Orientation.y + ANGLE(180.0f)),
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Defined by monkey backward state.
		};

		return Context::TestMonkeySwingSetup(item, coll, setupData);
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
		constexpr auto UPPER_FLOOR_BOUND = 0;
		constexpr auto LOWER_CEIL_BOUND	 = CLICK(1.5f);

		auto& player = GetLaraInfo(*item);

		auto pointColl = GetCollision(item, item->Pose.Orientation.y, BLOCK(0.25f));
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;
		int relCeilHeight = pointColl.Position.Ceiling - (item->Pose.Position.y - LARA_HEIGHT_REACH);

		// 1. Test for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		// 2. Assess point collision.
		if (relFloorHeight >= UPPER_FLOOR_BOUND && // Floor height is lower than upper floor bound.
			relCeilHeight <= LOWER_CEIL_BOUND)	   // Ceiling height is above lower ceiling bound.
		{
			return true;
		}

		return false;
	}

	bool CanPerformLedgeJump(ItemInfo* item, CollisionInfo* coll)
	{
		constexpr auto LEDGE_HEIGHT_MIN = CLICK(2);

		// 1. Check if ledge jumps are enabled.
		if (!g_GameFlow->HasLedgeJumps())
			return false;

		// Ray collision setup at minimum ledge height.
		auto origin = GameVector(
			item->Pose.Position.x,
			(item->Pose.Position.y - LARA_HEIGHT_STRETCH) + LEDGE_HEIGHT_MIN,
			item->Pose.Position.z,
			item->RoomNumber);
		auto target = GameVector(
			Geometry::TranslatePoint(origin.ToVector3i(), item->Pose.Orientation.y, OFFSET_RADIUS(coll->Setup.Radius)),
			item->RoomNumber);

		// 2. Assess level geometry ray collision.
		if (LOS(&origin, &target))
			return false;

		// TODO: Assess static object geometry ray collision.

		// Get point collision.
		auto pointColl = GetCollision(item);
		int relCeilHeight = pointColl.Position.Ceiling - (item->Pose.Position.y - LARA_HEIGHT_STRETCH);

		// 3. Assess point collision.
		if (relCeilHeight >= -coll->Setup.Height) // Ceiling isn't too low.
			return false;

		return true;
	}

	bool CanClimbLedgeToCrouch(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LedgeClimbSetupData
		{
			item->Pose.Orientation.y,
			LARA_HEIGHT_CRAWL, LARA_HEIGHT, // Space range.
			CLICK(1),
			true
		};

		return Context::TestLedgeClimbSetup(item, coll, setupData);
	}

	bool CanClimbLedgeToStand(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::LedgeClimbSetupData
		{
			item->Pose.Orientation.y,
			LARA_HEIGHT, -INFINITY, // Space range.
			CLICK(1),
			false
		};

		return Context::TestLedgeClimbSetup(item, coll, setupData);
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
		static const auto setupData = Context::JumpSetupData
		{
			0,
			0.0f,
			false
		};
		return Context::TestJumpSetup(item, coll, setupData);
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

		auto setupData = Context::JumpSetupData
		{
			item->Pose.Orientation.y,
			CLICK(3 / 2.0f)
		};
		return Context::TestJumpSetup(item, coll, setupData);
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
		auto setupData = Context::JumpSetupData
		{
			item->Pose.Orientation.y,
			CLICK(1.8f)
		};
		return Context::TestJumpSetup(item, coll, setupData);
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

	Context::VaultData GetVaultUp2Steps(ItemInfo* item, CollisionInfo* coll)
	{
		static const auto setupData = Context::VaultSetupData
		{
			-STEPUP_HEIGHT, -CLICK(2.5f), // Floor range.
			LARA_HEIGHT, INFINITY,			  // Space range.
			CLICK(1)
		};
		auto context = GetVaultContextData(item, coll, setupData);
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

	Context::WaterClimbOutData GetWaterClimbOutDownStep(ItemInfo* item, CollisionInfo* coll)
	{
		auto setupData = Context::WaterClimbOutSetupData
		{
			CLICK(5 / 4.0f) - 4, CLICK(1 / 2.0f),
			LARA_HEIGHT, -INFINITY,
			CLICK(1),
		};

		auto context = Context::GetWaterClimbOutData(item, coll, setupData);
		context.Height -= CLICK(1);
		return context;
	}

	bool TestSidestep(ItemInfo* item, CollisionInfo* coll, bool isGoingRight)
	{
		const auto& player = GetLaraInfo(*item);

		// 1a. Assess context in wade case.
		if (player.Control.WaterStatus == WaterStatus::Wade)
		{
			auto setupData = Context::LandMovementSetupData
			{
				short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				-MAX_HEIGHT, -(int)CLICK(1.25f), // NOTE: Upper bound defined by sidestep left/right states.
				false, false, false
			};

			return Context::TestLandMovementSetup(item, coll, setupData);
		}
		
		auto setupData = Context::LandMovementSetupData
		{
			short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
			(int)CLICK(1.25f), -(int)CLICK(1.25f) // NOTE: Defined by sidestep left/right states.
		};

		// 1b. Assess context in regular case.
		return Context::TestLandMovementSetup(item, coll, setupData);
	}

	bool TestMonkeyShimmy(ItemInfo* item, CollisionInfo* coll, bool isGoingRight)
	{
		auto setupData = Context::MonkeySwingSetupData
		{
			short(item->Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
			CLICK(0.5f), -CLICK(0.5f) // NOTE: Defined by monkey shimmy left/right states.
		};

		return Context::TestMonkeySwingSetup(item, coll, setupData);
	}

	bool TestDirectionalStandingJump(ItemInfo* item, CollisionInfo* coll, short relHeadingAngle)
	{
		// Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			return false;

		auto setupData = Context::JumpSetupData
		{
			short(item->Pose.Orientation.y + relHeadingAngle),
			CLICK(0.85f)
		};

		return Context::TestJumpSetup(item, coll, setupData);
	}

	bool TestLandMovementSetup(ItemInfo* item, CollisionInfo* coll, const Context::LandMovementSetupData& setupData, bool isCrawling)
	{
		// HACK: coll->Setup.Radius and coll->Setup.Height are set only in lara_col functions and then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any context assessment functions called in crouch/crawl lara_as routines.
		// If states become objects, a dedicated state init function should eliminate the need for the isCrawling parameter. -- Sezz 2022.03.16
		int playerRadius = isCrawling ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
		int playerHeight = isCrawling ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

		auto pointColl = GetCollision(item, setupData.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);
		int vPos = item->Pose.Position.y;
		int vPosTop = vPos - playerHeight;

		int relFloorHeight = pointColl.Position.Floor - vPos;
		int relCeilingHeight = pointColl.Position.Ceiling - vPos;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 1. Test for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		bool isSlipperySlopeDown = setupData.TestSlipperySlopeBelow ? (pointColl.Position.FloorSlope && (pointColl.Position.Floor > vPos)) : false;
		bool isSlipperySlopeUp	 = setupData.TestSlipperySlopeAbove ? (pointColl.Position.FloorSlope && (pointColl.Position.Floor < vPos)) : false;
		bool isDeathFloor		 = setupData.TestDeathFloor		   ? pointColl.Block->Flags.Death										  : false;

		// 2. Check for slippery floor slope or death floor (if applicable).
		if (isSlipperySlopeDown || isSlipperySlopeUp || isDeathFloor)
			return false;

		// Raycast setup at upper floor bound.
		auto originA = GameVector(
			item->Pose.Position.x,
			(vPos + setupData.UpperFloorBound) - 1,
			item->Pose.Position.z,
			item->RoomNumber);
		auto targetA = GameVector(
			pointColl.Coordinates.x,
			(vPos + setupData.UpperFloorBound) - 1,
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
		if (relFloorHeight <= setupData.LowerFloorBound && // Floor height is above lower floor bound.
			relFloorHeight >= setupData.UpperFloorBound && // Floor height is below upper floor bound.
			relCeilingHeight < -playerHeight &&				  // Ceiling height is above player height.
			floorToCeilHeight > playerHeight)				  // Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool TestMonkeySwingSetup(ItemInfo* item, CollisionInfo* coll, const Context::MonkeySwingSetupData& setupData)
	{
		// HACK: Have to make the height explicit for now (see comment in above function). -- Sezz 2022.07.28
		constexpr auto PLAYER_HEIGHT = LARA_HEIGHT_MONKEY;

		int vPos = item->Pose.Position.y;
		int vPosTop = vPos - PLAYER_HEIGHT;
		auto pointColl = GetCollision(item, setupData.HeadingAngle, OFFSET_RADIUS(coll->Setup.Radius));

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
			(vPosTop + setupData.LowerCeilingBound) + 1,
			item->Pose.Position.z,
			item->RoomNumber);
		auto target1 = GameVector(
			pointColl.Coordinates.x,
			(vPosTop + setupData.LowerCeilingBound) + 1,
			pointColl.Coordinates.z,
			item->RoomNumber);

		// 3. Assess level geometry ray collision.
		if (!LOS(&origin0, &target0) || !LOS(&origin1, &target1))
			return false;

		// TODO: Assess static object geometry ray collision.
		
		// 4. Assess point collision.
		if (pointColl.BottomBlock->Flags.Monkeyswing &&								    // Ceiling is a monkey swing.
			(pointColl.Position.Ceiling - vPosTop) <= setupData.LowerCeilingBound &&	// Ceiling is within lower ceiling bound.
			(pointColl.Position.Ceiling - vPosTop) >= setupData.UpperCeilingBound &&	// Ceiling is within upper ceiling bound.
			(pointColl.Position.Floor - vPos) > 0 &&									// Floor is within highest floor bound (player base).
			abs(pointColl.Position.Ceiling - pointColl.Position.Floor) > PLAYER_HEIGHT)	// Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool TestLedgeClimbSetup(ItemInfo* item, CollisionInfo* coll, const Context::LedgeClimbSetupData& setupData)
	{
		constexpr auto climbHeightTolerance = CLICK(1);
		
		int vPosTop = item->Pose.Position.y - LARA_HEIGHT_STRETCH;
		auto pointCollCenter = GetCollision(item);
		auto pointCollFront = GetCollision(item, setupData.HeadingAngle, coll->Setup.Radius, -(LARA_HEIGHT_STRETCH + CLICK(1 / 2.0f)));

		// 1. Check for slope (if applicable).
		bool isSlope = setupData.CheckSlope ? pointCollFront.Position.FloorSlope : false;
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
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > setupData.SpaceMin &&  // Space isn't too narrow.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= setupData.SpaceMax && // Space isn't too wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= setupData.GapMin)	 // Gap is visually permissive.
		{
			return true;
		}

		return false;
	}

	bool TestJumpSetup(ItemInfo* item, CollisionInfo* coll, const Context::JumpSetupData& setupData)
	{
		const auto& player = *GetLaraInfo(item);

		int vPos = item->Pose.Position.y;
		auto pointColl = GetCollision(item, setupData.HeadingAngle, setupData.Distance, -coll->Setup.Height);

		// 1. Check for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
		bool isWading = setupData.TestWadeStatus ? (player.Control.WaterStatus == WaterStatus::Wade) : false;

		// 2. Check for swamp or wade status (if applicable).
		if (isSwamp || isWading)
			return false;

		// 3. Check for corner.
		if (TestLaraFacingCorner(item, setupData.HeadingAngle, setupData.Distance))
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

	Context::VaultData GetVaultContextData(ItemInfo* item, CollisionInfo* coll, const Context::VaultSetupData& setupData)
	{
		const auto& player = *GetLaraInfo(item);

		auto pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), -coll->Setup.Height);
		auto pointCollCenter = GetCollision(item, 0, 0, -coll->Setup.Height / 2);

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
		bool isSwampTooDeep = setupData.TestSwampDepth ? (isSwamp && player.WaterSurfaceDist < -CLICK(3)) : isSwamp;
		
		int vPos = isSwamp ? item->Pose.Position.y : pointCollCenter.Position.Floor; // HACK: Avoid cheese when in the midst of performing a step. Can be done better. @Sezz 2022.04.08	

		// 1. Check swamp depth (if applicable).
		if (isSwampTooDeep)
			return Context::VaultData{ false };

		// NOTE: Where the point collision probe finds that
		// a) the "wall" in front is formed by a ceiling, or
		// b) the space between the floor and ceiling is too narrow,
		// any potentially climbable floor in a room above will be missed. The following loop hacks around this floordata limitation.

		// 2. Raise vertical position of point collision probe by increments of 1/8th blocks to find potential vault ledge.
		int vOffset = setupData.LowerFloorBound;
		while (((pointCollFront.Position.Ceiling - vPos) > -coll->Setup.Height ||					 // Ceiling is below Lara's height...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= setupData.SpaceMin/* ||	// OR space is too small...
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > setupData.ClampMax*/) &&	// OR space is too large (future-proofing; not possible right now).
			vOffset > (setupData.UpperFloorBound - coll->Setup.Height))							 // Offset is not too high.
		{
			pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), vOffset);
			vOffset -= std::max(CLICK(1 / 2.0f), setupData.SpaceMin);
		}

		// 3. Check for wall.
		if (pointCollFront.Position.Floor == NO_HEIGHT)
			return Context::VaultData{ false };

		// 4. Assess point collision.
		if ((pointCollFront.Position.Floor - vPos) < setupData.LowerFloorBound &&						 // Floor is within lower floor bound.
			(pointCollFront.Position.Floor - vPos) >= setupData.UpperFloorBound &&						 // Floor is within upper floor bound.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > setupData.SpaceMin &&	 // Space is not too narrow.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= setupData.SpaceMax && // Space is not too wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= setupData.GapMin)	 // Gap is visually permissive.
		{
			return Context::VaultData{ true, pointCollFront.Position.Floor };
		}

		return Context::VaultData{ false };
	}

	Context::WaterClimbOutData GetWaterClimbOutData(ItemInfo* item, CollisionInfo* coll, const Context::WaterClimbOutSetupData& setupData)
	{
		int vPos = item->Pose.Position.y;
		auto pointCollCenter = GetCollision(item);
		auto pointCollFront = GetCollision(item, coll->NearestLedgeAngle, OFFSET_RADIUS(coll->Setup.Radius), -(coll->Setup.Height + CLICK(1)));

		// Assess point collision.
		if ((pointCollFront.Position.Floor - vPos) <= setupData.LowerFloorBound &&						 // Floor is within lower floor bound.
			(pointCollFront.Position.Floor - vPos) > setupData.UpperFloorBound &&	//check				 // Floor is within upper floor bound.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > setupData.ClampMin &&	 // Space is not too narrow.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) <= setupData.ClampMax && // Space is not to wide.
			abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor) >= setupData.GapMin)	 // Gap is visually permissive.
		{
			return Context::WaterClimbOutData{ true, pointCollFront.Position.Floor };
		}

		return Context::WaterClimbOutData{ false };
	}
}
