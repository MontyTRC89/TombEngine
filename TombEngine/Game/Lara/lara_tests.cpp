#include "framework.h"
#include "Game/Lara/lara_tests.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/PlayerContext.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_monkey.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Entities::Player;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Utils;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool TestPlayerInteractAngle(const ItemInfo& item, short testAngle)
{
	return (abs(Geometry::GetShortestAngle(item.Pose.Orientation.y, testAngle)) <= PLAYER_INTERACT_ANGLE_CONSTRAINT);
}

bool TestLaraClimbIdle(ItemInfo* item, CollisionInfo* coll)
{
	int shiftRight = 0;
	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -700, CLICK(2), &shiftRight) != 1)
		return false;

	int shiftLeft = 0;
	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -700, CLICK(2), &shiftLeft) != 1)
		return false;

	if (shiftRight)
	{
		if (shiftLeft)
		{
			if (shiftRight < 0 != shiftLeft < 0)
				return false;

			if ((shiftRight < 0 && shiftLeft < shiftRight) ||
				(shiftRight > 0 && shiftLeft > shiftRight))
			{
				item->Pose.Position.y += shiftLeft;
				return true;
			}
		}

		item->Pose.Position.y += shiftRight;
	}
	else if (shiftLeft)
	{
		item->Pose.Position.y += shiftLeft;
	}

	return true;
}

bool TestLaraNearClimbableWall(ItemInfo* item, FloorInfo* floor)
{
	if (floor == nullptr)
		floor = GetCollision(item).BottomBlock;

	return ((256 << (GetQuadrant(item->Pose.Orientation.y))) & GetClimbFlags(floor));
}

bool TestLaraHangOnClimbableWall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);
	int shift, result;

	if (!lara->Control.CanClimbLadder)
		return false;

	if (item->Animation.Velocity.y < 0)
		return false;

	// HACK: Climb wall tests are highly fragile and depend on quadrant shifts.
	// Until climb wall tests are fully refactored, we need to recalculate CollisionInfo.

	auto coll2 = *coll;
	coll2.Setup.Mode = CollisionProbeMode::Quadrants;
	GetCollisionInfo(&coll2, item);

	switch (GetQuadrant(item->Pose.Orientation.y))
	{
	case NORTH:
	case SOUTH:
		item->Pose.Position.z += coll2.Shift.Position.z;
		break;

	case EAST:
	case WEST:
		item->Pose.Position.x += coll2.Shift.Position.x;
		break;

	default:
		break;
	}

	auto bounds = GameBoundingBox(item);

	if (lara->Control.MoveAngle != item->Pose.Orientation.y)
	{
		int l = LaraCeilingFront(item, item->Pose.Orientation.y, 0, 0);
		int r = LaraCeilingFront(item, lara->Control.MoveAngle, CLICK(0.5f), 0);

		if (abs(l - r) > SLOPE_DIFFERENCE)
			return false;
	}

	if (LaraTestClimbPos(item, LARA_RADIUS, LARA_RADIUS, bounds.Y1, bounds.GetHeight(), &shift) &&
		LaraTestClimbPos(item, LARA_RADIUS, -LARA_RADIUS, bounds.Y1, bounds.GetHeight(), &shift))
	{
		result = LaraTestClimbPos(item, LARA_RADIUS, 0, bounds.Y1, bounds.GetHeight(), &shift);
		if (result)
		{
			if (result != 1)
				item->Pose.Position.y += shift;

			return true;
		}
	}

	return false;
}

bool TestLaraWall(const ItemInfo* item, float dist, float height)
{
	auto origin = GameVector(
		Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, 0.0f, height),
		item->RoomNumber);
	auto target = GameVector(
		Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, dist, height),
		item->RoomNumber);

	return !LOS(&origin, &target);
}

bool TestLaraFacingCorner(const ItemInfo* item, short headingAngle, float dist)
{
	short angleLeft = headingAngle - ANGLE(15.0f);
	short angleRight = headingAngle + ANGLE(15.0f);

	auto origin = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z,
		item->RoomNumber);

	auto target0 = GameVector(
		item->Pose.Position.x + dist * phd_sin(angleLeft),
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z + dist * phd_cos(angleLeft),
		item->RoomNumber);

	auto target1 = GameVector(
		item->Pose.Position.x + dist * phd_sin(angleRight),
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z + dist * phd_cos(angleRight),
		item->RoomNumber);

	bool result0 = LOS(&origin, &target0);
	bool result1 = LOS(&origin, &target1);
	return (!result0 && !result1);
}

bool LaraPositionOnLOS(ItemInfo* item, short angle, int distance)
{
	auto origin0 = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - LARA_HEADROOM,
		item->Pose.Position.z,
		item->RoomNumber);
	auto target0 = GameVector(
		item->Pose.Position.x + distance * phd_sin(angle),
		item->Pose.Position.y - LARA_HEADROOM,
		item->Pose.Position.z + distance * phd_cos(angle),
		item->RoomNumber);

	auto origin2 = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - LARA_HEIGHT + LARA_HEADROOM,
		item->Pose.Position.z,
		item->RoomNumber);
	auto target1 = GameVector(
		item->Pose.Position.x + distance * phd_sin(angle),
		item->Pose.Position.y - LARA_HEIGHT + LARA_HEADROOM,
		item->Pose.Position.z + distance * phd_cos(angle),
		item->RoomNumber);

	bool result0 = LOS(&origin0, &target0);
	bool result1 = LOS(&origin2, &target1);
	return (result0 && result1);
}

int LaraFloorFront(ItemInfo* item, short angle, int distance)
{
	return LaraCollisionFront(item, angle, distance).Position.Floor;
}

int LaraCeilingFront(ItemInfo* item, short angle, int distance, int height)
{
	return LaraCeilingCollisionFront(item, angle, distance, height).Position.Ceiling;
}

CollisionResult LaraCollisionFront(ItemInfo* item, short angle, int distance)
{
	auto probe = GetCollision(item, angle, distance, -LARA_HEIGHT);

	if (probe.Position.Floor != NO_HEIGHT)
		probe.Position.Floor -= item->Pose.Position.y;

	return probe;
}

CollisionResult LaraCeilingCollisionFront(ItemInfo* item, short angle, int distance, int height)
{
	auto probe = GetCollision(item, angle, distance, -height);

	if (probe.Position.Ceiling != NO_HEIGHT)
		probe.Position.Ceiling += height - item->Pose.Position.y;

	return probe;
}

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
	item->Animation.Velocity.y = 0.0f;
	item->Animation.Velocity.z = 0.0f;
	item->Animation.IsAirborne = false;
	player.Control.WaterStatus = WaterStatus::Wade;

	return true;
}

bool TestLaraWaterClimbOut(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (coll->CollisionType != CT_FRONT || !IsHeld(In::Action))
		return false;

	if (lara->Control.HandStatus != HandStatus::Free &&
		(lara->Control.HandStatus != HandStatus::WeaponReady || lara->Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return false;
	}

	if (coll->Middle.Ceiling > -STEPUP_HEIGHT)
		return false;

	int frontFloor = coll->Front.Floor + LARA_HEIGHT_TREAD;
	if (coll->Front.Bridge == NO_ITEM &&
		(frontFloor <= -CLICK(2) ||
		frontFloor > CLICK(1.25f) - 4))
	{
		return false;
	}

	// Extra bridge check.
	if (coll->Front.Bridge != NO_ITEM)
	{
		int bridgeBorder = GetBridgeBorder(g_Level.Items[coll->Front.Bridge], false) - item->Pose.Position.y;
		
		frontFloor = bridgeBorder - CLICK(0.5f);
		if (frontFloor <= -CLICK(2) ||
			frontFloor > CLICK(1.25f) - 4)
		{
			return false;
		}
	}

	// TODO: Reference attractor for water exit.

	TestForObjectOnLedge(item, coll);
	if (coll->HitStatic)
		return false;

	auto probe = GetCollision(item, coll->Setup.ForwardAngle, CLICK(2), -CLICK(1));
	int headroom = probe.Position.Floor - probe.Position.Ceiling;

	if (frontFloor <= -CLICK(1))
	{
		if (headroom < LARA_HEIGHT)
		{
			if (g_GameFlow->HasCrawlExtended())
				SetAnimation(item, LA_ONWATER_TO_CROUCH_1_STEP);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_1_STEP);
	}
	else if (frontFloor > CLICK(0.5f))
	{
		if (headroom < LARA_HEIGHT)
		{
			if (g_GameFlow->HasCrawlExtended())
				SetAnimation(item, LA_ONWATER_TO_CROUCH_M1_STEP);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_M1_STEP);
	}

	else
	{
		if (headroom < LARA_HEIGHT)
		{
			if (g_GameFlow->HasCrawlExtended())
				SetAnimation(item, LA_ONWATER_TO_CROUCH_0_STEP);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_0_STEP);
	}

	UpdateLaraRoom(item, -LARA_HEIGHT / 2);
	AlignEntityToEdge(item, coll, 1.7f);

	item->Pose.Position.y += frontFloor - 5;
	item->Animation.ActiveState = LS_ONWATER_EXIT;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.z = 0;
	item->Animation.Velocity.y = 0;
	lara->Control.TurnRate = 0;
	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.WaterStatus = WaterStatus::Dry;
	return true;
}

bool TestLaraLadderClimbOut(ItemInfo* item, CollisionInfo* coll) // NEW function for water to ladder move
{
	auto* lara = GetLaraInfo(item);

	if (!IsHeld(In::Action) || !lara->Control.CanClimbLadder || coll->CollisionType != CT_FRONT)
	{
		return false;
	}

	if (lara->Control.HandStatus != HandStatus::Free &&
		(lara->Control.HandStatus != HandStatus::WeaponReady || lara->Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return false;
	}

	// HACK: Reduce probe radius, because free forward probe mode makes ladder tests to fail in some cases.
	coll->Setup.Radius *= 0.8f; 

	if (!TestLaraClimbIdle(item, coll))
		return false;

	short facing = item->Pose.Orientation.y;

	if (facing >= -ANGLE(35.0f) && facing <= ANGLE(35.0f))
		facing = 0;
	else if (facing >= ANGLE(55.0f) && facing <= ANGLE(125.0f))
		facing = ANGLE(90.0f);
	else if (facing >= ANGLE(145.0f) || facing <= -ANGLE(145.0f))
		facing = ANGLE(180.0f);
	else if (facing >= -ANGLE(125.0f) && facing <= -ANGLE(55.0f))
		facing = -ANGLE(90.0f);

	if (facing & 0x3FFF)
		return false;

	switch ((unsigned short)facing / ANGLE(90.0f))
	{
	case NORTH:
		item->Pose.Position.z = (item->Pose.Position.z | WALL_MASK) - LARA_RADIUS - 1;
		break;

	case EAST:
		item->Pose.Position.x = (item->Pose.Position.x | WALL_MASK) - LARA_RADIUS - 1;
		break;

	case SOUTH:
		item->Pose.Position.z = (item->Pose.Position.z & -BLOCK(1)) + LARA_RADIUS + 1;
		break;

	case WEST:
		item->Pose.Position.x = (item->Pose.Position.x & -BLOCK(1)) + LARA_RADIUS + 1;
		break;
	}

	SetAnimation(item, LA_ONWATER_IDLE);
	item->Animation.TargetState = LS_LADDER_IDLE;
	AnimateItem(item);

	item->Pose.Position.y -= 10; // Otherwise she falls back into the water.
	item->Pose.Orientation = EulerAngles(0, facing, 0);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity = Vector3::Zero;
	lara->Control.TurnRate = 0;
	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.WaterStatus = WaterStatus::Dry;
	return true;
}

void TestLaraWaterDepth(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	auto pointColl = GetCollision(item);
	int waterDepth = GetWaterDepth(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, pointColl.RoomNumber);

	if (waterDepth == NO_HEIGHT)
	{
		item->Animation.Velocity.y = 0.0f;
		item->Pose.Position = coll->Setup.PrevPosition;
	}

	else if (waterDepth <= (LARA_HEIGHT - (LARA_HEADROOM / 2)))
	{
		SetAnimation(item, LA_UNDERWATER_TO_STAND);
		ResetPlayerLean(item);
		item->Animation.TargetState = LS_IDLE;
		item->Pose.Position.y = pointColl.Position.Floor;
		item->Animation.IsAirborne = false;
		item->Animation.Velocity = Vector3::Zero;
		player.Control.WaterStatus = WaterStatus::Wade;
	}
}

bool TestLaraWeaponType(LaraWeaponType refWeaponType, const std::vector<LaraWeaponType>& weaponTypeList)
{
	return Contains(weaponTypeList, refWeaponType);
}

static std::vector<LaraWeaponType> StandingWeaponTypes
{
	LaraWeaponType::Shotgun,
	LaraWeaponType::HK,
	LaraWeaponType::Crossbow,
	LaraWeaponType::Torch,
	LaraWeaponType::GrenadeLauncher,
	LaraWeaponType::HarpoonGun,
	LaraWeaponType::RocketLauncher,
	LaraWeaponType::Snowmobile
};

bool IsStandingWeapon(const ItemInfo* item, LaraWeaponType weaponType)
{
	return (TestLaraWeaponType(weaponType, StandingWeaponTypes) || GetLaraInfo(*item).Weapons[(int)weaponType].HasLasersight);
}

bool IsVaultState(int state)
{
	static const std::vector<int> vaultStates
	{
		LS_VAULT,
		LS_VAULT_2_STEPS,
		LS_VAULT_3_STEPS,
		LS_VAULT_1_STEP_CROUCH,
		LS_VAULT_2_STEPS_CROUCH,
		LS_VAULT_3_STEPS_CROUCH,
		LS_AUTO_JUMP
	};
	return TestState(state, vaultStates);
}

bool IsJumpState(int state)
{
	static const std::vector<int> jumpStates
	{
		LS_JUMP_FORWARD,
		LS_JUMP_BACK,
		LS_JUMP_LEFT,
		LS_JUMP_RIGHT,
		LS_JUMP_UP,
		LS_FALL_BACK,
		LS_REACH,
		LS_SWAN_DIVE,
		LS_FREEFALL_DIVE,
		LS_FREEFALL
	};
	return TestState(state, jumpStates);
}

bool IsRunJumpQueueableState(int state)
{
	static const auto RUN_JUMP_QUEUABLE_STATES = std::vector<int>
	{
		LS_RUN_FORWARD,
		LS_SPRINT,
		LS_STEP_UP,
		LS_STEP_DOWN
	};

	return TestState(state, RUN_JUMP_QUEUABLE_STATES);
}

bool IsRunJumpCountableState(int state)
{
	static const std::vector<int> runningJumpTimerStates
	{
		LS_WALK_FORWARD,
		LS_RUN_FORWARD,
		LS_SPRINT,
		LS_SPRINT_DIVE,
		LS_JUMP_FORWARD
	};
	return TestState(state, runningJumpTimerStates);
}

// TODO: This function should become obsolete with more accurate and accessible collision detection in the future.
// For now, it supersedes old probes and is used alongside COLL_INFO. @Sezz 2021.10.24
bool TestLaraMoveTolerance(ItemInfo* item, CollisionInfo* coll, MoveTestSetup testSetup, bool useCrawlSetup)
{
	// HACK: coll->Setup.Radius and coll->Setup.Height are set in
	// lara_col functions, then reset by LaraAboveWater() to defaults.
	// This means they store the wrong values for move tests called in crawl lara_as functions.
	// When states become objects, collision setup should occur at the beginning of each state, eliminating the need
	// for the useCrawlSetup argument. @Sezz 2022.03.16
	int laraRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
	int laraHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(laraRadius);
	auto probe = GetCollision(item, testSetup.Angle, distance, -laraHeight);

	bool isSlopeDown = testSetup.CheckSlopeDown ? (probe.Position.FloorSlope && probe.Position.Floor > y) : false;
	bool isSlopeUp = testSetup.CheckSlopeUp ? (probe.Position.FloorSlope && probe.Position.Floor < y) : false;
	bool isDeath = testSetup.CheckDeath ? probe.Block->Flags.Death : false;

	auto start1 = GameVector(
		item->Pose.Position.x,
		y + testSetup.UpperFloorBound - 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end1 = GameVector(
		probe.Coordinates.x,
		y + testSetup.UpperFloorBound - 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	auto start2 = GameVector(
		item->Pose.Position.x,
		y - laraHeight + 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end2 = GameVector(
		probe.Coordinates.x,
		probe.Coordinates.y + 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	// Discard walls.
	if (probe.Position.Floor == NO_HEIGHT)
		return false;

	// Check for slope or death sector (if applicable).
	if (isSlopeDown || isSlopeUp || isDeath)
		return false;

	// Assess ray/room collision.
	if (!LOS(&start1, &end1) || !LOS(&start2, &end2))
		return false;

	// Assess point/room collision.
	if ((probe.Position.Floor - y) <= testSetup.LowerFloorBound &&			// Within lower floor bound.
		(probe.Position.Floor - y) >= testSetup.UpperFloorBound &&			// Within upper floor bound.
		(probe.Position.Ceiling - y) < -laraHeight &&						// Within lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > laraHeight)	// Space is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyMoveTolerance(ItemInfo* item, CollisionInfo* coll, MonkeyMoveTestSetup testSetup)
{
	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probe = GetCollision(item, testSetup.Angle, distance);

	auto start1 = GameVector(
		item->Pose.Position.x,
		y + testSetup.LowerCeilingBound + 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end1 = GameVector(
		probe.Coordinates.x,
		probe.Coordinates.y - LARA_HEIGHT_MONKEY + testSetup.LowerCeilingBound + 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	auto start2 = GameVector(
		item->Pose.Position.x,
		y + LARA_HEIGHT_MONKEY - 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end2 = GameVector(
		probe.Coordinates.x,
		probe.Coordinates.y - 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	// Discard walls.
	if (probe.Position.Ceiling == NO_HEIGHT)
		return false;

	// Check for ceiling slope.
	if (probe.Position.CeilingSlope)
		return false;

	// Assess ray/room collision.
	if (!LOS(&start1, &end1) || !LOS(&start2, &end2))
		return false;

	// Assess point/room collision.
	if (probe.BottomBlock->Flags.Monkeyswing &&										// Is monkey sector.
		(probe.Position.Floor - y) > LARA_HEIGHT_MONKEY &&							// Within highest floor bound.
		(probe.Position.Ceiling - y) <= testSetup.LowerCeilingBound &&				// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= testSetup.UpperCeilingBound &&				// Within upper ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_MONKEY)	// Space is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey forward collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.y,
		int(CLICK(1.25f)), int(-CLICK(1.25f))
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

bool TestLaraMonkeyBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey back collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(180.0f),
		int(CLICK(1.25f)), int(-CLICK(1.25f))
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

bool TestLaraMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey shimmy left collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.y - ANGLE(90.0f),
		int(CLICK(0.5f)), int(-CLICK(0.5f))
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

bool TestLaraMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey shimmy right collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(90.0f),
		int(CLICK(0.5f)), int(-CLICK(0.5f))
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

std::optional<VaultTestResult> TestLaraVaultTolerance(ItemInfo* item, CollisionInfo* coll, VaultTestSetup testSetup)
{
	auto* lara = GetLaraInfo(item);

	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
	bool swampTooDeep = testSetup.CheckSwampDepth ? (isSwamp && lara->Context.WaterSurfaceDist < -CLICK(3)) : isSwamp;
	int y = isSwamp ? item->Pose.Position.y : probeMiddle.Position.Floor; // HACK: Avoid cheese when in the midst of performing a step. Can be done better. @Sezz 2022.04.08	

	// Check swamp depth (if applicable).
	if (swampTooDeep)
		return std::nullopt;

	// NOTE: Where the point/room probe finds that
	// a) the "wall" in front is formed by a ceiling, or
	// b) the space between the floor and ceiling is a clamp (i.e. it is too narrow),
	// any potentially climbable floor in a room above will be missed. The following block considers this.
	
	// Raise y position of point/room probe by increments of CLICK(0.5f) to find potential vault ledge.
	int yOffset = testSetup.LowerFloorBound;
	while (((probeFront.Position.Ceiling - y) > -coll->Setup.Height ||								// Ceiling is below Lara's height...
			abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMin ||		// OR clamp is too small
			abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMax) &&		// OR clamp is too large (future-proofing; not possible right now).
		yOffset > (testSetup.UpperFloorBound - coll->Setup.Height))									// Offset is not too high.
	{
		probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, yOffset);
		yOffset -= std::max<int>(CLICK(0.5f), testSetup.ClampMin);
	}

	// Discard walls.
	if (probeFront.Position.Floor == NO_HEIGHT)
		return std::nullopt;

	// Assess point/room collision.
	if ((probeFront.Position.Floor - y) < testSetup.LowerFloorBound &&							// Within lower floor bound.
		(probeFront.Position.Floor - y) >= testSetup.UpperFloorBound &&							// Within upper floor bound.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMin &&	// Within clamp min.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMax &&	// Within clamp max.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= testSetup.GapMin)		// Gap is optically permissive.
	{
		return VaultTestResult{ probeFront.Position.Floor };
	}

	return std::nullopt;
}

std::optional<VaultTestResult> TestLaraVault2Steps(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
	// Clamp range: (-LARA_HEIGHT, -MAX_HEIGHT]

	VaultTestSetup testSetup
	{
		-STEPUP_HEIGHT, int(-CLICK(2.5f)),
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1)
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	if (!testResult.has_value())
		return std::nullopt;

	testResult->Height += CLICK(2);
	testResult->SetBusyHands = true;
	testResult->SnapToLedge = true;
	testResult->SetJumpVelocity = false;
	return testResult;
}

std::optional<VaultTestResult> TestLaraVault3Steps(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
	// Clamp range: (-LARA_HEIGHT, -MAX_HEIGHT]

	VaultTestSetup testSetup
	{
		int(-CLICK(2.5f)), int(-CLICK(3.5f)),
		LARA_HEIGHT, -MAX_HEIGHT,
		int(CLICK(1)),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	if (!testResult.has_value())
		return std::nullopt;

	testResult->Height += CLICK(3);
	testResult->SetBusyHands = true;
	testResult->SnapToLedge = true;
	testResult->SetJumpVelocity = false;
	return testResult;
}

std::optional<VaultTestResult> TestLaraVault1StepToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (0, -STEPUP_HEIGHT]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestSetup testSetup
	{
		0, -STEPUP_HEIGHT,
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	if (!testResult.has_value())
		return std::nullopt;

	testResult->Height += CLICK(1);
	testResult->SetBusyHands = true;
	testResult->SnapToLedge = true;
	testResult->SetJumpVelocity = false;
	return testResult;
}

std::optional<VaultTestResult> TestLaraVault2StepsToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestSetup testSetup
	{
		-STEPUP_HEIGHT, int(-CLICK(2.5f)),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		int(CLICK(1))
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	if (!testResult.has_value())
		return std::nullopt;

	testResult->Height += CLICK(2);
	testResult->SetBusyHands = true;
	testResult->SnapToLedge = true;
	testResult->SetJumpVelocity = false;
	return testResult;
}

std::optional<VaultTestResult> TestLaraVault3StepsToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestSetup testSetup
	{
		int(-CLICK(2.5f)), int(-CLICK(3.5f)),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		int(CLICK(1)),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	if (!testResult.has_value())
		return std::nullopt;

	testResult->Height += CLICK(3);
	testResult->SetBusyHands = true;
	testResult->SnapToLedge = true;
	testResult->SetJumpVelocity = false;
	return testResult;
}

std::optional<VaultTestResult> TestLaraLedgeAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-CLICK(3.5f), -CLICK(7.5f)]
	// Clamp range: (-CLICK(0.1f), -MAX_HEIGHT]

	VaultTestSetup testSetup
	{
		int(-CLICK(3.5f)), int(-CLICK(7.5f)),
		int(CLICK(0.1f)) /* TODO: Is this enough hand room?*/, -MAX_HEIGHT,
		int(CLICK(0.1f)),
		false
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	if (!testResult.has_value())
		return std::nullopt;

	testResult->SetBusyHands = false;
	testResult->SnapToLedge = true;
	testResult->SetJumpVelocity = true;
	return testResult;
}

std::optional<VaultTestResult> TestLaraLadderAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Check ledge angle.
	if (!TestPlayerInteractAngle(*item, coll->NearestLedgeAngle))
		return std::nullopt;

	if (lara->Control.CanClimbLadder &&								// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(6.5f) &&		// Within lowest middle ceiling bound. (Synced with TestLaraLadderMount())
		((probeFront.Position.Floor - y) <= -CLICK(6.5f) ||			// Floor height is appropriate, OR
			(probeFront.Position.Ceiling - y) > -CLICK(6.5f)) &&		// Ceiling height is appropriate. (Synced with TestLaraLadderMount())
		coll->NearestLedgeDistance <= coll->Setup.Radius)			// Appropriate distance from wall.
	{
		return VaultTestResult{ probeMiddle.Position.Ceiling, false, true, true };
	}

	return std::nullopt;
}

std::optional<VaultTestResult> TestLaraLadderMount(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Check ledge angle.
	if (!TestPlayerInteractAngle(*item, coll->NearestLedgeAngle))
		return std::nullopt;

	if (lara->Control.CanClimbLadder &&							// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(4.5f) &&	// Within lower middle ceiling bound.
		(probeMiddle.Position.Ceiling - y) > -CLICK(6.5f) &&	// Within upper middle ceiling bound.
		(probeMiddle.Position.Floor - y) > -CLICK(6.5f) &&		// Within upper middle floor bound. (Synced with TestLaraAutoJump())
		(probeFront.Position.Ceiling - y) <= -CLICK(4.5f) &&	// Within lowest front ceiling bound.
		coll->NearestLedgeDistance <= coll->Setup.Radius)		// Appropriate distance from wall.
	{
		return VaultTestResult{ NO_HEIGHT, true, true, false };
	}

	return std::nullopt;
}

std::optional<VaultTestResult> TestLaraMonkeyAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item);

	if (lara->Control.CanMonkeySwing &&							// Monkey swing sector flag set.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_MONKEY &&	// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(7))				// Within upper ceiling bound.
	{
		return VaultTestResult{ probe.Position.Ceiling, false, false, true };
	}

	return std::nullopt;
}

std::optional<VaultTestResult> TestLaraVault(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.HandStatus != HandStatus::Free)
		return std::nullopt;

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && lara->Context.WaterSurfaceDist < -CLICK(3))
		return std::nullopt;

	std::optional<VaultTestResult> vaultResult = {};

	// TODO: Attractors.
	// Attempt ledge vault.
	if (false)
	{
		// Vault to crouch up one step.
		vaultResult = TestLaraVault1StepToCrouch(item, coll);
		if (vaultResult.has_value())
		{
			vaultResult->TargetState = LS_VAULT_1_STEP_CROUCH;
			if (!HasStateDispatch(item, vaultResult->TargetState))
				return std::nullopt;

			return vaultResult;
		}

		// Vault to stand up two steps.
		vaultResult = TestLaraVault2Steps(item, coll);
		if (vaultResult.has_value())
		{
			vaultResult->TargetState = LS_VAULT_2_STEPS;
			if (!HasStateDispatch(item, vaultResult->TargetState))
				return std::nullopt;

			return vaultResult;
		}

		// Vault to crouch up two steps.
		vaultResult = TestLaraVault2StepsToCrouch(item, coll);
		if (vaultResult.has_value() && g_GameFlow->HasCrawlExtended())
		{
			vaultResult->TargetState = LS_VAULT_2_STEPS_CROUCH;
			if (!HasStateDispatch(item, vaultResult->TargetState))
				return std::nullopt;

			return vaultResult;
		}

		// Vault to stand up three steps.
		vaultResult = TestLaraVault3Steps(item, coll);
		if (vaultResult.has_value())
		{
			vaultResult->TargetState = LS_VAULT_3_STEPS;
			if (!HasStateDispatch(item, vaultResult->TargetState))
				return std::nullopt;

			return vaultResult;
		}

		// Vault to crouch up three steps.
		vaultResult = TestLaraVault3StepsToCrouch(item, coll);
		if (vaultResult.has_value() && g_GameFlow->HasCrawlExtended())
		{
			vaultResult->TargetState = LS_VAULT_3_STEPS_CROUCH;
			if (!HasStateDispatch(item, vaultResult->TargetState))
				return std::nullopt;

			return vaultResult;
		}

		// Auto jump to ledge.
		vaultResult = TestLaraLedgeAutoJump(item, coll);
		if (vaultResult.has_value())
		{
			vaultResult->TargetState = LS_AUTO_JUMP;
			if (!HasStateDispatch(item, vaultResult->TargetState))
				return std::nullopt;

			return vaultResult;
		}
	}

	// TODO: Move ladder checks here when ladders are less prone to breaking.
	// In this case, they fail due to a reliance on ShiftItem(). @Sezz 2021.02.05

	// Auto jump to monkey swing.
	vaultResult = TestLaraMonkeyAutoJump(item, coll);
	if (vaultResult.has_value() && g_GameFlow->HasMonkeyAutoJump())
	{
		vaultResult->TargetState = LS_AUTO_JUMP;
		if (!HasStateDispatch(item, vaultResult->TargetState))
			return std::nullopt;

		return vaultResult;
	}

	return std::nullopt;
}

// Temporary solution to ladder mounts until ladders stop breaking whenever anyone tries to do anything with them. -- Sezz 2022.02.05
bool TestAndDoLaraLadderClimb(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!IsHeld(In::Action) || !IsHeld(In::Forward) || lara->Control.HandStatus != HandStatus::Free)
		return false;

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && lara->Context.WaterSurfaceDist < -CLICK(3))
		return false;

	// Auto jump to ladder.
	auto vaultResult = TestLaraLadderAutoJump(item, coll);
	if (vaultResult.has_value())
	{
		// TODO: Somehow harmonise CalculatedJumpVelocity to work for both ledge and ladder auto jumps, because otherwise there will be a need for an odd workaround in the future.
		lara->Context.CalcJumpVelocity = -3 - sqrt(-9600 - 12 * std::max((vaultResult->Height - item->Pose.Position.y + CLICK(0.2f)), -CLICK(7.1f)));
		SetAnimation(item, LA_STAND_SOLID);
		item->Animation.TargetState = LS_JUMP_UP;
		lara->Control.TurnRate = 0;

		ShiftItem(item, coll);
		SnapEntityToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		lara->Context.TargetOrientation = EulerAngles(0, item->Pose.Orientation.y, 0);
		AnimateItem(item);

		return true;
	}

	// Mount ladder.
	vaultResult = TestLaraLadderMount(item, coll);
	if (vaultResult.has_value() && TestLaraClimbIdle(item, coll))
	{
		SetAnimation(item, LA_STAND_SOLID);
		item->Animation.TargetState = LS_LADDER_IDLE;
		lara->Control.HandStatus = HandStatus::Busy;
		lara->Control.TurnRate = 0;

		ShiftItem(item, coll);
		SnapEntityToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		AnimateItem(item);

		return true;
	}

	return false;
}

CrawlVaultTestResult TestLaraCrawlVaultTolerance(ItemInfo* item, CollisionInfo* coll, CrawlVaultTestSetup testSetup)
{
	int y = item->Pose.Position.y;
	auto probeA = GetCollision(item, item->Pose.Orientation.y, testSetup.CrossDist, -LARA_HEIGHT_CRAWL);	// Crossing.
	auto probeB = GetCollision(item, item->Pose.Orientation.y, testSetup.DestDist, -LARA_HEIGHT_CRAWL);		// Approximate destination.
	auto probeMiddle = GetCollision(item);

	bool isSlope = testSetup.CheckSlope ? probeB.Position.FloorSlope : false;
	bool isDeath = testSetup.CheckDeath ? probeB.Block->Flags.Death : false;

	// Discard walls.
	if (probeA.Position.Floor == NO_HEIGHT || probeB.Position.Floor == NO_HEIGHT)
		return CrawlVaultTestResult{ false };

	// Check for slope or death sector (if applicable).
	if (isSlope || isDeath)
		return CrawlVaultTestResult{ false };

	// Assess point/room collision.
	if ((probeA.Position.Floor - y) <= testSetup.LowerFloorBound &&							// Within lower floor bound.
		(probeA.Position.Floor - y) >= testSetup.UpperFloorBound &&							// Within upper floor bound.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > testSetup.ClampMin &&		// Crossing clamp limit.
		abs(probeB.Position.Ceiling - probeB.Position.Floor) > testSetup.ClampMin &&		// Destination clamp limit.
		abs(probeMiddle.Position.Ceiling - probeA.Position.Floor) >= testSetup.GapMin &&	// Gap is optically permissive (going up).
		abs(probeA.Position.Ceiling - probeMiddle.Position.Floor) >= testSetup.GapMin &&	// Gap is optically permissive (going down).
		abs(probeA.Position.Floor - probeB.Position.Floor) <= testSetup.FloorBound &&		// Crossing/destination floor height difference suggests continuous crawl surface.
		(probeA.Position.Ceiling - y) < -testSetup.GapMin)									// Ceiling height is permissive.
	{
		return CrawlVaultTestResult{ true };
	}

	return CrawlVaultTestResult{ false };
}

CrawlVaultTestResult TestLaraCrawlUpStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [-CLICK(1), -STEPUP_HEIGHT]

	CrawlVaultTestSetup testSetup
	{
		int(-CLICK(1)), -STEPUP_HEIGHT,
		LARA_HEIGHT_CRAWL,
		int(CLICK(0.6f)),
		int(CLICK(1.2f)),
		int(CLICK(2)),
		int(CLICK(1) - 1)
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlDownStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [STEPUP_HEIGHT, CLICK(1)]

	CrawlVaultTestSetup testSetup
	{
		STEPUP_HEIGHT, CLICK(1),
		LARA_HEIGHT_CRAWL,
		int(CLICK(0.6f)),
		int(CLICK(1.2f)),
		int(CLICK(2)),
		int(CLICK(1) - 1)
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlExitDownStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [STEPUP_HEIGHT, CLICK(1)]

	CrawlVaultTestSetup testSetup
	{
		STEPUP_HEIGHT, int(CLICK(1)),
		LARA_HEIGHT,
		int(CLICK(1.25f)),
		int(CLICK(1.2f)),
		int(CLICK(1.5f)),
		-MAX_HEIGHT,
		false, false
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlExitJump(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [NO_LOWER_BOUND, STEPUP_HEIGHT)

	CrawlVaultTestSetup testSetup
	{
		NO_LOWER_BOUND, STEPUP_HEIGHT + 1,
		LARA_HEIGHT,
		int(CLICK(1.25f)),
		int(CLICK(1.2f)),
		int(CLICK(1.5f)),
		NO_LOWER_BOUND,
		false, false
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlVault(ItemInfo* item, CollisionInfo* coll)
{
	if (!(IsHeld(In::Action) || IsHeld(In::Jump)))
		return CrawlVaultTestResult{ false };

	// Crawl vault exit down 1 step.
	auto crawlVaultResult = TestLaraCrawlExitDownStep(item, coll);
	if (crawlVaultResult.Success)
	{
		if (IsHeld(In::Crouch) && TestLaraCrawlDownStep(item, coll).Success)
			crawlVaultResult.TargetState = LS_CRAWL_STEP_DOWN;
		else USE_FEATURE_IF_CPP20([[likely]])
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_STEP_DOWN;

		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault exit jump.
	crawlVaultResult = TestLaraCrawlExitJump(item, coll);
	if (crawlVaultResult.Success)
	{
		if (IsHeld(In::Walk))
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_FLIP;
		else USE_FEATURE_IF_CPP20([[likely]])
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_JUMP;

		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault up 1 step.
	crawlVaultResult = TestLaraCrawlUpStep(item, coll);
	if (crawlVaultResult.Success)
	{
		crawlVaultResult.TargetState = LS_CRAWL_STEP_UP;
		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault down 1 step.
	crawlVaultResult = TestLaraCrawlDownStep(item, coll);
	if (crawlVaultResult.Success)
	{
		crawlVaultResult.TargetState = LS_CRAWL_STEP_DOWN;
		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	return CrawlVaultTestResult{ false };
}

bool TestLaraCrawlToHang(ItemInfo* item, CollisionInfo* coll)
{
	int y = item->Pose.Position.y;
	int distance = CLICK(1.2f);
	auto probe = GetCollision(item, item->Pose.Orientation.y + ANGLE(180.0f), distance, -LARA_HEIGHT_CRAWL);

	bool objectCollided = TestLaraObjectCollision(item, item->Pose.Orientation.y + ANGLE(180.0f), CLICK(1.2f), -LARA_HEIGHT_CRAWL);

	if (!objectCollided &&										// No obstruction.
		(probe.Position.Floor - y) >= LARA_HEIGHT_STRETCH &&	// Highest floor bound.
		(probe.Position.Ceiling - y) <= -CLICK(0.75f) &&		// Gap is optically permissive.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraPoleCollision(ItemInfo* item, CollisionInfo* coll, bool goingUp, float offset)
{
	static constexpr auto poleProbeCollRadius = 16.0f;

	bool atLeastOnePoleCollided = false;

	if (GetCollidedObjects(item, BLOCK(1), true, CollidedItems, nullptr, false) &&
		CollidedItems[0] != nullptr)
	{
		auto laraBox = GameBoundingBox(item).ToBoundingOrientedBox(item->Pose);

		// HACK: Because Core implemented upward pole movement as a SetPosition command, we can't precisely
		// check her position. So we add a fixed height offset.

		// Offset a sphere when jumping toward pole.
		auto sphereOffset2D = Vector3::Zero;
		sphereOffset2D = Geometry::TranslatePoint(sphereOffset2D, item->Pose.Orientation.y, coll->Setup.Radius + item->Animation.Velocity.z);

		auto spherePos = laraBox.Center + Vector3(0.0f, (laraBox.Extents.y + poleProbeCollRadius + offset) * (goingUp ? -1 : 1), 0.0f);

		auto sphere = BoundingSphere(spherePos, poleProbeCollRadius);
		auto offsetSphere = BoundingSphere(spherePos + sphereOffset2D, poleProbeCollRadius);

		//g_Renderer.AddDebugSphere(sphere.Center, 16.0f, Vector4(1, 0, 0, 1), RendererDebugPage::CollisionStats);

		int i = 0;
		while (CollidedItems[i] != nullptr)
		{
			auto*& object = CollidedItems[i];
			i++;

			if (object->ObjectNumber != ID_POLEROPE)
				continue;

			auto poleBox = GameBoundingBox(object).ToBoundingOrientedBox(object->Pose);
			poleBox.Extents = poleBox.Extents + Vector3(coll->Setup.Radius, 0.0f, coll->Setup.Radius);

			//g_Renderer.AddDebugBox(poleBox, Vector4(0, 0, 1, 1), RendererDebugPage::CollisionStats);

			if (poleBox.Intersects(sphere) || poleBox.Intersects(offsetSphere))
			{
				atLeastOnePoleCollided = true;
				break;
			}
		}
	}

	return atLeastOnePoleCollided;
}

bool TestLaraPoleUp(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestLaraPoleCollision(item, coll, true, CLICK(1)))
		return false;

	return (coll->Middle.Ceiling < -CLICK(1));
}

bool TestLaraPoleDown(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestLaraPoleCollision(item, coll, false))
		return false;

	return (coll->Middle.Floor > 0);
}
