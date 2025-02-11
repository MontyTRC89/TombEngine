#include "framework.h"
#include "Game/Lara/Context/MonkeySwing.h"

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
	bool CanPerformMonkeySwingStep(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto LOWER_CEIL_BOUND = MONKEY_STEPUP_HEIGHT;
		constexpr auto UPPER_CEIL_BOUND = -MONKEY_STEPUP_HEIGHT;

		// Get point collision.
		auto pointColl = GetPointCollision(item);
		int relCeilHeight = pointColl.GetCeilingHeight() - (item.Pose.Position.y - LARA_HEIGHT_MONKEY);

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
		auto pointColl = GetPointCollision(item);

		// 2) Test for slippery ceiling slope and check if overhang climb is disabled.
		if (pointColl.IsSteepCeiling() && !g_GameFlow->GetSettings()->Animations.OverhangClimb)
			return true;

		// 3) Assess point collision.
		int relCeilHeight = pointColl.GetCeilingHeight() - (item.Pose.Position.y - LARA_HEIGHT_MONKEY);
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
		auto pointColl = GetPointCollision(item);
		int relCeilHeight = pointColl.GetCeilingHeight() - (item.Pose.Position.y - LARA_HEIGHT_MONKEY);
		int floorToCeilHeight = abs(pointColl.GetCeilingHeight() - pointColl.GetFloorHeight());

		// 2) Assess collision with ceiling.
		if (relCeilHeight < 0 &&
			coll.CollisionType != CollisionType::Top &&
			coll.CollisionType != CollisionType::TopFront)
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
		auto pointColl = GetPointCollision(item, setup.HeadingAngle, OFFSET_RADIUS(coll.Setup.Radius));

		// 1) Test if ceiling is monkey swing.
		if (!pointColl.GetBottomSector().Flags.Monkeyswing)
			return false;

		// 2) Test for steep ceiling.
		if (pointColl.IsSteepCeiling())
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
			pointColl.GetPosition().x,
			vPos - 1,
			pointColl.GetPosition().z,
			item.RoomNumber);

		// Raycast setup at lower ceiling bound.
		auto origin1 = GameVector(
			item.Pose.Position.x,
			(vPosTop + setup.LowerCeilingBound) + 1,
			item.Pose.Position.z,
			item.RoomNumber);
		auto target1 = GameVector(
			pointColl.GetPosition().x,
			(vPosTop + setup.LowerCeilingBound) + 1,
			pointColl.GetPosition().z,
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

		int relFloorHeight = pointColl.GetFloorHeight() - vPos;
		int relCeilHeight = pointColl.GetCeilingHeight() - vPosTop;
		int floorToCeilHeight = abs(pointColl.GetCeilingHeight() - pointColl.GetFloorHeight());

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
			(int)CLICK(0.5f), (int)-CLICK(0.5f) // NOTE: Bounds defined by monkey shimmy left/right states.
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
}
