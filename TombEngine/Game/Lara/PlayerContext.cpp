#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/AttractorCollision.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
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
	PlayerAttractorData::~PlayerAttractorData()
	{
		// TODO: Polymorphism to avoid global.
		if (Ptr != nullptr)
			Ptr->DetachPlayer(*LaraItem);
	}

	void PlayerAttractorData::Attach(ItemInfo& playerItem, Attractor& attrac, float chainDist,
									 const Vector3& relPosOffset, const EulerAngles& relOrientOffset,
									 const Vector3& relDeltaPos, const EulerAngles& relDeltaOrient)
	{
		Ptr = &attrac;
		ChainDistance = chainDist;
		RelPosOffset = relPosOffset;
		RelOrientOffset = relOrientOffset;
		RelDeltaPos = relDeltaPos;
		RelDeltaOrient = relDeltaOrient;

		Ptr->AttachPlayer(playerItem);
	}

	void PlayerAttractorData::Attach(ItemInfo& playerItem, Attractor& attrac, float chainDist)
	{
		Ptr = &attrac;
		ChainDistance = chainDist;
		RelPosOffset = Vector3::Zero;
		RelOrientOffset = EulerAngles::Zero;
		RelDeltaPos = Vector3::Zero;
		RelDeltaOrient = EulerAngles::Zero;

		Ptr->AttachPlayer(playerItem);
	}

	void PlayerAttractorData::Detach(ItemInfo& playerItem)
	{
		if (Ptr != nullptr)
			Ptr->DetachPlayer(playerItem);
	};

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

	bool CanPerformMonkeySwingStep(const ItemInfo& item, const CollisionInfo& coll)
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

	bool CanMonkeySwingForward(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto setup = MonkeySwingMovementSetupData
		{
			item.Pose.Orientation.y,
			MONKEY_STEPUP_HEIGHT, -MONKEY_STEPUP_HEIGHT // NOTE: Bounds defined by monkey forward state.
		};

		return TestMonkeySwingSetup(item, coll, setup);
	}

	bool CanMonkeySwingBackward(const ItemInfo& item, const CollisionInfo& coll)
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
			(int)CLICK(0.5f), -(int)CLICK(0.5f) // NOTE: Bounds defined by monkey shimmy left/right states.
		};

		return TestMonkeySwingSetup(item, coll, setup);
	}

	bool CanMonkeySwingShimmyLeft(const ItemInfo& item, const CollisionInfo& coll)
	{
		return TestMonkeyShimmy(item, coll, false);
	}

	bool CanMonkeySwingShimmyRight(const ItemInfo& item, const CollisionInfo& coll)
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

	static std::optional<AttractorCollisionData> GetEdgeClimbAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																				const ClimbSetupData& setup,
																				const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SWAMP_DEPTH_MAX				= -CLICK(3);
		constexpr auto LEDGE_FLOOR_HEIGHT_TOLERANCE = CLICK(1);

		// HACK: Offset required for proper bridge surface height detection. Floordata should be revised for proper handling.
		constexpr auto PROBE_POINT_OFFSET = Vector3(0.0f, -CLICK(1), 0.0f);
		
		const auto& player = GetLaraInfo(item);

		// 1) Test swamp depth (if applicable).
		if (setup.TestSwampDepth)
		{
			if (TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber) && player.Context.WaterSurfaceDist < SWAMP_DEPTH_MAX)
				return std::nullopt;
		}

		float range2D = OFFSET_RADIUS(coll.Setup.Radius);

		// 2) Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 2.1) Check if attractor is edge type.
			if (attracColl.AttracPtr->GetType() != AttractorType::Edge)
				continue;

			// 2.2) Test if edge is within 2D range.
			if (attracColl.Proximity.Distance2D > range2D)
				continue;

			// 2.3) Test if edge slope is illegal.
			if (abs(attracColl.SlopeAngle) >= ILLEGAL_FLOOR_SLOPE_ANGLE)
				continue;

			// 2.4) Test relation to edge intersection.
			if (setup.TestEdgeFront)
			{
				if (!attracColl.IsInFront || !attracColl.IsFacingForward ||
					!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
				{
					continue;
				}
			}
			else
			{
				if (!attracColl.IsInFront || attracColl.IsFacingForward ||
					!TestPlayerInteractAngle(item, attracColl.HeadingAngle + ANGLE(180.0f)))
				{
					continue;
				}
			}

			// TODO: Point collision probing is wrong. Won't traverse rooms correctly.
			// Potential solution. Probe from player's position and room. Combine player/intersect delta pos and RelPosOffset.
			
			// Get point collision in front of edge.
			auto probePoint = Vector3i(attracColl.Proximity.Intersection) + PROBE_POINT_OFFSET;
			auto pointCollFront = GetCollision(probePoint, attracColl.AttracPtr->GetRoomNumber(), attracColl.HeadingAngle, -coll.Setup.Radius);

			// 2.5) Test if relative edge height is within edge intersection bounds.
			int relEdgeHeight = (setup.TestEdgeFront ? attracColl.Proximity.Intersection.y : pointCollFront.Position.Floor) - item.Pose.Position.y;
			if (relEdgeHeight >= setup.LowerEdgeBound || // Player-to-edge height is within lower edge bound.
				relEdgeHeight < setup.UpperEdgeBound)	 // Player-to-edge height is within upper edge bound.
			{
				continue;
			}

			// 2.6) Test if ceiling in front is adequately higher than edge.
			int edgeToCeilHeight = pointCollFront.Position.Ceiling - attracColl.Proximity.Intersection.y;
			if (edgeToCeilHeight > setup.LowerEdgeToCeilBound)
				continue;

			// Get point collision behind edge.
			auto pointCollBack = GetCollision(probePoint, attracColl.AttracPtr->GetRoomNumber(), attracColl.HeadingAngle, coll.Setup.Radius);

			// 2.7) Test ledge floor-to-ceiling height (if applicable).
			if (setup.TestLedgeHeights)
			{
				const auto& pointCollHeights = setup.TestEdgeFront ? pointCollBack.Position : pointCollFront.Position;

				int ledgeFloorToCeilHeight = abs(pointCollHeights.Ceiling - pointCollHeights.Floor);
				if (ledgeFloorToCeilHeight <= setup.LedgeFloorToCeilHeightMin ||
					ledgeFloorToCeilHeight > setup.LedgeFloorToCeilHeightMax)
				{
					continue;
				}
			}

			// 2.8) Test for illegal slope on ledge (if applicable).
			if (setup.TestLedgeIllegalSlope)
			{
				if (setup.TestEdgeFront ? pointCollBack.Position.FloorSlope : pointCollFront.Position.FloorSlope)
					continue;
			}

			const auto& staticPointColl = setup.TestEdgeFront ? pointCollBack : pointCollFront;
			auto origin = Vector3(staticPointColl.Coordinates.x, staticPointColl.Position.Floor, staticPointColl.Coordinates.z);

			// TODO: Check.
			// 2.9) Test for static object.
			auto staticLos = GetStaticObjectLos(origin, attracColl.AttracPtr->GetRoomNumber(), -Vector3::UnitY, coll.Setup.Height, false);
			if (staticLos.has_value())
				continue;

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetStandVault2StepsUpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-STEPUP_HEIGHT, -(int)CLICK(2.5f), // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,		   // Ledge floor-to-ceil range.
			-CLICK(1),						   // Edge-to-ceil height lower bound.
			true,							   // Test edge front.
			false,							   // Test swamp depth.
			true,							   // Test ledge heights.
			true							   // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(2);

		// Get standing vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_STAND_VAULT_2_STEPS_UP;
			context.IsInFront = attracColl->IsFacingForward;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetStandVault3StepsUpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-(int)CLICK(2.5f), -(int)CLICK(3.5f), // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,			  // Ledge floor-to-ceil range.
			-CLICK(1),							  // Edge-to-ceil height lower bound.
			true,								  // Test edge front.
			false,								  // Test swamp depth.
			true,								  // Test ledge heights.
			true								  // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(3);

		// Get standing vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_STAND_VAULT_3_STEPS_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetStandVault1StepUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			0, -STEPUP_HEIGHT,				// Edge height bounds.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT, // Ledge floor-to-ceil range.
			-CLICK(1),						// Edge-to-ceil height lower bound.
			true,							// Test edge front.
			false,							// Test swamp depth.
			true,							// Test ledge heights.
			true							// Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Get standing vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_STAND_VAULT_1_STEP_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetStandVault2StepsUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-STEPUP_HEIGHT, -(int)CLICK(2.5f), // Edge height bounds.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,	   // Ledge floor-to-ceil range.
			-CLICK(1),						   // Edge-to-ceil height lower bound.
			true,							   // Test edge front.
			false,							   // Test swamp depth.
			true,							   // Test ledge heights.
			true							   // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(2);

		// Get standing vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_STAND_VAULT_2_STEPS_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetStandVault3StepsUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-(int)CLICK(2.5f), -(int)CLICK(3.5f), // Edge height bounds.
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,		  // Ledge floor-to-ceil range.
			-CLICK(1),							  // Edge-to-ceil height lower bound.
			true,								  // Test edge front.
			false,								  // Test swamp depth.
			true,								  // Test ledge heights.
			true								  // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(3);

		// Get standing vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_STAND_VAULT_3_STEPS_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetAutoJumpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto LOWER_CEIL_BOUND = -LARA_HEIGHT_MONKEY;
		constexpr auto UPPER_CEIL_BOUND = -CLICK(7);

		constexpr auto SETUP = ClimbSetupData
		{
			-(int)CLICK(3.5f), -(int)CLICK(7.5f), // Edge height bounds.
			0, -MAX_HEIGHT,						  // Ledge floor-to-ceil range.
			-(int)CLICK(1 / 256.0f),			  // Edge-to-ceil height minumum.
			true,								  // Test edge front.
			false,								  // Test swamp depth.
			false,								  // Test ledge heights.
			false								  // Test ledge illegal slope.
		};

		const auto& player = GetLaraInfo(item);

		// 1) Get auto jump to edge standing vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			int relEdgeHeight = attracColl->Proximity.Intersection.y - item.Pose.Position.y;

			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, -relEdgeHeight, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_AUTO_JUMP;
			context.IsInFront = attracColl->IsFacingForward;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.SetBusyHands = false;
			context.SetJumpVelocity = true;

			return context;
		}

		// Auto jump to monkey swing disabled; return early.
		if (!g_GameFlow->HasMonkeyAutoJump())
			return std::nullopt;

		// Get point collision.
		auto pointColl = GetCollision(item);
		int relCeilHeight = pointColl.Position.Ceiling - item.Pose.Position.y;

		// 2) Get auto jump to monkey swing standing vault climb context.
		if (player.Control.CanMonkeySwing &&	// Player is standing below monkey swing.
			relCeilHeight < LOWER_CEIL_BOUND && // Ceiling height is within lower ceiling bound.
			relCeilHeight >= UPPER_CEIL_BOUND)	// Ceiling height is within upper ceiling bound.
		{
			auto context = ClimbContextData{};
			context.AttracPtr = nullptr;
			context.ChainDistance = 0.0f;
			context.RelPosOffset = Vector3(0.0f, -relCeilHeight, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_AUTO_JUMP;
			context.AlignType = ClimbContextAlignType::None;
			context.IsInFront = true;
			context.SetBusyHands = false;
			context.SetJumpVelocity = true;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetStandingClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(2);

		const auto& player = GetLaraInfo(item);

		// Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item, 0.0f, 0.0f, 0.0f, ATTRAC_DETECT_RADIUS);

		auto context = std::optional<ClimbContextData>();

		// 1) Stand vault 1 step up to crouch.
		context = GetStandVault1StepUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Stand vault 2 steps up to crouch.
		context = GetStandVault2StepsUpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 3) Stand vault 2 steps up.
		context = GetStandVault2StepsUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 4) Stand vault 3 steps up to crouch.
		context = GetStandVault3StepsUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 5) Stand vault 3 steps up.
		context = GetStandVault3StepsUpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 6) Vault auto jump.
		context = GetAutoJumpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// TODO: Move ladder checks here when ladders are less prone to breaking.
		// In this case, they fail due to a reliance on ShiftItem(). -- Sezz 2021.02.05

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVault1StepDownClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			STEPUP_HEIGHT, CRAWL_STEPUP_HEIGHT, // Edge height bounds.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,		// Ledge floor-to-ceil range.
			-(int)CLICK(0.6f),					// Edge-to-ceil height lower bound.
			false,								// Test edge front.
			false,								// Test swamp depth.
			true,								// Test ledge heights.
			true								// Test ledge illegal slope.
		};

		// Get crawling vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.TargetStateID = LS_CRAWL_VAULT_1_STEP_DOWN;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVault1StepDownToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			STEPUP_HEIGHT, -CRAWL_STEPUP_HEIGHT, // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,			 // Ledge floor-to-ceil range.
			-(int)CLICK(1.25f),					 // Edge-to-ceil height lower bound.
			false,								 // Test edge front.
			false,								 // Test swamp depth.
			true,								 // Test ledge heights.
			false								 // Test ledge illegal slope.
		};

		// Get crawling vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.TargetStateID = LS_CRAWL_VAULT_1_STEP_DOWN_TO_STAND;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetCrawlVault1StepUpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																			const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-CRAWL_STEPUP_HEIGHT, -STEPUP_HEIGHT, // Edge height bounds.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,		  // Ledge floor-to-ceil range.
			-(int)CLICK(0.6f),					  // Edge-to-ceil height lower bound.
			true,								  // Test edge front.
			false,								  // Test swamp depth.
			true,								  // Test ledge heights.
			true								  // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Get crawling vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())

		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_CRAWL_VAULT_1_STEP_UP;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetCrawlVaultJumpClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																		 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			NO_LOWER_BOUND, STEPUP_HEIGHT, // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,	   // Ledge floor-to-ceil range.
			-(int)CLICK(1.25f),			   // Edge-to-ceil height lower bound.
			false,						   // Test edge front.
			false,						   // Test swamp depth.
			true,						   // Test ledge heights.
			false						   // Test ledge illegal slope.
		};

		// 1) Get edge crawl vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles(0, ANGLE(180.0f), 0);
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.TargetStateID = IsHeld(In::Walk) ? LS_CRAWL_VAULT_JUMP_FLIP : LS_CRAWL_VAULT_JUMP;
			context.AlignType = ClimbContextAlignType::OffsetBlend;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		auto pointCollCenter = GetCollision(item);
		auto pointCollFront = GetCollision(&item, item.Pose.Orientation.y, BLOCK(0.25f), -coll.Setup.Height);
		int relFloorHeight = pointCollFront.Position.Floor - item.Pose.Position.y;

		// TODO
		// 2) Get illegal slope crawl vault climb context (special case).
		if (pointCollFront.Position.FloorSlope &&
			true
			/*relFloorHeight <= SETUP.LowerEdgeBound &&							// Within lower floor bound.
			relFloorHeight >= SETUP.UpperEdgeBound &&							// Within upper floor bound.

			abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor) > testSetup.ClampMin &&		// Crossing clamp limit.
			abs(pointCollCenter.Position.Ceiling - probeB.Position.Floor) > testSetup.ClampMin &&		// Destination clamp limit.
			abs(probeMiddle.Position.Ceiling - pointCollFront.Position.Floor) >= testSetup.GapMin &&	// Gap is optically permissive (going up).
			abs(probeA.Position.Ceiling - probeMiddle.Position.Floor) >= testSetup.GapMin &&	// Gap is optically permissive (going down).
			abs(probeA.Position.Floor - probeB.Position.Floor) <= testSetup.FloorBound &&		// Crossing/destination floor height difference suggests continuous crawl surface.
			(probeA.Position.Ceiling - y) < -testSetup.GapMin*/)									// Ceiling height is permissive.
		{
			auto context = ClimbContextData{};
			context.AttracPtr = nullptr;
			context.ChainDistance = 0.0f;
			context.RelPosOffset = Vector3::Zero;
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_CRAWL_VAULT_JUMP;
			context.AlignType = ClimbContextAlignType::None;
			context.IsInFront = true;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	std::optional<ClimbContextData> GetCrawlClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);

		const auto& player = GetLaraInfo(item);

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item, 0.0f, 0.0f, 0.0f, ATTRAC_DETECT_RADIUS);

		auto context = std::optional<ClimbContextData>();

		// 1) Crawl vault down 1 step to stand.
		context = GetCrawlVault1StepDownToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (!IsHeld(In::Crouch) && HasStateDispatch(&item, context->TargetStateID))
				return context;

			// 2) Crawl vault down 1 step.
			context = GetCrawlVault1StepDownClimbContext(item, coll, attracColls);
			if (context.has_value())
			{
				if (HasStateDispatch(&item, context->TargetStateID))
					return context;
			}
		}

		// 3) Crawl vault down 1 step.
		context = GetCrawlVault1StepDownClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 4) Crawl vault jump.
		context = GetCrawlVaultJumpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 5) Crawl vault up 1 step.
		context = GetCrawlVault1StepUpClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepDownToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			STEPUP_HEIGHT, (int)CLICK(0.5f), // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,		 // Ledge floor-to-ceil range.
			-CLICK(1),						 // Edge-to-ceil height lower bound.
			true,							 // Test edge front.
			false,							 // Test swamp depth.
			true,							 // Test ledge heights.
			true							 // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = -CLICK(1);

		// Crouch action held and extended crawl moveset enabled; return nullopt.
		if (IsHeld(In::Crouch) && g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_DOWN_TO_STAND;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetTreadWaterVault0StepsToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																					   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			(int)CLICK(0.5f), -(int)CLICK(0.5f), // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,			 // Ledge floor-to-ceil range.
			-CLICK(1),							 // Edge-to-ceil height lower bound.
			true,								 // Test edge front.
			false,								 // Test swamp depth.
			true,								 // Test ledge heights.
			true								 // Test ledge illegal slope.
		};

		// Crouch action held and extended crawl moveset enabled; return nullopt.
		if (IsHeld(In::Crouch) && g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_TREAD_WATER_VAULT_0_STEPS_TO_STAND;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}
	
	static std::optional<ClimbContextData> GetTreadWaterVault1StepUpToStandClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-(int)CLICK(0.5f), -STEPUP_HEIGHT, // Edge height bounds.
			LARA_HEIGHT, -MAX_HEIGHT,		   // Ledge floor-to-ceil range.
			-CLICK(1),						   // Edge-to-ceil height lower bound.
			true,							   // Test edge front.
			false,							   // Test swamp depth.
			true,							   // Test ledge heights.
			true							   // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Crouch action held and extended crawl moveset enabled; return nullopt.
		if (IsHeld(In::Crouch) && g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_UP_TO_STAND;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepDownToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						   const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			STEPUP_HEIGHT, (int)CLICK(0.5f), // Edge height bounds.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,	 // Ledge floor-to-ceil range.
			-(int)CLICK(0.6f),				 // Edge-to-ceil height lower bound.
			true,							 // Test edge front.
			false,							 // Test swamp depth.
			true,							 // Test ledge heights.
			true							 // Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = -CLICK(1);

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_DOWN_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault0StepsToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			(int)CLICK(0.5f), -(int)CLICK(0.5f), // Edge height bounds.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT,		 // Ledge floor-to-ceil range.
			-(int)CLICK(0.6f),					 // Edge-to-ceil height lower bound.
			true,								 // Test edge front.
			false,								 // Test swamp depth.
			true,								 // Test ledge heights.
			true								 // Test ledge illegal slope.
		};

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_TREAD_WATER_VAULT_0_STEPS_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetTreadWaterVault1StepUpToCrouchClimbContext(const ItemInfo& item, const CollisionInfo& coll,
																						 const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-CLICK(1), -STEPUP_HEIGHT,		// Edge height bounds.
			LARA_HEIGHT_CRAWL, -MAX_HEIGHT, // Ledge floor-to-ceil range.
			-(int)CLICK(0.6f),				// Edge-to-ceil height lower bound.
			true,							// Test edge front.
			false,							// Test swamp depth.
			true,							// Test ledge heights.
			true							// Test ledge illegal slope.
		};
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Extended crawl moveset disabled; return nullopt.
		if (!g_GameFlow->HasCrawlExtended())
			return std::nullopt;

		// Get tread water vault climb context.
		auto attracColl = GetEdgeClimbAttractorCollision(item, coll, SETUP, attracColls);
		if (attracColl.has_value())
		{
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl->AttracPtr;
			context.ChainDistance = attracColl->Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_TREAD_WATER_VAULT_1_STEP_UP_TO_CROUCH;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl->IsFacingForward;
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		return std::nullopt;
	}

	// TODO: Refactor.
	bool TestPlayerWaterStepOut(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Get point collision.
		auto pointColl = GetCollision(item);
		int vPos = item->Pose.Position.y;

		if (coll->CollisionType == CT_FRONT ||
			pointColl.Position.FloorSlope ||
			(pointColl.Position.Floor - vPos) <= 0)
		{
			return false;
		}

		if ((pointColl.Position.Floor - vPos) >= -CLICK(0.5f))
		{
			SetAnimation(item, LA_STAND_IDLE);
		}
		else
		{
			SetAnimation(item, LA_ONWATER_TO_WADE_1_STEP);
			item->Animation.TargetState = LS_IDLE;
		}

		item->Pose.Position.y = pointColl.Position.Floor;
		UpdateLaraRoom(item, -(STEPUP_HEIGHT - 3));

		ResetPlayerLean(item);
		item->Animation.Velocity = Vector3::Zero;
		item->Animation.IsAirborne = false;
		player.Control.WaterStatus = WaterStatus::Wade;

		return true;
	}

	std::optional<ClimbContextData> GetTreadWaterClimbContext(ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);

		const auto& player = GetLaraInfo(item);

		// Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return std::nullopt;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item, 0.0f, 0.0f, 0.0f, ATTRAC_DETECT_RADIUS);

		auto context = std::optional<ClimbContextData>();

		// 1) Water tread vault 1 step down to stand.
		context = GetTreadWaterVault1StepDownToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 2) Water tread vault 1 step down to crouch.
		context = GetTreadWaterVault1StepDownToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 3) Water tread vault 0 steps to stand.
		context = GetTreadWaterVault0StepsToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 4) Water tread vault 0 steps to crouch.
		context = GetTreadWaterVault0StepsToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 5) Water tread vault up 1 step up to stand.
		context = GetTreadWaterVault1StepUpToStandClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		// 6) Water tread vault up 1 step up to crouch.
		context = GetTreadWaterVault1StepUpToCrouchClimbContext(item, coll, attracColls);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}
		
		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	// TODO
	std::optional<ClimbContextData> GetSafeEdgeDescentClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{

		// Create and return crawl to hang vault context.
		auto context = ClimbContextData{};
		//context.AttracPtr = attracColl.AttracPtr;
		//context.ChainDistance = attracColl.Proximity.ChainDistance;
		context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
		context.RelOrientOffset = EulerAngles::Zero;
		context.TargetStateID = LS_CRAWL_TO_HANG;
		context.AlignType = ClimbContextAlignType::AttractorParent;
		//context.IsInFront = attracColl.IsFacingForward; // TODO: Check.
		context.SetBusyHands = true;
		context.SetJumpVelocity = false;

		return context;
	}

	std::optional<ClimbContextData> GetCrawlEdgeDescentClimbContext(ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto SETUP = ClimbSetupData
		{
			-MAX_HEIGHT, LARA_HEIGHT_STRETCH, // Edge height bounds.
			0, -MAX_HEIGHT,					  // Ledge floor-to-ceil range (irrelevant).
			-(int)CLICK(0.6f),				  // Edge-to-ceil height lower bound.
			false,							  // Test edge front (irrelevant).
			false,							  // Test swamp depth (irrelevant).
			false,							  // Test ledge heights (irrelevant).
			false							  // Test ledge illegal slope (irrelevant).
		};
		constexpr auto ATTRAC_DETECT_RADIUS = BLOCK(0.5f);

		const auto& player = GetLaraInfo(item);
		
		// TODO: Better way. Use GetStaticObjectLos().
		// 1) Test for object collision.
		bool isObjectCollided = TestLaraObjectCollision(&item, item.Pose.Orientation.y + ANGLE(180.0f), CLICK(1.2f), -LARA_HEIGHT_CRAWL);
		if (isObjectCollided)
			return std::nullopt;

		float range2D = OFFSET_RADIUS(coll.Setup.Radius);

		// 2) Assess attractor collision.
		auto attracColls = GetAttractorCollisions(item, 0.0f, 0.0f, 0.0f, ATTRAC_DETECT_RADIUS);
		for (const auto& attracColl : attracColls)
		{
			// 2.1) Check if attractor is edge type.
			if (attracColl.AttracPtr->GetType() != AttractorType::Edge)
				continue;

			// 2.2) Test if edge is within 2D range.
			if (attracColl.Proximity.Distance2D > range2D)
				continue;

			// 2.3) Test if edge slope is illegal.
			if (abs(attracColl.SlopeAngle) >= ILLEGAL_FLOOR_SLOPE_ANGLE)
				continue;

			// TODO
			// 2.4) Test relation to edge intersection.
			if (/*!attracColl.IsInFront || attracColl.IsFacingForward ||
				*/!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
			{
				continue;
			}

			// Get point collision in front of edge.
			auto pointCollFront = GetCollision(
				attracColl.Proximity.Intersection, attracColl.AttracPtr->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);
			
			// TODO
			// 2.5) Test if relative edge height is within edge intersection bounds.
			auto relEdgeHeight = attracColl.Proximity.Intersection.y - pointCollFront.Position.Floor;
			if (relEdgeHeight >= SETUP.LowerEdgeBound || // Floor-to-edge height is within lower edge bound.
				relEdgeHeight < SETUP.UpperEdgeBound)	 // Floor-to-edge height is within upper edge bound.
			{
				//continue;
			}

			// 2.6) Test if ceiling in front is adequately higher than edge.
			int edgeToCeilHeight = pointCollFront.Position.Ceiling - attracColl.Proximity.Intersection.y;
			if (edgeToCeilHeight > SETUP.LowerEdgeToCeilBound)
				continue;

			// Create and return crawl to hang vault context.
			auto context = ClimbContextData{};
			context.AttracPtr = attracColl.AttracPtr;
			context.ChainDistance = attracColl.Proximity.ChainDistance;
			context.RelPosOffset = Vector3(0.0f, 0.0f, coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Zero;
			context.TargetStateID = LS_CRAWL_TO_HANG;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsInFront = attracColl.IsFacingForward; // TODO: Check.
			context.SetBusyHands = true;
			context.SetJumpVelocity = false;

			return context;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<AttractorCollisionData> GetEdgeCatchAttractorCollision(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_PROBE_FORWARD		= BLOCK(1 / (float)BLOCK(1));
		constexpr auto ATTRAC_DETECT_RADIUS		= BLOCK(0.5f);
		constexpr auto FLOOR_TO_EDGE_HEIGHT_MIN = LARA_HEIGHT_STRETCH;

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item, 0.0f, -coll.Setup.Height, 0.0f, ATTRAC_DETECT_RADIUS);

		float range2D = OFFSET_RADIUS(std::max((float)coll.Setup.Radius, item.Animation.Velocity.Length()));

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check if attractor is edge type.
			if (attracColl.AttracPtr->GetType() != AttractorType::Edge)
				continue;

			// 2) Test if edge is within 2D range.
			if (attracColl.Proximity.Distance2D > range2D)
				continue;

			// 3) Test if edge slope is illegal.
			if (abs(attracColl.SlopeAngle) >= ILLEGAL_FLOOR_SLOPE_ANGLE)
				continue;

			// 4) Test relation to edge intersection.
			if (!attracColl.IsInFront || !attracColl.IsFacingForward ||
				!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
			{
				continue;
			}

			// Get point collision in front of edge.
			auto probePoint = Vector3i(attracColl.Proximity.Intersection.x, attracColl.Proximity.Intersection.y - CLICK(1), attracColl.Proximity.Intersection.z);
			auto pointColl = GetCollision(
				probePoint, attracColl.AttracPtr->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// 5) Test if edge is high enough from floor.
			int floorToEdgeHeight = pointColl.Position.Floor - attracColl.Proximity.Intersection.y;
			if (floorToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN)
				continue;

			// 6) Test if ceiling in front is adequately higher than edge.
			int edgeToCeilHeight = pointColl.Position.Ceiling - attracColl.Proximity.Intersection.y;
			if (edgeToCeilHeight >= 0)
				continue;

			int vPos = item.Pose.Position.y - coll.Setup.Height;
			int relEdgeHeight = attracColl.Proximity.Intersection.y - vPos;

			bool isFalling = (item.Animation.Velocity.y >= 0.0f);
			int lowerBound = isFalling ? (int)round(item.Animation.Velocity.y) : 0;
			int upperBound = isFalling ? 0 : (int)round(item.Animation.Velocity.y);

			// 7) Test catch trajectory.
			if (relEdgeHeight <= lowerBound && // Edge height is above lower height bound.
				relEdgeHeight >= upperBound)   // Edge height is below upper height bound.
			{
				return attracColl;
			}
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	static std::optional<EdgeCatchContextData> GetAttractorEdgeCatch(const ItemInfo& item, const CollisionInfo& coll)
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
		return EdgeCatchContextData
		{
			attracColl->AttracPtr,
			EdgeType::Attractor,
			attracColl->Proximity.Intersection,
			attracColl->Proximity.ChainDistance,
			headingAngle
		};
	}

	static std::optional<EdgeCatchContextData> GetClimbableWallEdgeCatch(ItemInfo& item, CollisionInfo& coll)
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
			return EdgeCatchContextData{ nullptr, EdgeType::ClimbableWall, offset };
		}

		return std::nullopt;
	}

	std::optional<EdgeCatchContextData> GetEdgeCatchContext(ItemInfo& item, CollisionInfo& coll)
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

	std::optional<MonkeySwingCatchContextData> GetMonkeySwingCatchContext(const ItemInfo& item, const CollisionInfo& coll)
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
			return MonkeySwingCatchContextData{ monkeyHeight };
		}

		return std::nullopt;
	}
}
