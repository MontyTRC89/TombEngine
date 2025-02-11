#include "framework.h"
#include "Game/Lara/Context/GroundMovement.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/configuration.h"
#include "Specific/Input/Input.h"

using namespace TEN::Collision::Point;
using namespace TEN::Input;

namespace TEN::Player
{
	bool CanChangeElevation(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_FLOOR_BOUND = STEPUP_HEIGHT;
		constexpr auto UPPER_FLOOR_BOUND = -STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetPointCollision(item, 0, 0, -coll.Setup.Height / 2); // NOTE: Height offset required for correct bridge collision.
		int relFloorHeight = pointColl.GetFloorHeight() - item.Pose.Position.y;

		// 1) Test if player is already aligned with floor.
		if (relFloorHeight == 0)
			return false;

		// 2) Assess point collision and player status.
		if ((relFloorHeight <= LOWER_FLOOR_BOUND ||				// Floor height is above lower floor bound.
			player.Control.WaterStatus == WaterStatus::Wade) && // OR is wading.
			relFloorHeight >= UPPER_FLOOR_BOUND)				// Floor height is below upper floor bound.
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
		auto pointColl = GetPointCollision(item, 0, 0, -coll.Setup.Height / 2); // NOTE: Height offset required for correct bridge collision.
		int relFloorHeight = pointColl.GetFloorHeight() - item.Pose.Position.y;

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
		if (!g_GameFlow->GetSettings()->Animations.PoseTimeout)
			return false;

		// 2) Test player hand and water status.
		if (player.Control.HandStatus != HandStatus::Free ||
			player.Control.WaterStatus == WaterStatus::Wade)
		{
			return false;
		}

		// 3) Assess player status.
		if (!(IsHeld(In::Flare) || IsHeld(In::Draw)) &&				   // Avoid unsightly concurrent actions.
			(player.Control.Weapon.GunType != LaraWeaponType::Flare || // Not handling flare.
				player.Flare.Life) &&								   // OR flare is still active.
			player.Context.Vehicle == NO_VALUE)						   // Not in a vehicle.
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

	static bool TestBasicMovementSetup(const ItemInfo& item, const CollisionInfo& coll, const BasicMovementSetupData& setup, bool isCrawling = false)
	{
		constexpr auto SLOPE_ASPECT_ANGLE_DELTA_MAX = ANGLE(90.0f);

		// HACK: coll.Setup.Radius and coll.Setup.Height are set only in lara_col functions and then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any context assessment functions called in crouch/crawl lara_as routines.
		// If states become objects, a dedicated state init function should eliminate the need for the isCrawling parameter. -- Sezz 2022.03.16
		int playerRadius = isCrawling ? LARA_RADIUS_CRAWL : coll.Setup.Radius;
		int playerHeight = isCrawling ? LARA_HEIGHT_CRAWL : coll.Setup.Height;

		// Get point collision.
		auto pointColl = GetPointCollision(item, setup.HeadingAngle, OFFSET_RADIUS(playerRadius), -playerHeight);
		int vPos = item.Pose.Position.y;
		int vPosTop = vPos - playerHeight;

		// Calculate slope aspect delta angle.
		short aspectAngle = Geometry::GetSurfaceAspectAngle(pointColl.GetFloorNormal());
		short aspectAngleDelta = Geometry::GetShortestAngle(setup.HeadingAngle, aspectAngle);

		// 1) Check for steep floor below floor (if applicable).
		if (setup.TestSteepFloorBelow &&
			(pointColl.IsSteepFloor() && abs(aspectAngleDelta) <= SLOPE_ASPECT_ANGLE_DELTA_MAX))
		{
			return false;
		}

		// 1) Check for steep floor above floor (if applicable).
		if (setup.TestSteepFloorAbove &&
			(pointColl.IsSteepFloor() && abs(aspectAngleDelta) >= SLOPE_ASPECT_ANGLE_DELTA_MAX))
		{
			return false;
		}

		// 3) Check for death floor (if applicable).
		if (setup.TestDeathFloor && pointColl.GetSector().Flags.Death && pointColl.GetFloorBridgeItemNumber() == NO_VALUE)
			return false;

		// LOS setup at upper floor bound.
		auto origin0 = GameVector(
			item.Pose.Position.x,
			(vPos + setup.UpperFloorBound) - 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target0 = GameVector(
			pointColl.GetPosition().x,
			(vPos + setup.UpperFloorBound) - 1,
			pointColl.GetPosition().z,
			item.RoomNumber);

		// LOS setup at lowest ceiling bound (player height).
		auto origin1 = GameVector(
			item.Pose.Position.x,
			vPosTop + 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target1 = GameVector(
			pointColl.GetPosition().x,
			vPosTop + 1,
			pointColl.GetPosition().z,
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

		int relFloorHeight = pointColl.GetFloorHeight() - vPos;
		int relCeilHeight = pointColl.GetCeilingHeight() - vPos;
		int floorToCeilHeight = abs(pointColl.GetCeilingHeight() - pointColl.GetFloorHeight());

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
			!player.Control.IsRunJumpQueued) // NOTE: Queued running jump blocks 180 roll.
		{
			return true;
		}

		return false;
	}

	bool CanRunForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = BasicMovementSetupData
		{
			item.Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by run forward state.
			false, true, false
		};

		return TestBasicMovementSetup(item, coll, setup);
	}

	bool CanRunBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = BasicMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by run backward state.
			false, false, false
		};

		return TestBasicMovementSetup(item, coll, setup);
	}

	bool CanWalkForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = BasicMovementSetupData
		{
			item.Pose.Orientation.y,
			STEPUP_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by walk forward state.
		};

		return TestBasicMovementSetup(item, coll, setup);
	}

	bool CanWalkBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = BasicMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			STEPUP_HEIGHT, -STEPUP_HEIGHT // NOTE: Bounds defined by walk backward state.
		};

		return TestBasicMovementSetup(item, coll, setup);
	}

	static bool TestSidestep(const ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		const auto& player = GetLaraInfo(item);

		auto setup = BasicMovementSetupData{};

		// Wade case.
		if (player.Control.WaterStatus == WaterStatus::Wade)
		{
			setup = BasicMovementSetupData
			{
				short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				-MAX_HEIGHT, (int)-CLICK(1.25f), // NOTE: Upper bound defined by sidestep left/right states.
				false, false, false
			};
		}
		// Regular case.
		else
		{
			setup = BasicMovementSetupData
			{
				short(item.Pose.Orientation.y + (isGoingRight ? ANGLE(90.0f) : ANGLE(-90.0f))),
				(int)CLICK(0.8f), (int)-CLICK(0.8f) // NOTE: Bounds defined by sidestep left/right states.
			};
		}

		return TestBasicMovementSetup(item, coll, setup);
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

		auto setup = BasicMovementSetupData
		{
			item.Pose.Orientation.y,
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by wade forward state.
			false, false, false
		};

		// 2) Assess context.
		return TestBasicMovementSetup(item, coll, setup);
	}

	bool CanWadeBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus != WaterStatus::Wade)
			return false;

		auto setup = BasicMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			-MAX_HEIGHT, -STEPUP_HEIGHT, // NOTE: Bounds defined by walk backward state.
			false, false, false
		};

		// 2) Assess context.
		return TestBasicMovementSetup(item, coll, setup);
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
		auto pointCollCenter = GetPointCollision(item, 0, 0.0f, -LARA_HEIGHT / 2);
		int floorToCeilHeightCenter = abs(pointCollCenter.GetCeilingHeight() - pointCollCenter.GetFloorHeight());

		// Assess center point collision.
		if (floorToCeilHeightCenter < LARA_HEIGHT ||					// Floor-to-ceiling height isn't too wide.
			abs(coll.Middle.Ceiling - LARA_HEIGHT_CRAWL) < LARA_HEIGHT) // Consider statics overhead detected by GetCollisionInfo().
		{
			return true;
		}

		// TODO: Check whether < or <= and > or >=.

		// Get front point collision.
		auto pointCollFront = GetPointCollision(item, item.Pose.Orientation.y, radius, -coll.Setup.Height);
		int floorToCeilHeightFront = abs(pointCollFront.GetCeilingHeight() - pointCollFront.GetFloorHeight());
		int relFloorHeightFront = abs(pointCollFront.GetFloorHeight() - pointCollCenter.GetFloorHeight());

		// Assess front point collision.
		if (relFloorHeightFront <= CRAWL_STEPUP_HEIGHT && // Floor is within upper/lower floor bounds.
			floorToCeilHeightFront < LARA_HEIGHT &&		  // Floor-to-ceiling height isn't too wide.
			floorToCeilHeightFront > LARA_HEIGHT_CRAWL)	  // Floor-to-ceiling height isn't too narrow.
		{
			return true;
		}

		// Get back point collision.
		auto pointCollBack = GetPointCollision(item, item.Pose.Orientation.y, -radius, -coll.Setup.Height);
		int floorToCeilHeightBack = abs(pointCollBack.GetCeilingHeight() - pointCollBack.GetFloorHeight());
		int relFloorHeightBack = abs(pointCollBack.GetFloorHeight() - pointCollCenter.GetFloorHeight());

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
		if (player.Control.WaterStatus != WaterStatus::Wade && // Player is wading.
			!((player.Control.HandStatus == HandStatus::WeaponReady ||
				player.Control.HandStatus == HandStatus::WeaponDraw ||
				player.Control.HandStatus == HandStatus::WeaponUndraw) &&
				IsStandingWeapon(&item, player.Control.Weapon.GunType))) // OR player is wielding a non-standing weapon.
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
		if (!g_GameFlow->GetSettings()->Animations.CrouchRoll)
			return false;

		// 2) Test water depth.
		if (player.Context.WaterSurfaceDist < WATER_HEIGHT_MAX)
			return false;

		// TODO: Extend point collision struct to also find water depths.
		float dist = 0.0f;
		auto pointColl0 = GetPointCollision(item);

		// 3) Test continuity of path.
		while (dist < PROBE_DIST_MAX)
		{
			// Get point collision.
			dist += STEP_DIST;
			auto pointColl1 = GetPointCollision(item, item.Pose.Orientation.y, dist, -LARA_HEIGHT_CRAWL);

			int floorHeightDelta = abs(pointColl0.GetFloorHeight() - pointColl1.GetFloorHeight());
			int floorToCeilHeight = abs(pointColl1.GetCeilingHeight() - pointColl1.GetFloorHeight());

			// Assess point collision.
			if (floorHeightDelta > FLOOR_BOUND ||				 // Avoid floor height delta beyond crawl stepup threshold.
				floorToCeilHeight <= FLOOR_TO_CEIL_HEIGHT_MAX || // Avoid narrow spaces.
				pointColl1.IsSteepFloor())					 // Avoid slippery floor slopes.
			{
				return false;
			}

			pointColl0 = std::move(pointColl1);
		}

		return true;
	}

	bool CanCrawlForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = BasicMovementSetupData
		{
			item.Pose.Orientation.y,
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Bounds defined by crawl forward state.
		};

		return TestBasicMovementSetup(item, coll, setup, true);
	}

	bool CanCrawlBackward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = BasicMovementSetupData
		{
			short(item.Pose.Orientation.y + ANGLE(180.0f)),
			CRAWL_STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT // NOTE: Bounds defined by crawl backward state.
		};

		return TestBasicMovementSetup(item, coll, setup, true);
	}
}
