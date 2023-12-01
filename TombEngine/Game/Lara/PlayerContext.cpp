#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/AttractorCollision.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"

using namespace TEN::Collision::Attractor;
using namespace TEN::Collision::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Player
{
	PlayerContext::PlayerContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		/*ItemPtr = &item;
		PlayerPtr = &player;
		CollPtr = &coll;*/
	}

	bool CanChangeElevation(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND = -STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2); // NOTE: Height offset required for correct bridge collision.
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// 1) Test if player is already aligned with floor.
		if (relFloorHeight == 0)
			return false;

		// 2) Assess point collision and player status.
		if ((relFloorHeight <= LOWER_FLOOR_BOUND ||					// Floor height is above lower floor bound.
				player.Control.WaterStatus == WaterStatus::Wade) && // OR player is wading.
			relFloorHeight >= UPPER_FLOOR_BOUND)					// Floor height is below upper floor bound.
		{
			return true;
		}

		return false;
	}

	static bool CanPerformStep(const ItemInfo& item, const CollisionInfo& coll, bool isGoingUp)
	{
		constexpr auto LOWER_FLOOR_BOUND_UP	  = -CLICK(0.75f);
		constexpr auto UPPER_FLOOR_BOUND_UP	  = -STEPUP_HEIGHT;
		constexpr auto LOWER_FLOOR_BOUND_DOWN = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND_DOWN = CLICK(0.75f);

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2); // NOTE: Height offset required for correct bridge collision.
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;

		// Determine appropriate floor bounds.
		int lowerFloorBound = isGoingUp ? LOWER_FLOOR_BOUND_UP : LOWER_FLOOR_BOUND_DOWN;
		int upperFloorBound = isGoingUp ? UPPER_FLOOR_BOUND_UP : UPPER_FLOOR_BOUND_DOWN;

		// Assess point collision.
		if (relFloorHeight <= lowerFloorBound && // Floor height is above lower floor bound.
			relFloorHeight >= upperFloorBound)	 // Floor height is below upper floor bound.
		{
			return true;
		}

		return false;
	}

	bool CanStepUp(const ItemInfo& item, const CollisionInfo& coll)
	{
		return CanPerformStep(item, coll, true);
	}

	bool CanStepDown(const ItemInfo& item, const CollisionInfo& coll)
	{
		return CanPerformStep(item, coll, false);
	}

	bool CanStrikeAfkPose(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Check if AFK posing is enabled.
		if (!g_GameFlow->HasAFKPose())
			return false;

		// 2) Test AFK pose timer.
		if (player.Control.Count.Pose < PLAYER_POSE_TIME)
			return false;

		// 3) Test player hand and water status.
		if (player.Control.HandStatus != HandStatus::Free ||
			player.Control.WaterStatus == WaterStatus::Wade)
		{
			return false;
		}

		// 4) Assess player status.
		if (!(IsHeld(In::Flare) || IsHeld(In::Draw)) &&				   // Avoid unsightly concurrent actions.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare.
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
		if (player.Control.WaterStatus == WaterStatus::Wade || // Is wading.
			TestEnvironment(ENV_FLAG_SWAMP, &item))			   // OR is in swamp.
		{
			return true;
		}

		return false;
	}

	bool CanTurnFast(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Test player turn rate.
		if (isGoingRight)
		{
			if (player.Control.TurnRate/*.y*/ >= LARA_SLOW_TURN_RATE_MAX)
				return true;
		}
		else
		{
			if (player.Control.TurnRate/*.y*/ <= -LARA_SLOW_TURN_RATE_MAX)
				return true;
		}

		// 2) Assess player status.
		if (player.Control.WaterStatus == WaterStatus::Dry &&
			((player.Control.HandStatus == HandStatus::WeaponReady && player.Control.Weapon.GunType != LaraWeaponType::Torch) ||
				(player.Control.HandStatus == HandStatus::WeaponDraw && player.Control.Weapon.GunType != LaraWeaponType::Flare)))
		{
			return true;
		}

		return false;
	}

	static bool TestGroundMovementSetup(const ItemInfo& item, const CollisionInfo& coll, const GroundMovementSetupData& setup, bool isCrawling = false)
	{
		constexpr auto SLOPE_ASPECT_ANGLE_DELTA_MAX = ANGLE(90.0f);

		// HACK: coll.Setup.Radius and coll.Setup.Height are set only in lara_col functions and then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any context assessment functions called in crouch/crawl lara_as routines.
		// If states become objects, a dedicated state init function should eliminate the need for the isCrawling parameter. -- Sezz 2022.03.16
		int playerRadius = isCrawling ? LARA_RADIUS_CRAWL : coll.Setup.Radius;
		int playerHeight = isCrawling ? LARA_HEIGHT_CRAWL : coll.Setup.Height;

		// Get point collision.
		auto pointColl = GetCollision(&item, setup.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);
		int vPos = item.Pose.Position.y;
		int vPosTop = vPos - playerHeight;

		// Calculate slope aspect delta angle.
		short aspectAngle = Geometry::GetSurfaceAspectAngle(pointColl.FloorNormal);
		short aspectAngleDelta = Geometry::GetShortestAngle(setup.HeadingAngle, aspectAngle);

		// 1) Check for slippery slope below floor (if applicable).
		if (setup.TestSlipperySlopeBelow &&
			(pointColl.Position.FloorSlope && abs(aspectAngleDelta) <= SLOPE_ASPECT_ANGLE_DELTA_MAX))
		{
			return false;
		}
		
		// 1) Check for slippery slope above floor (if applicable).
		if (setup.TestSlipperySlopeAbove &&
			(pointColl.Position.FloorSlope && abs(aspectAngleDelta) >= SLOPE_ASPECT_ANGLE_DELTA_MAX))
		{
			return false;
		}

		// 3) Check for death floor (if applicable).
		if (setup.TestDeathFloor && pointColl.Block->Flags.Death)
			return false;

		// LOS setup at upper floor bound.
		auto origin0 = GameVector(
			item.Pose.Position.x,
			(vPos + setup.UpperFloorBound) - 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target0 = GameVector(
			pointColl.Coordinates.x,
			(vPos + setup.UpperFloorBound) - 1,
			pointColl.Coordinates.z,
			item.RoomNumber);

		// LOS setup at lowest ceiling bound (player height).
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

		// Calculate LOS direction.
		auto origin = target0.ToVector3();
		auto target = target1.ToVector3();
		auto dir = target - origin;
		dir.Normalize();

		// 4) Assess static LOS.
		auto staticLos = GetStaticObjectLos(origin, item.RoomNumber, dir, Vector3::Distance(origin, target), false);
		if (staticLos.has_value())
			return false;

		// 5) Assess room LOS.
		if (!LOS(&origin0, &target0) || !LOS(&origin1, &target1))
			return false;

		int relFloorHeight = pointColl.Position.Floor - vPos;
		int relCeilHeight = pointColl.Position.Ceiling - vPos;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 6) Assess point collision.
		if (relFloorHeight <= setup.LowerFloorBound && // Floor height is above lower floor bound.
			relFloorHeight >= setup.UpperFloorBound && // Floor height is below upper floor bound.
			relCeilHeight < -playerHeight &&		   // Ceiling height is above player height.
			floorToCeilHeight > playerHeight)		   // Floor-to-ceiling height isn't too narrow.
		{
			return true;
		}

		return false;
	}

	bool CanRoll180Running(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		if (player.Control.WaterStatus != WaterStatus::Wade &&
			!player.Control.IsRunJumpQueued) // NOTE: Queued running jump blocks 180 roll by design.
		{
			return true;
		}

		return false;
	}

	bool CanRunForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by run forward state.
			false, true, false
		};

		return TestGroundMovementSetup(item, coll, setup);
	}

	bool CanRunBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by run backward state.
			false, false, false
		};

		return TestGroundMovementSetup(item, coll, setup);
	}

	bool CanWalkForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by walk forward state.
		};

		return TestGroundMovementSetup(item, coll, setup);
	}

	bool CanWalkBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // NOTE: Bounds defined by walk backward state.
		};

		return TestGroundMovementSetup(item, coll, setup);
	}

	static bool TestSidestep(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		const auto& player = GetLaraInfo(item);

		auto setup = GroundMovementSetupData{};

		// Wade case.
		if (player.Control.WaterStatus == WaterStatus::Wade)
		{
			setup = GroundMovementSetupData
			{
				short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				-MAX_HEIGHT, -(int)CLICK(1.25f), // NOTE: Upper bound defined by sidestep left/right states.
				false, false, false
			};
		}
		// Regular case.
		else
		{
			setup = GroundMovementSetupData
			{
				short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				(int)CLICK(0.8f), -(int)CLICK(0.8f) // NOTE: Bounds defined by sidestep left/right states.
			};
		}

		return TestGroundMovementSetup(item, coll, setup);
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

		auto setup = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by wade forward state.
			false, false, false
		};

		// 2) Assess context.
		return TestGroundMovementSetup(item, coll, setup);
	}

	bool CanWadeBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		auto setup = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by walk backward state.
			false, false, false
		};

		// 2) Assess context.
		return TestGroundMovementSetup(item, coll, setup);
	}

	bool CanVaultFromSprint(const ItemInfo& item, const CollisionInfo& coll)
	{
		return !TestLaraWall(&item, OFFSET_RADIUS(coll.Setup.Radius), -BLOCK(5 / 8.0f));
	}

	bool CanSlide(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_FLOOR_BOUND = STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		// Get point collision.
		auto pointColl = GetCollision(&item, 0, 0, -coll.Setup.Height / 2); // NOTE: Offset required for correct bridge collision.
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

		// Get center point collision.
		auto pointCollCenter = GetCollision(&item, 0, 0.0f, -LARA_HEIGHT / 2);
		int floorToCeilHeightCenter = abs(pointCollCenter.Position.Ceiling - pointCollCenter.Position.Floor);

		// Assess center point collision.
		if (floorToCeilHeightCenter < LARA_HEIGHT ||					// Floor-to-ceiling height isn't too wide.
			abs(coll.Middle.Ceiling - LARA_HEIGHT_CRAWL) < LARA_HEIGHT) // Consider statics overhead detected by GetCollisionInfo().
		{
			return true;
		}

		// TODO: Check whether < or <= and > or >=.

		// Get front point collision.
		auto pointCollFront = GetCollision(&item, item.Pose.Orientation.y, radius, -coll.Setup.Height);
		int floorToCeilHeightFront = abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor);
		int relFloorHeightFront = abs(pointCollFront.Position.Floor - pointCollCenter.Position.Floor);

		// Assess front point collision.
		if (relFloorHeightFront <= CRAWL_STEPUP_HEIGHT && // Floor is within upper/lower floor bounds.
			floorToCeilHeightFront < LARA_HEIGHT &&		  // Floor-to-ceiling height isn't too wide.
			floorToCeilHeightFront > LARA_HEIGHT_CRAWL)	  // Floor-to-ceiling height isn't too narrow.
		{
			return true;
		}

		// Get back point collision.
		auto pointCollBack = GetCollision(&item, item.Pose.Orientation.y, -radius, -coll.Setup.Height);
		int floorToCeilHeightBack = abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor);
		int relFloorHeightBack = abs(pointCollBack.Position.Floor - pointCollCenter.Position.Floor);

		// Assess back point collision.
		if (relFloorHeightBack <= CRAWL_STEPUP_HEIGHT && // Floor is within upper/lower floor bounds.
			floorToCeilHeightBack < LARA_HEIGHT &&		 // Floor-to-ceiling height isn't too wide.
			floorToCeilHeightBack > LARA_HEIGHT_CRAWL)	 // Floor-to-ceiling height isn't too narrow.
		{
			return true;
		}

		return false;
	}

	bool CanCrouch(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Assess player status.
		if (player.Control.WaterStatus != WaterStatus::Wade &&			  // Player is wading.
			(player.Control.HandStatus == HandStatus::Free ||			  // Player hands are free.
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
		if (!(IsHeld(In::Flare) || IsHeld(In::Draw)) &&				   // Avoid unsightly concurrent actions.
			player.Control.HandStatus == HandStatus::Free &&		   // Hands are free.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare. TODO: Should be allowed, but flare animation bugs out. -- Sezz 2022.03.18
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
		float dist = 0.0f;
		auto pointColl0 = GetCollision(&item);

		// 3) Test continuity of path.
		while (dist < PROBE_DIST_MAX)
		{
			// Get point collision.
			dist += STEP_DIST;
			auto pointColl1 = GetCollision(&item, item.Pose.Orientation.y, dist, -LARA_HEIGHT_CRAWL);

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
		auto setup = GroundMovementSetupData
		{
			item.Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Bounds defined by crawl forward state.
		};

		return TestGroundMovementSetup(item, coll, setup, true);
	}

	bool CanCrawlBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = GroundMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Bounds defined by crawl backward state.
		};

		return TestGroundMovementSetup(item, coll, setup, true);
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

	static bool TestMonkeySwingSetup(const ItemInfo& item, const CollisionInfo& coll, const MonkeySwingMovementSetupData& setup)
	{
		// HACK: Have to make the height explicit for now. -- Sezz 2022.07.28
		constexpr auto PLAYER_HEIGHT = LARA_HEIGHT_MONKEY;

		// Get point collision.
		auto pointColl = GetCollision(&item, setup.HeadingAngle, OFFSET_RADIUS(coll.Setup.Radius));

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
			(vPosTop + setup.LowerCeilingBound) + 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target1 = GameVector(
			pointColl.Coordinates.x,
			(vPosTop + setup.LowerCeilingBound) + 1,
			pointColl.Coordinates.z,
			item.RoomNumber);

		// Prepare data for static object LOS.
		auto origin = target0.ToVector3();
		auto target = target1.ToVector3();
		auto dir = target - origin;
		dir.Normalize();

		// 3) Assess ray-static collision.
		auto staticLos = GetStaticObjectLos(origin, item.RoomNumber, dir, Vector3::Distance(origin, target), false);
		if (staticLos.has_value())
			return false;

		// 3) Assess level geometry ray collision.
		if (!LOS(&origin0, &target0) || !LOS(&origin1, &target1))
			return false;

		// TODO: Assess static object geometry ray collision.

		int relFloorHeight = pointColl.Position.Floor - vPos;
		int relCeilHeight = pointColl.Position.Ceiling - vPosTop;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 4) Assess point collision.
		if (relFloorHeight > 0 &&						// Floor is within highest floor bound (player base).
			relCeilHeight <= setup.LowerCeilingBound && // Ceiling is within lower ceiling bound.
			relCeilHeight >= setup.UpperCeilingBound && // Ceiling is within upper ceiling bound.
			floorToCeilHeight > PLAYER_HEIGHT)			// Space is not too narrow.
		{
			return true;
		}

		return false;
	}

	bool CanMonkeyForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = MonkeySwingMovementSetupData
		{
			item.Pose.Orientation.y,
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Bounds defined by monkey forward state.
		};

		return TestMonkeySwingSetup(item, coll, setup);
	}

	bool CanMonkeyBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = MonkeySwingMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Bounds defined by monkey backward state.
		};

		return TestMonkeySwingSetup(item, coll, setup);
	}

	static bool TestMonkeyShimmy(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		auto setup = MonkeySwingMovementSetupData
		{
			short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
			(int)CLICK(0.5f), (int)-CLICK(0.5f) // NOTE: Bounds defined by monkey shimmy left/right states.
		};

		return TestMonkeySwingSetup(item, coll, setup);
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
		// 1) Check airborne status and vertical velocity.
		if (!item.Animation.IsAirborne || item.Animation.Velocity.y < 0.0f)
			return false;

		// 2) Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, &item))
			return true;

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int vPos = item.Pose.Position.y;

		// 3) Assess point collision.
		if ((pointColl.Position.Floor - vPos) <= item.Animation.Velocity.y) // Floor height is above projected vertical position.
			return true;

		return false;
	}

	bool CanPerformJump(const ItemInfo& item, const CollisionInfo& coll)
	{
		return !TestEnvironment(ENV_FLAG_SWAMP, &item);
	}

	static bool TestJumpSetup(const ItemInfo& item, const CollisionInfo& coll, const JumpSetupData& setup)
	{
		const auto& player = GetLaraInfo(item);

		bool isWading = setup.TestWadeStatus ? (player.Control.WaterStatus == WaterStatus::Wade) : false;
		bool isInSwamp = TestEnvironment(ENV_FLAG_SWAMP, &item);

		// 1) Check for swamp or wade status (if applicable).
		if (isWading || isInSwamp)
			return false;

		// 2) Check for corner.
		if (TestLaraFacingCorner(&item, setup.HeadingAngle, setup.Distance))
			return false;

		// Prepare data for static object LOS.
		// TODO: Lock-up occurs.
		/*auto origin = Geometry::TranslatePoint(item.Pose.Position.ToVector3(), item.Pose.Orientation.y, OFFSET_RADIUS(coll.Setup.Radius));
		auto target = origin + Vector3(0.0f,-STEPUP_HEIGHT, 0.0f);
		auto dir = target - origin;
		dir.Normalize();

		// 3) Assess ray-static collision.
		auto staticLos = GetStaticObjectLos(origin, item.RoomNumber, dir, Vector3::Distance(origin, target), false);
		if (staticLos.has_value())
			return false;*/

		// Get point collision.
		auto pointColl = GetCollision(&item, setup.HeadingAngle, setup.Distance, -coll.Setup.Height);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;
		int relCeilHeight = pointColl.Position.Ceiling - item.Pose.Position.y;

		// 4) Assess point collision.
		if (relFloorHeight >= -STEPUP_HEIGHT &&								  // Floor is within highest floor bound.
			(relCeilHeight < -(coll.Setup.Height + (LARA_HEADROOM * 0.8f)) || // Ceiling is within lowest ceiling bound.
				(relCeilHeight < -coll.Setup.Height &&						  // OR ceiling is level with Lara's head.
					relFloorHeight >= CLICK(0.5f))))						  // AND there is a drop below.
		{
			return true;
		}

		return false;
	}

	bool CanJumpUp(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto setup = JumpSetupData
		{
			0,
			0.0f,
			false
		};

		return TestJumpSetup(item, coll, setup);
	}

	static bool TestDirectionalStandingJump(const ItemInfo& item, const CollisionInfo& coll, short relHeadingAngle)
	{
		// Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, &item))
			return false;

		auto setup = JumpSetupData
		{
			short(item.Pose.Orientation.y + relHeadingAngle),
			CLICK(0.85f)
		};

		return TestJumpSetup(item, coll, setup);
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

	bool CanQueueRunningJump(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto UPPER_FLOOR_BOUND	 = CLICK(0.5f);
		constexpr auto LOWER_CEIL_BOUND_BASE = -LARA_HEADROOM * 0.8f;

		auto& player = GetLaraInfo(item);

		// 1) Test if running jump is immediately possible.
		if (CanRunJumpForward(item, coll))
			return IsRunJumpQueueableState(item.Animation.TargetState);

		// Get point collision.
		auto pointColl = GetCollision(&item, item.Pose.Orientation.y, BLOCK(1), -coll.Setup.Height);

		int lowerCeilingBound = (LOWER_CEIL_BOUND_BASE - coll.Setup.Height);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;
		int relCeilHeight = pointColl.Position.Ceiling - item.Pose.Position.y;

		// 2) Assess point collision for possible running jump ahead.
		if (relCeilHeight < lowerCeilingBound || // Ceiling height is above lower ceiling bound.
			relFloorHeight >= UPPER_FLOOR_BOUND) // OR floor height ahead is below upper floor bound.
		{
			return IsRunJumpQueueableState(item.Animation.TargetState);
		}

		return false;
	}

	bool CanRunJumpForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Check running jump timer.
		if (player.Control.Count.Run < PLAYER_RUN_JUMP_TIME)
			return false;

		auto setup = JumpSetupData
		{
			item.Pose.Orientation.y,
			CLICK(3 / 2.0f)
		};

		return TestJumpSetup(item, coll, setup);
	}

	bool CanSprintJumpForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Check if sprint jump is enabled.
		if (!g_GameFlow->HasSprintJump())
			return false;

		// 2) Check for jump state dispatch.
		if (!HasStateDispatch(&item, LS_JUMP_FORWARD))
			return false;

		// 3) Check running jump timer.
		if (player.Control.Count.Run < PLAYER_SPRINT_JUMP_TIME)
			return false;

		auto setup = JumpSetupData
		{
			item.Pose.Orientation.y,
			CLICK(1.8f)
		};

		// 4) Assess context.
		return TestJumpSetup(item, coll, setup);
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

	static bool TestLedgeClimbSetup(const ItemInfo& item, CollisionInfo& coll, const LedgeClimbSetupData& setup)
	{
		constexpr auto ABS_FLOOR_BOUND = CLICK(0.8);

		// Get point collision.
		int probeHeight = -(LARA_HEIGHT_STRETCH + ABS_FLOOR_BOUND);
		auto pointCollCenter = GetCollision(&item);
		auto pointCollFront = GetCollision(&item, setup.HeadingAngle, OFFSET_RADIUS(coll.Setup.Radius), probeHeight);

		int vPosTop = item.Pose.Position.y - LARA_HEIGHT_STRETCH;
		int relFloorHeight = abs(pointCollFront.Position.Floor - vPosTop);
		int floorToCeilHeight = abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor);
		int gapHeight = abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor);

		// TODO: This check fails for no reason.
		// 1) Test for illegal slope (if applicable).
		bool isIllegalSlope = setup.TestIllegalSlope ? pointCollFront.Position.FloorSlope : false;
		if (isIllegalSlope)
			return false;

		// 2) Test for object blocking ledge.
		TestForObjectOnLedge(&item, &coll);
		if (coll.HitStatic)
			return false;

		// 3) Test for valid ledge.
		// TODO: Attractors.

		// 4) Assess point collision.
		if (relFloorHeight <= ABS_FLOOR_BOUND &&			   // Floor height is within lower/upper floor bounds.
			floorToCeilHeight > setup.FloorToCeilHeightMin &&  // Floor-to-ceiling height isn't too narrow.
			floorToCeilHeight <= setup.FloorToCeilHeightMax && // Floor-to-ceiling height isn't too wide.
			gapHeight >= setup.GapHeightMin)				   // Gap height is permissive.
		{
			return true;
		}

		return false;
	}	

	bool CanSwingOnLedge(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto UPPER_FLOOR_BOUND = 0;
		constexpr auto LOWER_CEIL_BOUND	 = CLICK(1.5f);

		auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetCollision(&item, item.Pose.Orientation.y, OFFSET_RADIUS(coll.Setup.Radius));
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;
		int relCeilHeight = pointColl.Position.Ceiling - (item.Pose.Position.y - coll.Setup.Height);

		// Assess point collision.
		if (relFloorHeight >= UPPER_FLOOR_BOUND && // Floor height is below upper floor bound.
			relCeilHeight <= LOWER_CEIL_BOUND)	   // Ceiling height is above lower ceiling bound.
		{
			return true;
		}

		return false;
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

	bool CanPerformLedgeHandstand(const ItemInfo& item, CollisionInfo& coll)
	{
		auto setup = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT, -MAX_HEIGHT,
			CLICK(3),
			false
		};

		return TestLedgeClimbSetup(item, coll, setup);
	}

	bool CanClimbLedgeToCrouch(const ItemInfo& item, CollisionInfo& coll)
	{
		auto setup = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,
			(int)CLICK(0.6f),
			true
		};

		return TestLedgeClimbSetup(item, coll, setup);
	}

	bool CanClimbLedgeToStand(const ItemInfo& item, CollisionInfo& coll)
	{
		auto setup = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT, -MAX_HEIGHT,
			CLICK(1),
			false
		};

		return TestLedgeClimbSetup(item, coll, setup);
	}

	bool CanShimmyUp(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto WALL_STEP_HEIGHT = -CLICK(1);

		auto& player = GetLaraInfo(item);

		// 1) Check for climbable wall flag.
		if (!player.Control.CanClimbLadder)
			return false;

		// Get point collision.
		auto pointCollCenter = GetCollision(&item);
		auto pointCollLeft = GetCollision(&item, item.Pose.Orientation.y - ANGLE(90.0f), OFFSET_RADIUS(coll.Setup.Radius));
		auto pointCollRight = GetCollision(&item, item.Pose.Orientation.y + ANGLE(90.0f), OFFSET_RADIUS(coll.Setup.Radius));

		int vPos = item.Pose.Position.y - LARA_HEIGHT_STRETCH;
		int relCeilHeightCenter = pointCollCenter.Position.Ceiling - vPos;
		int relCeilHeightLeft = pointCollCenter.Position.Ceiling - vPos;
		int relCeilHeightRight = pointCollCenter.Position.Ceiling - vPos;

		// 2) Assess point collision.
		if (relCeilHeightCenter <= WALL_STEP_HEIGHT &&
			relCeilHeightLeft <= WALL_STEP_HEIGHT &&
			relCeilHeightRight <= WALL_STEP_HEIGHT)
		{
			return true;
		}

		return false;
	}

	// TODO!!
	bool CanShimmyDown(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto WALL_STEP_HEIGHT = CLICK(1);

		auto& player = GetLaraInfo(item);

		auto pointCollCenter = GetCollision(&item);

		int relFloorHeight = pointCollCenter.Position.Floor - item.Pose.Position.y;
		// Left and right.

		// 1) Check if wall is climbable.
		if (!player.Control.CanClimbLadder)
			return false;

		// 2) Assess point collision.
		if (relFloorHeight >= WALL_STEP_HEIGHT)
			return true;

		return false;
	}

	bool CanShimmyLeft(ItemInfo& item, CollisionInfo& coll)
	{
		return true;

		//return TestLaraHangSideways(&item, &coll, ANGLE(-90.0f));
	}

	bool CanShimmyRight(ItemInfo& item, CollisionInfo& coll)
	{
		return true;

		//return TestLaraHangSideways(&item, &coll, ANGLE(90.0f));
	}

	bool CanDismountTightrope(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		auto pointColl = GetCollision(&item);

		if (player.Control.Tightrope.CanDismount &&			  // Dismount is allowed.
			pointColl.Position.Floor == item.Pose.Position.y) // Floor is level with player.
		{
			return true;
		}

		return false;
	}

	static std::optional<AttractorCollisionData> GetVaultEdgeAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																				const VaultSetupData& setup,
																				const std::vector<AttractorCollisionData>& attracColls,
																				bool testLedge = true)
	{
		constexpr auto SWAMP_DEPTH_MAX = -CLICK(3);

		const auto& player = GetLaraInfo(item);

		// 1) Test swamp depth (if applicable).
		if (setup.TestSwampDepth)
		{
			if (TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber) && player.Context.WaterSurfaceDist < SWAMP_DEPTH_MAX)
				return std::nullopt;
		}

		// 2) Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 2.1) Check if attractor is edge type.
			if (attracColl.AttracPtr->GetType() != AttractorType::Edge)
				continue;

			// 2.2) Test if edge is within range.
			float range = OFFSET_RADIUS(coll.Setup.Radius);
			if (attracColl.Proximity.Distance2D > range)
				continue;

			// 2.3) Test if edge slope is illegal.
			if (abs(attracColl.SlopeAngle) >= ILLEGAL_FLOOR_SLOPE_ANGLE)
				continue;

			// 2.4) Test vault position and angle.
			if (!attracColl.IsInFront || !attracColl.IsFacingForward ||
				!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
			{
				continue;
			}

			// 2.5) Test if relative edge height is within edge intersection bounds.
			auto relEdgeHeight = attracColl.Proximity.Intersection.y - item.Pose.Position.y;
			if (relEdgeHeight >= setup.LowerEdgeBound || // Player-to-edge height is within lower edge bound.
				relEdgeHeight < setup.UpperEdgeBound)	 // Player-to-edge height is within upper edge bound.
			{
				continue;
			}

			// Get point collision at edge front.
			auto pointCollFront = GetCollision(
				Vector3i(attracColl.Proximity.Intersection), attracColl.AttracPtr->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// TODO: This can be wrong. Range with the previous check may misalign. Prevents vault cheese, but better to leave out.
			// 2.6) Test if edge height is within attractor intersection edge intersection bounds.
			/*int floorToEdgeHeight = pointCollFront.Position.Floor - attracColl.Proximity.Intersection.y;
			if (floorToEdgeHeight >= setup.LowerEdgeBound || // Floor-to-edge height is within lower edge bound.
				floorToEdgeHeight < setup.UpperEdgeBound)	 // Floor-to-edge height is within upper edge bound.
			{
				continue;
			}*/

			// 2.7) Test if ceiling is adequately higher than edge.
			int ceilToEdgeHeight = abs(pointCollFront.Position.Ceiling - attracColl.Proximity.Intersection.y);
			if (ceilToEdgeHeight < setup.EdgeToCeilHeightMin)
				continue;

			// Get point collision at edge back.
			auto pointCollBack = GetCollision(
				Vector3i(attracColl.Proximity.Intersection), attracColl.AttracPtr->GetRoomNumber(),
				attracColl.HeadingAngle, coll.Setup.Radius);

			if (testLedge)
			{
				// 2.8) Test for illegal slope on ledge.
				if (pointCollBack.Position.FloorSlope)
					continue;

				// 2.9) Test ledge floor height.
				int absLedgeFloorHeight = abs(pointCollBack.Position.Floor - attracColl.Proximity.Intersection.y);
				if (absLedgeFloorHeight > STEPUP_HEIGHT)
					continue;

				// 2.10) Test ledge floor-to-ceiling height.
				int ledgeFloorToCeilHeight = abs(pointCollBack.Position.Ceiling - pointCollBack.Position.Floor);
				if (ledgeFloorToCeilHeight <= setup.LedgeFloorToCeilHeightMin ||
					ledgeFloorToCeilHeight > setup.LedgeFloorToCeilHeightMax)
				{
					continue;
				}
			}

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	const std::optional<VaultContextData> Get2StepToStandVaultContext(const ItemInfo& item, const CollisionInfo& coll,
																	  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = VaultSetupData
		{
			-STEPUP_HEIGHT, -(int)CLICK(2.5f), // Edge range.
			LARA_HEIGHT, -MAX_HEIGHT,		   // Ledge floor-to-ceil range.
			CLICK(1),						   // Edge-to-ceil height minimum.
			false							   // Test swamp depth.
		};
		constexpr auto INTERSECT_OFFSET = Vector3(0.0f, CLICK(2), 0.0f);

		// Get vault edge attractor collision.
		auto edgeAttracColl = GetVaultEdgeAttractorCollision(item, coll, SETUP, attracColls);
		if (!edgeAttracColl.has_value())
			return std::nullopt;

		// Create and return vault context.
		auto vaultContext = VaultContextData{};
		vaultContext.AttracPtr = edgeAttracColl->AttracPtr;
		vaultContext.Intersection = edgeAttracColl->Proximity.Intersection + INTERSECT_OFFSET;
		vaultContext.EdgeAngle = edgeAttracColl->HeadingAngle;
		vaultContext.SetBusyHands = true;
		vaultContext.SnapToLedge = true;
		vaultContext.SetJumpVelocity = false;

		return vaultContext;
	}
	
	const std::optional<VaultContextData> Get3StepToStandVaultContext(const ItemInfo& item, const CollisionInfo& coll,
																	  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = VaultSetupData
		{
			-(int)CLICK(2.5f), -(int)CLICK(3.5f), // Edge range.
			LARA_HEIGHT, -MAX_HEIGHT,			  // Ledge floor-to-ceil range.
			CLICK(1),							  // Edge-to-ceil height minimum.
			false								  // Test swamp depth.
		};
		constexpr auto INTERSECT_OFFSET = Vector3(0.0f, CLICK(3), 0.0f);

		// Get vault edge attractor collision.
		auto edgeAttracColl = GetVaultEdgeAttractorCollision(item, coll, SETUP, attracColls);
		if (!edgeAttracColl.has_value())
			return std::nullopt;

		// Create and return vault context.
		auto vaultContext = VaultContextData{};
		vaultContext.AttracPtr = edgeAttracColl->AttracPtr;
		vaultContext.Intersection = edgeAttracColl->Proximity.Intersection + INTERSECT_OFFSET;
		vaultContext.EdgeAngle = edgeAttracColl->HeadingAngle;
		vaultContext.SetBusyHands = true;
		vaultContext.SnapToLedge = true;
		vaultContext.SetJumpVelocity = false;

		return vaultContext;
	}
	
	const std::optional<VaultContextData> Get1StepToCrouchVaultContext(const ItemInfo& item, const CollisionInfo& coll,
																	   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = VaultSetupData
		{
			0, -STEPUP_HEIGHT,				// Edge range.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT, // Ledge floor-to-ceil range.
			CLICK(1),						// Edge-to-ceil height minimum.
			false							// Test swamp depth.
		};
		constexpr auto INTERSECT_OFFSET = Vector3(0.0f, CLICK(1), 0.0f);

		// Get vault edge attractor collision.
		auto edgeAttracColl = GetVaultEdgeAttractorCollision(item, coll, SETUP, attracColls);
		if (!edgeAttracColl.has_value())
			return std::nullopt;

		// Create and return vault context.
		auto vaultContext = VaultContextData{};
		vaultContext.AttracPtr = edgeAttracColl->AttracPtr;
		vaultContext.Intersection = edgeAttracColl->Proximity.Intersection + INTERSECT_OFFSET;
		vaultContext.EdgeAngle = edgeAttracColl->HeadingAngle;
		vaultContext.SetBusyHands = true;
		vaultContext.SnapToLedge = true;
		vaultContext.SetJumpVelocity = false;

		return vaultContext;
	}
	
	const std::optional<VaultContextData> Get2StepToCrouchVaultContext(const ItemInfo& item, const CollisionInfo& coll,
																	   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = VaultSetupData
		{
			-STEPUP_HEIGHT, -(int)CLICK(2.5f), // Edge range.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,	   // Ledge floor-to-ceil range.
			CLICK(1),						   // Edge-to-ceil height minimum.
			false							   // Test swamp depth.
		};
		constexpr auto INTERSECT_OFFSET = Vector3(0.0f, CLICK(2), 0.0f);

		// Get vault edge attractor collision.
		auto edgeAttracColl = GetVaultEdgeAttractorCollision(item, coll, SETUP, attracColls);
		if (!edgeAttracColl.has_value())
			return std::nullopt;

		// Create and return vault context.
		auto vaultContext = VaultContextData{};
		vaultContext.AttracPtr = edgeAttracColl->AttracPtr;
		vaultContext.Intersection = edgeAttracColl->Proximity.Intersection + INTERSECT_OFFSET;
		vaultContext.EdgeAngle = edgeAttracColl->HeadingAngle;
		vaultContext.SetBusyHands = true;
		vaultContext.SnapToLedge = true;
		vaultContext.SetJumpVelocity = false;

		return vaultContext;
	}
	
	const std::optional<VaultContextData> Get3StepToCrouchVaultContext(const ItemInfo& item, const CollisionInfo& coll,
																	   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = VaultSetupData
		{
			-(int)CLICK(2.5f), -(int)CLICK(3.5f), // Edge range.
			LARA_HEIGHT_CRAWL, -LARA_HEIGHT,	  // Ledge floor-to-ceil range.
			CLICK(1),							  // Edge-to-ceil height minimum.
			false								  // Test swamp depth.
		};
		constexpr auto INTERSECT_OFFSET = Vector3(0.0f, CLICK(3), 0.0f);

		// Get vault edge attractor collision.
		auto edgeAttracColl = GetVaultEdgeAttractorCollision(item, coll, SETUP, attracColls);
		if (!edgeAttracColl.has_value())
			return std::nullopt;

		// Create and return vault context.
		auto vaultContext = VaultContextData{};
		vaultContext.AttracPtr = edgeAttracColl->AttracPtr;
		vaultContext.Intersection = edgeAttracColl->Proximity.Intersection + INTERSECT_OFFSET;
		vaultContext.EdgeAngle = edgeAttracColl->HeadingAngle;
		vaultContext.SetBusyHands = true;
		vaultContext.SnapToLedge = true;
		vaultContext.SetJumpVelocity = false;

		return vaultContext;
	}
	
	const std::optional<VaultContextData> GetAutoJumpVaultContext(const ItemInfo& item, const CollisionInfo& coll,
																  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = VaultSetupData
		{
			-(int)CLICK(3.5f), -(int)CLICK(7.5f), // Edge range
			0, -MAX_HEIGHT,						  // Ledge floor-to-ceil range
			1,									  // Edge-to-ceil height minumum
			false								  // Test swamp depth
		};

		// Get vault edge attractor collision.
		auto edgeAttracColl = GetVaultEdgeAttractorCollision(item, coll, SETUP, attracColls, false);
		if (!edgeAttracColl.has_value())
			return std::nullopt;

		// Create and return vault context.
		auto vaultContext = VaultContextData{};
		vaultContext.AttracPtr = edgeAttracColl->AttracPtr;
		vaultContext.Intersection = edgeAttracColl->Proximity.Intersection;
		vaultContext.EdgeAngle = edgeAttracColl->HeadingAngle;
		vaultContext.SetBusyHands = false;
		vaultContext.SnapToLedge = true;
		vaultContext.SetJumpVelocity = false;

		return vaultContext;
	}

	static std::optional<VaultContextData> GetAutoJumpMonkeySwingVaultContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_CEIL_BOUND = -LARA_HEIGHT_MONKEY;
		constexpr auto UPPER_CEIL_BOUND = -CLICK(7);

		const auto& player = GetLaraInfo(item);

		if (!g_GameFlow->HasMonkeyAutoJump())
			return std::nullopt;

		// Get point collision.
		auto pointColl = GetCollision(item);
		int vPos = item.Pose.Position.y;

		// Assess point collision.
		if (player.Control.CanMonkeySwing &&							 // Monkey swing sector flag set.
			(pointColl.Position.Ceiling - vPos) < -LARA_HEIGHT_MONKEY && // Ceiling height is within lower ceiling bound.
			(pointColl.Position.Ceiling - vPos) >= -CLICK(7))			 // Ceiling height is within upper ceiling bound.
		{
			// Create and return vault context.
			auto vaultContext = VaultContextData{};
			vaultContext.Intersection = Vector3(item.Pose.Position.x, pointColl.Position.Ceiling, item.Pose.Position.z);
			vaultContext.SetBusyHands = false;
			vaultContext.SnapToLedge = false;
			vaultContext.SetJumpVelocity = false;

			return vaultContext;
		}

		return std::nullopt;
	}
	std::optional<VaultContextData> GetVaultContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(2);

		const auto& player = GetLaraInfo(item);

		// Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return std::nullopt;

		// Test swamp depth.
		//if (TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber) && player.Context.WaterSurfaceDist < -CLICK(3))
		//	return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item, coll.Setup.Radius, 0.0f, 0.0f, ATTRAC_DETECT_RADIUS);

		auto vaultContext = std::optional<VaultContextData>();

		// Vault to crouch up 1 step.
		vaultContext = Get1StepToCrouchVaultContext(item, coll, attracColls);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_VAULT_1_STEP_CROUCH;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		// Vault up 2 steps to stand.
		vaultContext = Get2StepToStandVaultContext(item, coll, attracColls);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_VAULT_2_STEPS;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		// Vault to crouch up 2 steps.
		vaultContext = Get2StepToCrouchVaultContext(item, coll, attracColls);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_VAULT_2_STEPS_CROUCH;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		// Vault to stand up 3 steps.
		vaultContext = Get3StepToStandVaultContext(item, coll, attracColls);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_VAULT_3_STEPS;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		// Vault to crouch up 3 steps.
		vaultContext = Get3StepToCrouchVaultContext(item, coll, attracColls);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_VAULT_3_STEPS_CROUCH;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		// Vault auto jump.
		vaultContext = GetAutoJumpVaultContext(item, coll, attracColls);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_AUTO_JUMP;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		// TODO: Move ladder checks here when ladders are less prone to breaking.
		// In this case, they fail due to a reliance on ShiftItem(). -- Sezz 2021.02.05

		// Vault auto jump to monkey swing.
		vaultContext = GetAutoJumpMonkeySwingVaultContext(item, coll);
		if (vaultContext.has_value())
		{
			vaultContext->TargetStateID = LS_AUTO_JUMP;
			if (!HasStateDispatch(&item, vaultContext->TargetStateID))
				return std::nullopt;

			return vaultContext;
		}

		return std::nullopt;
	}

	static std::optional<AttractorCollisionData> GetEdgeCatchAttractorCollision(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto FLOOR_TO_EDGE_HEIGHT_MIN = LARA_HEIGHT_STRETCH;

		// Get attractor collisions.
		float detectRadius = OFFSET_RADIUS(std::max((float)coll.Setup.Radius, item.Animation.Velocity.Length()));
		auto attracColls = GetAttractorCollisions(item, coll.Setup.Radius, -coll.Setup.Height, 0.0f, detectRadius);

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check if attractor is edge type.
			if (attracColl.AttracPtr->GetType() != AttractorType::Edge)
				continue;

			// 2) Test if edge slope is illegal.
			if (abs(attracColl.SlopeAngle) >= ILLEGAL_FLOOR_SLOPE_ANGLE)
				continue;

			// 3) Test catch position and angle.
			if (!attracColl.IsInFront || !attracColl.IsFacingForward ||
				!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
			{
				continue;
			}

			// Get point collision at edge front.
			auto pointColl = GetCollision(
				Vector3i(attracColl.Proximity.Intersection), attracColl.AttracPtr->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// 4) Test if edge is high enough off ground.
			int floorToEdgeHeight = pointColl.Position.Floor - attracColl.Proximity.Intersection.y;
			if (floorToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN)
				continue;

			// 5) Test if ceiling is higher than edge.
			int ceilToEdgeHeight = (pointColl.Position.Ceiling - attracColl.Proximity.Intersection.y);
			if (ceilToEdgeHeight >= 0)
				continue;

			int vPos = item.Pose.Position.y - coll.Setup.Height;
			int relEdgeHeight = attracColl.Proximity.Intersection.y - vPos;

			bool isMovingUp = (item.Animation.Velocity.y <= 0.0f);
			int lowerBound = isMovingUp ? 0 : (int)round(item.Animation.Velocity.y);
			int upperBound = isMovingUp ? (int)round(item.Animation.Velocity.y) : 0;

			// 6) Assess catch trajectory.
			if (relEdgeHeight <= lowerBound && // Edge height is above lower height bound.
				relEdgeHeight >= upperBound)   // Edge height is below upper height bound.
			{
				// Return closest edge attractor collision.
				return attracColl;
			}
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<EdgeCatchData> GetAttractorEdgeCatch(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Get edge catch attractor collision.
		auto attracColl = GetEdgeCatchAttractorCollision(item, coll);
		if (!attracColl.has_value())
			return std::nullopt;

		// TODO: Not needed? Handled by hang function.
		// Calculate heading angle. NOTE: Less accurate if edge catch spans connecting attractors.
		auto pointLeft = attracColl->AttracPtr->GetIntersectionAtChainDistance(attracColl->Proximity.ChainDistance - coll.Setup.Radius);
		auto pointRight = attracColl->AttracPtr->GetIntersectionAtChainDistance(attracColl->Proximity.ChainDistance + coll.Setup.Radius);
		short headingAngle = Geometry::GetOrientToPoint(pointLeft, pointRight).y - ANGLE(90.0f);

		// Return edge catch data.
		return EdgeCatchData
		{
			attracColl->AttracPtr,
			EdgeType::Attractor,
			attracColl->Proximity.Intersection,
			attracColl->Proximity.ChainDistance,
			headingAngle
		};
	}

	static std::optional<EdgeCatchData> GetClimbableWallEdgeCatch(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto WALL_STEP_HEIGHT = CLICK(1);

		const auto& player = GetLaraInfo(item);

		// 1) Check for climbable wall flag.
		if (!player.Control.CanClimbLadder)
			return std::nullopt;

		// TODO.
		// 2) Test for valid ledge on climbable wall.
		//if (!TestValidLedge(&item, &coll, true))
			return std::nullopt;

		// 3) Test movement direction.
		bool isMovingUp = (item.Animation.Velocity.y <= 0.0f);
		if (isMovingUp)
			return std::nullopt;

		// Get point collision.
		auto pointCollCenter = GetCollision(&item);

		int vPos = item.Pose.Position.y - coll.Setup.Height;
		int edgeHeight = (int)floor((vPos + item.Animation.Velocity.y) / WALL_STEP_HEIGHT) * WALL_STEP_HEIGHT;

		// 4) Test if wall edge is high enough off the ground.
		int floorToEdgeHeight = abs(edgeHeight - pointCollCenter.Position.Floor);
		if (floorToEdgeHeight <= LARA_HEIGHT_STRETCH)
			return std::nullopt;

		// Get point collision.
		float probeHeight = -(coll.Setup.Height + abs(item.Animation.Velocity.y));
		auto pointCollFront = GetCollision(&item, item.Pose.Orientation.y, OFFSET_RADIUS(coll.Setup.Radius), probeHeight);

		// 5) Test if edge is on wall.
		if (edgeHeight < pointCollFront.Position.Floor && // Edge is above floor.
			edgeHeight > pointCollFront.Position.Ceiling) // Edge is below ceiling.
		{
			return std::nullopt;
		}

		// TODO
		// 6) Test if climbable wall is valid.
		bool isClimbableWall = TestLaraHangOnClimbableWall(&item, &coll);
		//if (!isClimbableWall && !TestValidLedge(&item, &coll, true, true))
			return std::nullopt;

		// 7) Assess point collision to wall edge.
		int relEdgeHeight = edgeHeight - vPos;
		if (relEdgeHeight <= item.Animation.Velocity.y && // Edge height is above lower height bound.
			relEdgeHeight >= 0)							  // Edge height is below upper height bound.
		{
			auto offset = Vector3(0.0f, edgeHeight, 0.0f);
			return EdgeCatchData{ nullptr, EdgeType::ClimbableWall, offset };
		}

		return std::nullopt;
	}

	std::optional<EdgeCatchData> GetEdgeCatch(ItemInfo& item, CollisionInfo& coll)
	{
		// 1) Get and return edge catch.
		auto edgeCatch = GetAttractorEdgeCatch(item, coll);
		if (edgeCatch.has_value())
			return edgeCatch;

		// 2) Get and return climbable wall edge catch.
		auto wallEdgeCatch = GetClimbableWallEdgeCatch(item, coll);
		if (wallEdgeCatch.has_value())
			return wallEdgeCatch;

		// No valid edge catch; return nullopt.
		return std::nullopt;
	}

	std::optional<MonkeySwingCatchData> GetMonkeySwingCatch(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_CEIL_BOUND			= CLICK(0.5f);
		constexpr auto FLOOR_TO_CEIL_HEIGHT_MAX = LARA_HEIGHT_MONKEY;

		const auto& player = GetLaraInfo(item);

		// 1) Check for monkey swing flag.
		if (!player.Control.CanMonkeySwing)
			return std::nullopt;

		// 2) Check collision type.
		if (coll.CollisionType != CollisionType::CT_TOP &&
			coll.CollisionType != CollisionType::CT_TOP_FRONT)
		{
			return std::nullopt;
		}

		// Get point collision.
		auto pointColl = GetCollision(&item);

		int vPos = item.Pose.Position.y - coll.Setup.Height;
		int relCeilHeight = pointColl.Position.Ceiling - vPos;
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

		// 3) Assess point collision.
		if (abs(relCeilHeight) <= ABS_CEIL_BOUND &&		  // Ceiling height is within lower/upper ceiling bounds.
			floorToCeilHeight > FLOOR_TO_CEIL_HEIGHT_MAX) // Floor-to-ceiling height is wide enough.
		{
			int monkeyHeight = pointColl.Position.Ceiling;
			return MonkeySwingCatchData{ monkeyHeight };
		}

		return std::nullopt;
	}

	// TODO
	std::optional<ShimmyData> GetShimmy(const ItemInfo& item, const CollisionInfo& coll)
	{
		return std::nullopt;
	}
}
