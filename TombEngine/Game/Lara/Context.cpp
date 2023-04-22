#include "framework.h"
#include "Game/Lara/Context.h"

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

using namespace TEN::Collision::Floordata;
using namespace TEN::Input;

namespace TEN::Player::Context
{
	bool CanPerformStep(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND = -STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// 1) Test for wall.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return false;

		// 2) Test if player is already aligned with floor.
		if (relFloorHeight == 0)
			return false;

		// 3) Assess point collision and player status.
		if ((relFloorHeight <= LOWER_FLOOR_BOUND ||					// Floor height is above lower floor bound...
				player.Control.WaterStatus == WaterStatus::Wade) && // OR player is wading.
			relFloorHeight >= UPPER_FLOOR_BOUND)					// Floor height is below upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStepUp(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = -CLICK(0.5f);
		constexpr auto UPPER_FLOOR_BOUND = -STEPUP_HEIGHT;

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// Assess point collision.
		if (relFloorHeight <= LOWER_FLOOR_BOUND && // Floor height is above lower floor bound.
			relFloorHeight >= UPPER_FLOOR_BOUND)   // Floor height is below than upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStepDown(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND = CLICK(0.5f);

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// Assess point collision.
		if (relFloorHeight <= LOWER_FLOOR_BOUND && // Floor height is above lower floor bound.
			relFloorHeight >= UPPER_FLOOR_BOUND)   // Floor height is below upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStrikeAfkPose(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Check if AFK posing is enabled.
		if (!g_GameFlow->HasAFKPose())
			return false;

		// 2) Test AFK pose timer.
		if (player.Control.Count.Pose < LARA_POSE_TIME)
			return false;

		// 3) Test player hand and water status.
		if (player.Control.HandStatus != HandStatus::Free ||
			player.Control.WaterStatus == WaterStatus::Wade)
		{
			return false;
		}

		// 4) Assess player status.
		if (!(IsHeld(In::Flare) || IsHeld(In::DrawWeapon)) &&		   // Avoid unsightly concurrent actions.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare...
				player.Flare.Life) &&								   // OR flare is still active.
			player.Context.Vehicle == NO_ITEM)						   // Not in a vehicle.
		{
			return true;
		}

		return false;
	}

	bool CanTurn180(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Assess player status.
		if (player.Control.WaterStatus == WaterStatus::Wade || // Is wading...
			TestEnvironment(ENV_FLAG_SWAMP, &item))			   // OR is in swamp.
		{
			return true;
		}

		return false;
	}

	bool CanTurnFast(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Assess player status.
		if (player.Control.WaterStatus == WaterStatus::Dry &&
			((player.Control.HandStatus == HandStatus::WeaponReady && player.Control.Weapon.GunType != LaraWeaponType::Torch) ||
				(player.Control.HandStatus == HandStatus::WeaponDraw && player.Control.Weapon.GunType != LaraWeaponType::Flare)))
		{
			return true;
		}

		return false;
	}

	static bool TestGroundMovementSetup(const ItemInfo& item, const CollisionInfo& coll, const GroundMovementSetupData& setupData, bool isCrawling = false)
	{
		// HACK: coll.Setup.Radius and coll.Setup.Height are set only in lara_col functions and then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any context assessment functions called in crouch/crawl lara_as routines.
		// If states become objects, a dedicated state init function should eliminate the need for the isCrawling parameter. -- Sezz 2022.03.16
		int playerRadius = isCrawling ? LARA_RADIUS_CRAWL : coll.Setup.Radius;
		int playerHeight = isCrawling ? LARA_HEIGHT_CRAWL : coll.Setup.Height;

		// Get point collision.
		auto pointColl = GetCollision(&item, setupData.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);
		int vPos = item.Pose.Position.y;
		int vPosTop = vPos - playerHeight;

		bool isSlipperySlopeDown = setupData.TestSlipperySlopeBelow ? (pointColl.Position.FloorSlope && (pointColl.Position.Floor > vPos)) : false;
		bool isSlipperySlopeUp	 = setupData.TestSlipperySlopeAbove ? (pointColl.Position.FloorSlope && (pointColl.Position.Floor < vPos)) : false;
		bool isDeathFloor		 = setupData.TestDeathFloor			? pointColl.Block->Flags.Death										   : false;

		// 2) Check for slippery floor slope or death floor (if applicable).
		if (isSlipperySlopeDown || isSlipperySlopeUp || isDeathFloor)
			return false;

		// Raycast setup at upper floor bound.
		auto origin0 = GameVector(
			item.Pose.Position.x,
			(vPos + setupData.UpperFloorBound) - 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target0 = GameVector(
			pointColl.Coordinates.x,
			(vPos + setupData.UpperFloorBound) - 1,
			pointColl.Coordinates.z,
			item.RoomNumber);

		// Raycast setup at lowest ceiling bound (player height).
		auto origin1 = GameVector(
			item.Pose.Position.x,
			vPosTop + 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target1 = GameVector(
			pointColl.Coordinates.x,
			vPosTop + 1,
			pointColl.Coordinates.z,
			item.RoomNumber);

		// 3) Assess level geometry ray collision.
		if (!LOS(&origin0, &target0) || !LOS(&origin1, &target1))
			return false;

		// TODO: Assess hard object geometry ray collision.

		int relFloorHeight = pointColl.Position.Floor - vPos;
		int relCeilHeight = pointColl.Position.Ceiling - vPos;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 4) Assess point collision.
		if (relFloorHeight <= setupData.LowerFloorBound && // Floor height is above lower floor bound.
			relFloorHeight >= setupData.UpperFloorBound && // Floor height is below upper floor bound.
			relCeilHeight < -playerHeight &&			   // Ceiling height is above player height.
			floorToCeilHeight > playerHeight)			   // Floor-to-ceiling height isn't too narrow.
		{
			return true;
		}

		return false;
	}

	bool CanRunForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by run forward state.
			false, true, false
		};

		return TestGroundMovementSetup(item, coll, setupData);
	}

	bool CanRunBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by run backward state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, setupData);
	}

	bool CanWalkForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by walk forward state.
		};

		return TestGroundMovementSetup(item, coll, setupData);
	}

	bool CanWalkBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // NOTE: Bounds defined by walk backward state.
		};

		return TestGroundMovementSetup(item, coll, setupData);
	}

	static bool TestSidestep(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		const auto& player = GetLaraInfo(item);

		auto setupData = GroundMovementSetupData{};

		// Wade case.
		if (player.Control.WaterStatus == WaterStatus::Wade)
		{
			setupData = GroundMovementSetupData
			{
				short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				-MAX_HEIGHT, -(int)CLICK(1.25f), // NOTE: Upper bound defined by sidestep left/right states.
				false, false, false
			};
		}
		// Regular case.
		else
		{
			setupData = GroundMovementSetupData
			{
				short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				(int)CLICK(1.25f), -(int)CLICK(1.25f) // NOTE: Bounds defined by sidestep left/right states.
			};
		}

		return TestGroundMovementSetup(item, coll, setupData);
	}

	bool CanSidestepLeft(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestSidestep(item, coll, false);
	}

	bool CanSidestepRight(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestSidestep(item, coll, true);
	}

	bool CanWadeForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		auto setupData = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by wade forward state.
			false, false, false
		};

		// 2) Assess context.
		return TestGroundMovementSetup(item, coll, setupData);
	}

	bool CanWadeBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		auto setupData = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by walk backward state.
			false, false, false
		};

		// 2) Assess context.
		return TestGroundMovementSetup(item, coll, setupData);
	}

	bool CanSlide(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_FLOOR_BOUND = STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// 2) Assess point collision.
		if (abs(relFloorHeight) <= ABS_FLOOR_BOUND && // Floor height is within upper/lower floor bounds.
			pointColl.Position.FloorSlope)			  // Floor is a slippery slope.
		{
			return true;
		}

		return false;
	}

	bool CanSteerOnSlide(const ItemInfo& item, const CollisionInfo& coll)
	{
		return g_GameFlow->HasSlideExtended();
	}

	bool IsInLowSpace(const ItemInfo& item, const CollisionInfo& coll)
	{
		static const auto CROUCH_STATES = std::vector<int>
		{
			LS_CROUCH_IDLE,
			LS_CROUCH_TURN_LEFT,
			LS_CROUCH_TURN_RIGHT
		};

		// HACK: coll.Setup.Radius is only set to LARA_RADIUS_CRAWL in lara_col functions, then reset by LaraAboveWater(),
		// meaning that for tests called in lara_as functions it will store the wrong radius. -- Sezz 2021.11.05
		float radius = TestState(item.Animation.ActiveState, CROUCH_STATES) ? LARA_RADIUS_CRAWL : LARA_RADIUS;

		// Assess center point collision.
		auto pointCollCenter = GetCollision(&item, 0, 0.0f, -LARA_HEIGHT / 2);
		if (abs(pointCollCenter.Position.Ceiling - pointCollCenter.Position.Floor) < LARA_HEIGHT ||	// Center space is narrow enough.
			abs(coll.Middle.Ceiling - LARA_HEIGHT_CRAWL) < LARA_HEIGHT)								// Consider statics overhead detected by GetCollisionInfo().
		{
			return true;
		}

		// TODO: Check whether < or <= and > or >=.

		// Assess front point collision.
		auto pointCollFront = GetCollision(&item, item.Pose.Orientation.y, radius, -coll.Setup.Height);
		if (abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) < LARA_HEIGHT &&		// Front space is narrow enough.
			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > LARA_HEIGHT_CRAWL && // Front space not too narrow.
			abs(pointCollFront.Position.Floor - pointCollCenter.Position.Floor) <= CRAWL_STEPUP_HEIGHT) // Front floor is within upper/lower floor bounds.
		{
			return true;
		}

		// Assess back point collision.
		auto pointCollBack = GetCollision(&item, item.Pose.Orientation.y, -radius, -coll.Setup.Height);
		if (abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor) < LARA_HEIGHT &&		   // Back space is narrow enough.
			abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor) > LARA_HEIGHT_CRAWL &&  // Back space not too narrow.
			abs(pointCollBack.Position.Floor - pointCollCenter.Position.Floor) <= CRAWL_STEPUP_HEIGHT) // Back floor is within upper/lower floor bounds.
		{
			return true;
		}

		return false;
	}

	bool CanCrouch(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Assess player status.
		if (player.Control.WaterStatus != WaterStatus::Wade &&			 // Player is wading.
			(player.Control.HandStatus == HandStatus::Free ||			 // Player hands are free...
				!IsStandingWeapon(&item, player.Control.Weapon.GunType))) // OR player is wielding a non-standing weapon.
		{
			return true;
		}

		return false;
	}

	bool CanCrouchToCrawl(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

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

	bool CanCrouchRoll(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto FLOOR_BOUND				= CRAWL_STEPUP_HEIGHT;
		constexpr auto FLOOR_TO_CEIL_HEIGHT_MAX = LARA_HEIGHT_CRAWL;
		constexpr auto WATER_HEIGHT_MAX			= -CLICK(1);
		constexpr auto PROBE_DIST_MAX			= BLOCK(1);
		constexpr auto STEP_DIST				= BLOCK(0.25f);

		const auto& player = GetLaraInfo(item);

		// 1) Check if crouch roll is enabled.
		if (!g_GameFlow->HasCrouchRoll())
			return false;

		// 2) Test water depth.
		if (player.Context.WaterSurfaceDist < WATER_HEIGHT_MAX)
			return false;

		// TODO: Extend point collision struct to also find water depths.
		float distance = 0.0f;
		auto pointColl0 = GetCollision(&item);

		// 3) Test continuity of path.
		while (distance < PROBE_DIST_MAX)
		{
			// Get point collision.
			distance += STEP_DIST;
			auto pointColl1 = GetCollision(&item, item.Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

			int floorHeightDelta = abs(pointColl0.Position.Floor - pointColl1.Position.Floor);
			int floorToCeilHeight = abs(pointColl1.Position.Ceiling - pointColl1.Position.Floor);

			// Assess point collision.
			if (floorHeightDelta > FLOOR_BOUND ||				 // Avoid floor height delta beyond crawl stepup threshold.
				floorToCeilHeight <= FLOOR_TO_CEIL_HEIGHT_MAX || // Avoid narrow spaces.
				pointColl1.Position.FloorSlope)					 // Avoid slippery floor slopes.
			{
				return false;
			}

			pointColl0 = std::move(pointColl1);
		}

		return true;
	}

	bool CanCrawlForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Bounds defined by crawl forward state.
		};

		return TestGroundMovementSetup(item, coll, setupData, true);
	}

	bool CanCrawlBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Bounds defined by crawl backward state.
		};

		return TestGroundMovementSetup(item, coll, setupData, true);
	}

	bool CanPerformMonkeyStep(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_CEIL_BOUND = MONKEY_STEPUP_HEIGHT;
		constexpr auto UPPER_CEIL_BOUND = -MONKEY_STEPUP_HEIGHT;

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - LARA_HEIGHT_MONKEY);

		// Assess point collision.
		if (relCeilHeight <= LOWER_CEIL_BOUND && // Ceiling height is above lower ceiling bound.
			relCeilHeight >= UPPER_CEIL_BOUND)	 // Ceiling height is below upper ceiling bound.
		{
			return true;
		}

		return false;
	}

	bool CanFallFromMonkeySwing(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_CEIL_BOUND = CLICK(1.25f);

		auto& player = GetLaraInfo(item);

		// 1) Check for monkey swing ceiling.
		if (!player.Control.CanMonkeySwing)
			return true;

		// Get point collision.
		auto pointColl = GetCollision(&item);

		// 2) Test for slippery ceiling slope and check if overhang climb is disabled.
		if (pointColl.Position.CeilingSlope && !g_GameFlow->HasOverhangClimb())
			return true;

		// 3) Assess point collision.
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - LARA_HEIGHT_MONKEY);
		if (abs(relCeilHeight) > ABS_CEIL_BOUND) // Ceiling height is within lower/upper ceiling bound.
			return true;

		return false;
	}

	bool CanGrabMonkeySwing(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_CEIL_BOUND			= CLICK(0.5f);
		constexpr auto FLOOR_TO_CEIL_HEIGHT_MAX = LARA_HEIGHT_MONKEY;

		const auto& player = GetLaraInfo(item);

		// 1) Check for monkey swing ceiling.
		if (!player.Control.CanMonkeySwing)
			return false;

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - LARA_HEIGHT_MONKEY);
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 2) Assess collision with ceiling.
		if (relCeilHeight < 0 &&
			coll.CollisionType != CollisionType::CT_TOP &&
			coll.CollisionType != CollisionType::CT_TOP_FRONT)
		{
			return false;
		}

		// 3) Assess point collision.
		if (relCeilHeight <= LOWER_CEIL_BOUND &&		  // Ceiling height is above lower ceiling bound.
			floorToCeilHeight > FLOOR_TO_CEIL_HEIGHT_MAX) // Floor-to-ceiling space isn't too narrow.
		{
			return true;
		}

		return false;
	}

	static bool TestMonkeySwingSetup(const ItemInfo& item, const CollisionInfo& coll, const MonkeySwingSetupData& setupData)
	{
		// HACK: Have to make the height explicit for now. -- Sezz 2022.07.28
		constexpr auto PLAYER_HEIGHT = LARA_HEIGHT_MONKEY;

		// Get point collision.
		auto pointColl = GetCollision(&item, setupData.HeadingAngle, OFFSET_RADIUS(coll.Setup.Radius));

		// 1) Test if ceiling is monkey swing.
		if (!pointColl.BottomBlock->Flags.Monkeyswing)
			return false;

		// 2) Test for ceiling slippery slope.
		if (pointColl.Position.CeilingSlope)
			return false;

		int vPos = item.Pose.Position.y;
		int vPosTop = vPos - coll.Setup.Height;

		// Ray collision setup at highest floor bound (player base).
		auto origin0 = GameVector(
			item.Pose.Position.x,
			vPos - 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target0 = GameVector(
			pointColl.Coordinates.x,
			vPos - 1,
			pointColl.Coordinates.z,
			item.RoomNumber);

		// Raycast setup at lower ceiling bound.
		auto origin1 = GameVector(
			item.Pose.Position.x,
			(vPosTop + setupData.LowerCeilingBound) + 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target1 = GameVector(
			pointColl.Coordinates.x,
			(vPosTop + setupData.LowerCeilingBound) + 1,
			pointColl.Coordinates.z,
			item.RoomNumber);

		// 3) Assess level geometry ray collision.
		if (!LOS(&origin0, &target0) || !LOS(&origin1, &target1))
			return false;

		// TODO: Assess static object geometry ray collision.

		int relFloorHeight = pointColl.Position.Floor - vPos;
		int relCeilHeight = pointColl.Position.Ceiling - vPosTop;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 4) Assess point collision.
		if (relFloorHeight > 0 &&							// Floor is within highest floor bound (player base).
			relCeilHeight <= setupData.LowerCeilingBound && // Ceiling is within lower ceiling bound.
			relCeilHeight >= setupData.UpperCeilingBound && // Ceiling is within upper ceiling bound.
			floorToCeilHeight > PLAYER_HEIGHT)				// Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool CanMonkeyForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = MonkeySwingSetupData
		{
			item.Pose.Orientation.y,
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Bounds defined by monkey forward state.
		};

		return TestMonkeySwingSetup(item, coll, setupData);
	}

	bool CanMonkeyBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setupData = MonkeySwingSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Bounds defined by monkey backward state.
		};

		return TestMonkeySwingSetup(item, coll, setupData);
	}

	static bool TestMonkeyShimmy(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		auto setupData = MonkeySwingSetupData
		{
			short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
			CLICK(0.5f), -CLICK(0.5f) // NOTE: Bounds defined by monkey shimmy left/right states.
		};

		return TestMonkeySwingSetup(item, coll, setupData);
	}

	bool CanMonkeyShimmyLeft(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestMonkeyShimmy(item, coll, false);
	}

	bool CanMonkeyShimmyRight(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestMonkeyShimmy(item, coll, true);
	}

	bool CanFall(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto UPPER_FLOOR_BOUND = STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// 2) Assess point collision.
		if (relFloorHeight > UPPER_FLOOR_BOUND) // Floor height is below upper floor bound.
			return true;

		return false;
	}

	bool CanLand(const ItemInfo& item, const CollisionInfo& coll)
	{
		// 1) Check airborne status and Y velocity.
		if (!item.Animation.IsAirborne || item.Animation.Velocity.y < 0.0f)
			return false;

		// 2) Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, &item))
			return true;

		int vPos = item.Pose.Position.y;
		auto pointColl = GetCollision(&item);

		// 3) Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= item.Animation.Velocity.y)
			return true;

		return false;
	}

	bool CanPerformJump(const ItemInfo& item, const CollisionInfo& coll)
	{
		return !TestEnvironment(ENV_FLAG_SWAMP, &item);
	}

	static bool TestJumpSetup(const ItemInfo& item, const CollisionInfo& coll, const JumpSetupData& setupData)
	{
		const auto& player = GetLaraInfo(item);

		int vPos = item.Pose.Position.y;
		auto pointColl = GetCollision(&item, setupData.HeadingAngle, setupData.Distance, -coll.Setup.Height);

		bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, &item);
		bool isWading = setupData.TestWadeStatus ? (player.Control.WaterStatus == WaterStatus::Wade) : false;

		// 1) Check for swamp or wade status (if applicable).
		if (isSwamp || isWading)
			return false;

		// 2) Check for corner.
		if (TestLaraFacingCorner(&item, setupData.HeadingAngle, setupData.Distance))
			return false;

		// 3) Assess point collision.
		if ((pointColl.Position.Floor - vPos) >= -STEPUP_HEIGHT &&									 // Floor is within highest floor bound.
			((pointColl.Position.Ceiling - vPos) < -(coll.Setup.Height + (LARA_HEADROOM * 0.8f)) || // Ceiling is within lowest ceiling bound... 
				((pointColl.Position.Ceiling - vPos) < -coll.Setup.Height &&							// OR ceiling is level with Lara's head...
					(pointColl.Position.Floor - vPos) >= CLICK(1 / 2.0f))))								// AND there is a drop below.
		{
			return true;
		}

		return false;
	}

	bool CanJumpUp(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto setupData = JumpSetupData
		{
			0,
			0.0f,
			false
		};

		return TestJumpSetup(item, coll, setupData);
	}

	static bool TestDirectionalStandingJump(const ItemInfo& item, const CollisionInfo& coll, short relHeadingAngle)
	{
		// Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, &item))
			return false;

		auto setupData = JumpSetupData
		{
			short(item.Pose.Orientation.y + relHeadingAngle),
			CLICK(0.85f)
		};

		return TestJumpSetup(item, coll, setupData);
	}

	bool CanJumpForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(0.0f));
	}

	bool CanJumpBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(180.0f));
	}

	bool CanJumpLeft(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(-90.0f));
	}

	bool CanJumpRight(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestDirectionalStandingJump(item, coll, ANGLE(90.0f));
	}

	bool CanRunJumpForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Check running jump timer.
		if (player.Control.Count.Run < LARA_RUN_JUMP_TIME)
			return false;

		auto setupData = JumpSetupData
		{
			item.Pose.Orientation.y,
			CLICK(3 / 2.0f)
		};

		return TestJumpSetup(item, coll, setupData);
	}

	bool CanSprintJumpForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Check whether sprint jump is enabled.
		if (!g_GameFlow->HasSprintJump())
			return false;

		// 2) Check for jump state dispatch.
		if (!HasStateDispatch(&item, LS_JUMP_FORWARD))
			return false;

		// 3) Check run timer.
		if (player.Control.Count.Run < LARA_SPRINT_JUMP_TIME)
			return false;

		auto setupData = JumpSetupData
		{
			item.Pose.Orientation.y,
			CLICK(1.8f)
		};

		// 4) Assess context.
		return TestJumpSetup(item, coll, setupData);
	}

	bool CanPerformSlideJump(const ItemInfo& item, const CollisionInfo& coll)
	{
		// TODO: Get back to this project. -- Sezz 2022.11.11
		return true;

		// Check whether extended slide mechanics are enabled.
		if (!g_GameFlow->HasSlideExtended())
			return true;

		// TODO: Broken on diagonal slides?

		auto pointColl = GetCollision(&item);

		//short aspectAngle = GetLaraSlideHeadingAngle(item, coll);
		//short slopeAngle = Geometry::GetSurfaceSlopeAngle(GetSurfaceNormal(pointColl.FloorTilt, true));
		//return (abs(short(coll.Setup.ForwardAngle - aspectAngle)) <= abs(slopeAngle));
	}

	bool CanCrawlspaceDive(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto pointColl = GetCollision(&item, coll.Setup.ForwardAngle, coll.Setup.Radius, -coll.Setup.Height);
		return (abs(pointColl.Position.Ceiling - pointColl.Position.Floor) < LARA_HEIGHT || IsInLowSpace(item, coll));
	}

	bool CanPerformLedgeJump(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LEDGE_HEIGHT_MIN = CLICK(2);

		// 1) Check if ledge jumps are enabled.
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

		// 2) Assess level geometry ray collision.
		if (LOS(&origin, &target))
			return false;

		// TODO: Assess static object geometry ray collision.

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - LARA_HEIGHT_STRETCH);

		// 3) Assess point collision.
		if (relCeilHeight >= -coll.Setup.Height) // Ceiling height is below upper ceiling bound.
			return false;

		return true;
	}

	bool CanDismountTightrope(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		auto pointColl = GetCollision(&item);

		if (player.Control.Tightrope.CanDismount &&			   // Dismount is allowed.
			pointColl.Position.Floor == item.Pose.Position.y) // Floor is level with player.
		{
			return true;
		}

		return false;
	}
}
