#include "framework.h"
#include "Game/Lara/Context/Jump.h"

#include "Game/animation.h"
#include "Game/collision/Attractor.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/Context/Structs.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/configuration.h"
#include "Specific/Input/Input.h"

using namespace TEN::Collision::Attractor;
using namespace TEN::Collision::Point;
using namespace TEN::Input;

namespace TEN::Player
{
	bool CanFall(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto UPPER_FLOOR_BOUND = STEPUP_HEIGHT;

		const auto& player = GetLaraInfo(item);

		// 1) Test player water status.
		if (player.Control.WaterStatus == WaterStatus::Wade)
			return false;

		// Get point collision.
		auto pointColl = GetPointCollision(item, 0, 0, -coll.Setup.Height / 2);
		int relFloorHeight = pointColl.GetFloorHeight() - item.Pose.Position.y;

		// 2) Assess point collision.
		if (relFloorHeight > UPPER_FLOOR_BOUND) // Floor height is below upper floor bound.
			return true;

		return false;
	}

	bool CanLand(const ItemInfo& item, const CollisionInfo& coll)
	{
		float projVerticalVel = item.Animation.Velocity.y + GetEffectiveGravity(item.Animation.Velocity.y);

		// 1) Check airborne status and vertical velocity.
		if (!item.Animation.IsAirborne || projVerticalVel < 0.0f)
			return false;

		// 2) Check for swamp.
		if (TestEnvironment(ENV_FLAG_SWAMP, &item))
			return true;

		// Get point collision.
		auto pointColl = GetPointCollision(item);
		int vPos = item.Pose.Position.y;

		// 3) Assess point collision.
		if ((pointColl.GetFloorHeight() - vPos) <= projVerticalVel) // Floor height is above projected vertical position.
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
		auto pointColl = GetPointCollision(item, setup.HeadingAngle, setup.Distance, -coll.Setup.Height);
		int relFloorHeight = pointColl.GetFloorHeight() - item.Pose.Position.y;
		int relCeilHeight = pointColl.GetCeilingHeight() - item.Pose.Position.y;

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
		auto pointColl = GetPointCollision(item, item.Pose.Orientation.y, BLOCK(1), -coll.Setup.Height);

		int lowerCeilingBound = (LOWER_CEIL_BOUND_BASE - coll.Setup.Height);
		int relFloorHeight = pointColl.GetFloorHeight() - item.Pose.Position.y;
		int relCeilHeight = pointColl.GetCeilingHeight() - item.Pose.Position.y;

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

		auto pointColl = GetPointCollision(item);

		//short aspectAngle = GetLaraSlideHeadingAngle(item, coll);
		//short slopeAngle = Geometry::GetSurfaceSlopeAngle(GetSurfaceNormal(pointColl.FloorTilt, true));
		//return (abs(short(coll.Setup.ForwardAngle - aspectAngle)) <= abs(slopeAngle));
	}

	bool CanCrawlspaceDive(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto pointColl = GetPointCollision(item, coll.Setup.ForwardAngle, coll.Setup.Radius, -coll.Setup.Height);
		return (abs(pointColl.GetCeilingHeight() - pointColl.GetFloorHeight()) < LARA_HEIGHT || IsInLowSpace(item, coll));
	}

	static std::optional<AttractorCollisionData> GetEdgeCatchAttractorCollision(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ATTRAC_DETECT_RADIUS		 = BLOCK(0.5f);
		constexpr auto FLOOR_TO_EDGE_HEIGHT_MIN	 = LARA_HEIGHT_STRETCH;
		constexpr auto WALL_EDGE_FLOOR_THRESHOLD = CLICK(0.25f);

		constexpr auto POINT_COLL_BACK_DOWN_OFFSET	   = -CLICK(1);
		constexpr auto POINT_COLL_FRONT_FORWARD_OFFSET = BLOCK(1 / 256.0f);

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(
			item.Pose.Position.ToVector3(), ATTRAC_DETECT_RADIUS, item.RoomNumber, item.Pose.Orientation.y,
			0.0f, -coll.Setup.Height, 0.0f);

		float range2D = std::max<float>(OFFSET_RADIUS(coll.Setup.Radius), Vector2(item.Animation.Velocity.x, item.Animation.Velocity.z).Length());

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check attractor type.
			if (attracColl.Attractor->GetType() != AttractorType::Edge &&
				attracColl.Attractor->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 2) Test if edge is within 2D range.
			if (attracColl.Distance2D > range2D)
				continue;

			// 3) Test if edge slope is steep.
			if (abs(attracColl.SlopeAngle) >= STEEP_FLOOR_SLOPE_ANGLE)
				continue;

			// 4) Test edge angle relation.
			if (!attracColl.IsInFront || !TestPlayerInteractAngle(item.Pose.Orientation.y, attracColl.HeadingAngle))
				continue;

			// Get point collision behind edge.
			auto pointCollBack = GetPointCollision(
				attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius, 0.0f, POINT_COLL_BACK_DOWN_OFFSET);

			// 5) Test if edge is high enough from floor.
			int floorToEdgeHeight = pointCollBack.GetFloorHeight() - attracColl.Intersection.y;
			if (floorToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN)
				continue;

			// 6) Test if edge is high enough from water surface (if applicable).
			if (pointCollBack.GetWaterTopHeight() != NO_HEIGHT && pointCollBack.GetWaterBottomHeight() != NO_HEIGHT)
			{
				int waterSurfaceToEdgeHeight = pointCollBack.GetWaterTopHeight() - attracColl.Intersection.y;
				int waterBottomToEdgeHeight = pointCollBack.GetWaterBottomHeight() - attracColl.Intersection.y;

				if (waterSurfaceToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN &&
					waterBottomToEdgeHeight >= 0)
				{
					continue;
				}
			}

			// 7) Test if ceiling behind is adequately higher than edge.
			int edgeToCeilHeight = pointCollBack.GetCeilingHeight() - attracColl.Intersection.y;
			if (edgeToCeilHeight >= 0)
				continue;

			int vPos = item.Pose.Position.y - coll.Setup.Height;
			int relEdgeHeight = attracColl.Intersection.y - vPos;

			float projVerticalVel = item.Animation.Velocity.y + GetEffectiveGravity(item.Animation.Velocity.y);
			bool isFalling = (projVerticalVel >= 0.0f);

			// 8) SPECIAL CASE: Wall edge.
			if (attracColl.Attractor->GetType() == AttractorType::WallEdge)
			{
				// 8.1) Test if player is falling.
				if (!isFalling)
					return std::nullopt;

				// Get point collision in front of edge.
				auto pointCollFront = GetPointCollision(
					attracColl.Intersection, attracColl.Attractor->GetRoomNumber(),
					attracColl.HeadingAngle, POINT_COLL_FRONT_FORWARD_OFFSET, 0.0f, POINT_COLL_BACK_DOWN_OFFSET);

				// TODO: Can do it another way. Parent stacked WallEdge attractors to pushables and gates?
				// 8.2) Test if wall edge is near wall.
				if (pointCollFront.GetFloorHeight() > (attracColl.Intersection.y + WALL_EDGE_FLOOR_THRESHOLD) &&
					pointCollFront.GetCeilingHeight() < (attracColl.Intersection.y - WALL_EDGE_FLOOR_THRESHOLD))
				{
					return std::nullopt;
				}
			}

			int lowerBound = isFalling ? (int)ceil(projVerticalVel) : 0;
			int upperBound = isFalling ? 0 : (int)floor(projVerticalVel);

			// 9) Test catch trajectory.
			if (relEdgeHeight > lowerBound ||
				relEdgeHeight < upperBound)
			{
				return std::nullopt;
			}

			return attracColl;
		}

		// No valid edge attractor collision; return nullopt.
		return std::nullopt;
	}

	const bool CanSwingOnLedge(const ItemInfo& item, const CollisionInfo& coll, const AttractorCollisionData& attracColl)
	{
		constexpr auto UPPER_FLOOR_BOUND = 0;
		constexpr auto LOWER_CEIL_BOUND	 = CLICK(1.5f);

		auto& player = GetLaraInfo(item);

		// Get point collision.
		auto pointColl = GetPointCollision(
			attracColl.Intersection, item.RoomNumber,
			attracColl.HeadingAngle, coll.Setup.Radius / 2, coll.Setup.Height);

		int relFloorHeight = pointColl.GetFloorHeight() - (attracColl.Intersection.y + coll.Setup.Height);
		int relCeilHeight = pointColl.GetCeilingHeight() - attracColl.Intersection.y;

		// Assess point collision.
		if (relFloorHeight >= UPPER_FLOOR_BOUND && // Floor height is below upper floor bound.
			relCeilHeight <= LOWER_CEIL_BOUND)	   // Ceiling height is above lower ceiling bound.
		{
			return true;
		}

		return false;
	}

	static std::optional<ClimbContextData> GetEdgeJumpCatchClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto VERTICAL_OFFSET = LARA_HEIGHT_STRETCH;

		// Get edge catch climb context.
		auto attracColl = GetEdgeCatchAttractorCollision(item, coll);
		if (attracColl.has_value())
		{
			int targetStateID = ((item.Animation.ActiveState == LS_REACH) && CanSwingOnLedge(item, coll, *attracColl)) ?
				LS_EDGE_HANG_SWING_CATCH : LS_EDGE_HANG_IDLE;

			auto context = ClimbContextData{};
			context.Attractor = attracColl->Attractor;
			context.PathDistance = attracColl->PathDistance;
			context.RelPosOffset = Vector3(0.0f, VERTICAL_OFFSET, -coll.Setup.Radius);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = targetStateID;
			context.AlignType = ClimbContextAlignType::AttractorParent;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	static std::optional<ClimbContextData> GetMonkeySwingJumpCatchClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto ABS_CEIL_BOUND			= CLICK(0.5f);
		constexpr auto FLOOR_TO_CEIL_HEIGHT_MAX = LARA_HEIGHT_MONKEY;

		const auto& player = GetLaraInfo(item);

		// 1) Check for monkey swing flag.
		if (!player.Control.CanMonkeySwing)
			return std::nullopt;

		// 2) Check collision type.
		if (coll.CollisionType != CollisionType::Top &&
			coll.CollisionType != CollisionType::TopFront)
		{
			return std::nullopt;
		}

		// Get point collision.
		auto pointColl = GetPointCollision(item);

		int vPos = item.Pose.Position.y - coll.Setup.Height;
		int relCeilHeight = abs(pointColl.GetCeilingHeight() - vPos);
		int floorToCeilHeight = abs(pointColl.GetCeilingHeight() - pointColl.GetFloorHeight());

		// 3) Assess point collision.
		if (relCeilHeight <= ABS_CEIL_BOUND &&			  // Ceiling height is within lower/upper ceiling bounds.
			floorToCeilHeight > FLOOR_TO_CEIL_HEIGHT_MAX) // Floor-to-ceiling height is wide enough.
		{
			// HACK: Set catch animation.
			int animNumber = (item.Animation.ActiveState == LS_JUMP_UP) ? LA_JUMP_UP_TO_MONKEY : LA_REACH_TO_MONKEY;
			SetAnimation(*LaraItem, animNumber);

			// Get monkey swing catch climb context.
			auto context = ClimbContextData{};
			context.Attractor = nullptr;
			context.PathDistance = 0.0f;
			context.RelPosOffset = Vector3(0.0f, item.Pose.Position.y - (pointColl.GetCeilingHeight() + LARA_HEIGHT_MONKEY), 0.0f);
			context.RelOrientOffset = EulerAngles::Identity;
			context.TargetStateID = LS_MONKEY_IDLE;
			context.AlignType = ClimbContextAlignType::Snap;
			context.IsJump = false;

			return context;
		}

		return std::nullopt;
	}

	// TODO: Dispatch checks. Should be added to several animations for most responsive catch actions.
	std::optional<ClimbContextData> GetJumpCatchClimbContext(const ItemInfo& item, const CollisionInfo& coll)
	{
		auto context = std::optional<ClimbContextData>();

		context = GetEdgeJumpCatchClimbContext(item, coll);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		context = GetMonkeySwingJumpCatchClimbContext(item, coll);
		if (context.has_value())
		{
			if (HasStateDispatch(&item, context->TargetStateID))
				return context;
		}

		return std::nullopt;
	}
}
