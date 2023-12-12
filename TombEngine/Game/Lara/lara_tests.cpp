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

bool TestPlayerInteractAngle(short playerHeadingAngle, short testAngle)
{
	return (abs(Geometry::GetShortestAngle(playerHeadingAngle, testAngle)) <= PLAYER_INTERACT_ANGLE_CONSTRAINT);
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

bool TestLaraLadderClimbOut(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = *GetLaraInfo(item);

	if (!IsHeld(In::Action) || !player.Control.CanClimbLadder || coll->CollisionType != CT_FRONT)
		return false;

	if (player.Control.HandStatus != HandStatus::Free &&
		(player.Control.HandStatus != HandStatus::WeaponReady || player.Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return false;
	}

	// HACK: Reduce probe radius. Free forward probe mode makes ladder tests fail in some cases.
	coll->Setup.Radius *= 0.8f; 

	if (!TestLaraClimbIdle(item, coll))
		return false;

	short headingAngle = item->Pose.Orientation.y;
	if (headingAngle >= ANGLE(-35.0f) && headingAngle <= ANGLE(35.0f))
	{
		headingAngle = 0;
	}
	else if (headingAngle >= ANGLE(55.0f) && headingAngle <= ANGLE(125.0f))
	{
		headingAngle = ANGLE(90.0f);
	}
	else if (headingAngle >= ANGLE(145.0f) || headingAngle <= ANGLE(-145.0f))
	{
		headingAngle = ANGLE(180.0f);
	}
	else if (headingAngle >= ANGLE(-125.0f) && headingAngle <= ANGLE(-55.0f))
	{
		headingAngle = ANGLE(-90.0f);
	}

	if (headingAngle & 0x3FFF)
		return false;

	switch ((unsigned short)headingAngle / ANGLE(90.0f))
	{
	case NORTH:
		item->Pose.Position.z = (item->Pose.Position.z | WALL_MASK) - (LARA_RADIUS - 1);
		break;

	case EAST:
		item->Pose.Position.x = (item->Pose.Position.x | WALL_MASK) - (LARA_RADIUS - 1);
		break;

	case SOUTH:
		item->Pose.Position.z = (item->Pose.Position.z & -BLOCK(1)) + (LARA_RADIUS + 1);
		break;

	case WEST:
		item->Pose.Position.x = (item->Pose.Position.x & -BLOCK(1)) + (LARA_RADIUS + 1);
		break;
	}

	SetAnimation(item, LA_ONWATER_IDLE);
	item->Animation.TargetState = LS_LADDER_IDLE;
	AnimateItem(item);

	item->Pose.Position.y -= 10; // NOTE: Offset required or player will fall back in water.
	item->Pose.Orientation = EulerAngles(0, headingAngle, 0);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity = Vector3::Zero;
	player.Control.TurnRate = 0;
	player.Control.HandStatus = HandStatus::Busy;
	player.Control.WaterStatus = WaterStatus::Dry;
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
		LS_STAND_VAULT,
		LS_STAND_VAULT_2_STEPS_UP,
		LS_STAND_VAULT_3_STEPS_UP,
		LS_STAND_VAULT_1_STEP_UP_TO_CROUCH,
		LS_STAND_VAULT_2_STEPS_UP_TO_CROUCH,
		LS_STAND_VAULT_3_STEPS_UP_TO_CROUCH,
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
