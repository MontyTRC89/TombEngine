#include "framework.h"
#include "Game/Lara/Context.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/ContextData.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Internal/TEN/Flow/FlowHandler.h"

// Debug
#include "Renderer/Renderer11.h"
using TEN::Renderer::g_Renderer;

namespace TEN::Entities::Player::Context
{
	static bool TestLedgeClimbSetup(const ItemInfo& item, CollisionInfo& coll, const LedgeClimbSetupData& setupData)
	{
		constexpr auto ABS_FLOOR_BOUND = CLICK(0.8);

		// Get point collision.
		int probeHeight = -(LARA_HEIGHT_STRETCH + ABS_FLOOR_BOUND);
		auto pointCollCenter = GetCollision(&item);
		auto pointCollFront = GetCollision(&item, setupData.HeadingAngle, OFFSET_RADIUS(coll.Setup.Radius), probeHeight);

		int vPosTop = item.Pose.Position.y - LARA_HEIGHT_STRETCH;
		int relFloorHeight = abs(pointCollFront.Position.Floor - vPosTop);
		int floorToCeilHeight = abs(pointCollFront.Position.Ceiling - pointCollFront.Position.Floor);
		int gapHeight = abs(pointCollCenter.Position.Ceiling - pointCollFront.Position.Floor);

		// 1) Test for slippery slope (if applicable).
		// TODO: This check fails for no reason.
		bool isSlipperySlope = setupData.TestSlipperySlope ? pointCollFront.Position.FloorSlope : false;
		if (isSlipperySlope)
			return false;

		// 2) Test for object blocking ledge.
		TestForObjectOnLedge(&item, &coll);
		if (coll.HitStatic)
			return false;

		// 3) Test for valid ledge.
		if (!TestValidLedge(&item, &coll))
			return false;

		// 4) Assess point collision.
		if (relFloorHeight <= ABS_FLOOR_BOUND &&				   // Floor height is within lower/upper floor bounds.
			floorToCeilHeight > setupData.FloorToCeilHeightMin &&  // Floor-to-ceiling height isn't too narrow.
			floorToCeilHeight <= setupData.FloorToCeilHeightMax && // Floor-to-ceiling height isn't too wide.
			gapHeight >= setupData.GapHeightMin)				   // Gap height is permissive.
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
		auto setupData = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT, -MAX_HEIGHT,
			CLICK(3),
			false
		};

		return TestLedgeClimbSetup(item, coll, setupData);
	}

	bool CanClimbLedgeToCrouch(const ItemInfo& item, CollisionInfo& coll)
	{
		auto setupData = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT_CRAWL, LARA_HEIGHT,
			(int)CLICK(0.6f),
			true
		};

		return TestLedgeClimbSetup(item, coll, setupData);
	}

	bool CanClimbLedgeToStand(const ItemInfo& item, CollisionInfo& coll)
	{
		auto setupData = LedgeClimbSetupData
		{
			item.Pose.Orientation.y,
			LARA_HEIGHT, -MAX_HEIGHT,
			CLICK(1),
			false
		};

		return TestLedgeClimbSetup(item, coll, setupData);
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

	static bool TestLateralShimmy(const ItemInfo& item, const CollisionInfo& coll, short testAngle)
	{
		// Get nearby attractor pointers.
		auto attracPtrs = GetNearbyAttractorPtrs(item);

		// Get attractor collisions.
		auto rotMatrix = Matrix::CreateRotationY(TO_RAD(testAngle));
		auto refPoint = item.Pose.Position.ToVector3() + Vector3::Transform(Vector3(0.0f, -coll.Setup.Height, coll.Setup.Radius), rotMatrix);
		float range = OFFSET_RADIUS(coll.Setup.Radius);
		auto attracColls = GetAttractorCollisions(item, attracPtrs, refPoint, range);

		// Find closest edge attractor.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check if attractor is edge type.
			if (!attracColl.AttractorPtr->IsEdge())
				continue;

			// 2) Check if edge is within range.
			if (!attracColl.IsIntersected)
				continue;

			// 3) Test if edge slope is slippery.
			if (abs(attracColl.SlopeAngle) >= SLIPPERY_SLOPE_ANGLE)
				continue;

			// Get point collision off edge side.
			auto pointCollOffEdge = GetCollision(
				Vector3i(attracColl.ClosestPoint), attracColl.AttractorPtr->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// 4) Test if edge is too low to the ground.
			int floorToEdgeHeight = abs(attracColl.ClosestPoint.y - pointCollOffEdge.Position.Floor);
			if (floorToEdgeHeight <= LARA_HEIGHT_STRETCH)
				continue;

			return true;
		}

		return false;
	}

	bool CanShimmyLeft(ItemInfo& item, CollisionInfo& coll)
	{
		//return TestLateralShimmy(item, coll, ANGLE(-90.0f));
		return TestLaraHangSideways(&item, &coll, ANGLE(-90.0f));
	}

	bool CanShimmyRight(ItemInfo& item, CollisionInfo& coll)
	{
		//return TestLateralShimmy(item, coll, ANGLE(90.0f));
		return TestLaraHangSideways(&item, &coll, ANGLE(90.0f));
	}

	static bool TestPlayerInteractAngle(const ItemInfo& item, short testAngle)
	{
		return (abs(short(testAngle - item.Pose.Orientation.y)) <= LARA_GRAB_THRESHOLD);
	}

	static std::optional<EdgeCatchData> GetLedgeCatchData(const ItemInfo& item, const CollisionInfo& coll,
														  const std::vector<AttractorCollisionData>& attracColls)
	{
		constexpr auto EDGE_TYPE = EdgeType::Ledge;

		const auto& player = GetLaraInfo(item);

		const AttractorCollisionData* attracCollPtr = nullptr;
		float closestDist = INFINITY;

		// Find closest edge attractor.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check if attractor is edge type.
			if (!attracColl.AttractorPtr->IsEdge())
				continue;

			// 2) Check if edge is within range and in front.
			if (!attracColl.IsIntersected || !attracColl.IsInFront)
				continue;

			// 3) Test catch angle.
			if (!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
				continue;

			// 4) Test if edge slope is slippery.
			if (abs(attracColl.SlopeAngle) >= SLIPPERY_SLOPE_ANGLE)
				continue;

			// Get point collision off edge's side.
			auto pointCollOffSide = GetCollision(
				Vector3i(attracColl.ClosestPoint), attracColl.AttractorPtr->GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// 5) Test if edge is too low to the ground.
			int floorToEdgeHeight = abs(attracColl.ClosestPoint.y - pointCollOffSide.Position.Floor);
			if (floorToEdgeHeight <= LARA_HEIGHT_STRETCH)
				continue;

			int vPos = item.Pose.Position.y - coll.Setup.Height;
			int edgeHeight = attracColl.ClosestPoint.y;
			int relEdgeHeight = edgeHeight - vPos;

			bool isMovingUp = (item.Animation.Velocity.y <= 0.0f);
			int lowerBound = isMovingUp ? 0 : item.Animation.Velocity.y;
			int upperBound = isMovingUp ? item.Animation.Velocity.y : 0;

			// 6) Assess point collision to edge.
			if (relEdgeHeight <= lowerBound && // Edge height is above lower height bound.
				relEdgeHeight >= upperBound)   // Edge height is below upper height bound.
			{
				if (attracColl.Distance < closestDist)
				{
					attracCollPtr = &attracColl;
					closestDist = attracColl.Distance;
				}

				continue;
			}
		}

		// No edge found; return nullopt.
		if (attracCollPtr == nullptr)
			return std::nullopt;

		// Return edge catch data.
		return EdgeCatchData{ EDGE_TYPE, attracCollPtr->ClosestPoint, attracCollPtr->HeadingAngle };
	}

	static std::optional<EdgeCatchData> GetClimbableWallEdgeCatchData(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto EDGE_TYPE		= EdgeType::ClimbableWall;
		constexpr auto WALL_STEP_HEIGHT = CLICK(1);

		const auto& player = GetLaraInfo(item);

		// 1) Check for climbable wall flag.
		if (!player.Control.CanClimbLadder)
			return std::nullopt;

		// 2) Test for valid ledge at climbable wall.
		if (!TestValidLedge(&item, &coll, true))
			return std::nullopt;

		// 3) Test movement direction.
		bool isMovingUp = (item.Animation.Velocity.y <= 0.0f);
		if (isMovingUp)
			return std::nullopt;

		// Get point collision.
		auto pointCollCenter = GetCollision(&item);

		int vPos = item.Pose.Position.y - coll.Setup.Height;
		int edgeHeight = (int)floor((vPos + item.Animation.Velocity.y) / WALL_STEP_HEIGHT) * WALL_STEP_HEIGHT;

		// 4) Test if wall edge is too low to the ground.
		int floorToEdgeHeight = abs(edgeHeight - pointCollCenter.Position.Floor);
		if (floorToEdgeHeight <= LARA_HEIGHT_STRETCH)
			return std::nullopt;

		// Get point collision.
		float probeHeight = -(coll.Setup.Height + abs(item.Animation.Velocity.y));
		auto pointCollFront = GetCollision(&item, item.Pose.Orientation.y, OFFSET_RADIUS(coll.Setup.Radius), probeHeight);

		// 5) Test if edge isn't within a wall.
		if (edgeHeight < pointCollFront.Position.Floor && // Edge is above floor.
			edgeHeight > pointCollFront.Position.Ceiling) // Edge is below ceiling.
		{
			return std::nullopt;
		}

		// 6) Test if climbable wall is valid.
		bool isClimbableWall = TestLaraHangOnClimbableWall(&item, &coll);
		if (!isClimbableWall && !TestValidLedge(&item, &coll, true, true))
			return std::nullopt;

		// 7) Assess point collision to wall edge.
		int relEdgeHeight = edgeHeight - vPos;
		if (relEdgeHeight <= item.Animation.Velocity.y && // Edge height is above lower height bound.
			relEdgeHeight >= 0)							  // Edge height is below upper height bound.
		{
			auto offset = Vector3(0.0f, edgeHeight, 0.0f);
			return EdgeCatchData{ EDGE_TYPE, offset };
		}
		
		return std::nullopt;
	}

	std::optional<EdgeCatchData> GetEdgeCatchData(ItemInfo& item, CollisionInfo& coll)
	{
		// Get nearby attractor pointers.
		auto attracPtrs = GetNearbyAttractorPtrs(item);

		// Get attractor collisions.
		auto refPoint = item.Pose.Position.ToVector3() + Vector3(0.0f, -coll.Setup.Height, 0.0f);
		float range = OFFSET_RADIUS(coll.Setup.Radius);
		auto attracColls = GetAttractorCollisions(item, attracPtrs, refPoint, range);

		// 1) Get and return ledge catch data (if valid).
		auto ledgeCatchData = GetLedgeCatchData(item, coll, attracColls);
		if (ledgeCatchData.has_value())
			return ledgeCatchData;

		// 2) Get and return climbable wall edge catch data (if valid).
		auto wallEdgeCatchData = GetClimbableWallEdgeCatchData(item, coll);
		if (wallEdgeCatchData.has_value())
			return wallEdgeCatchData;

		return std::nullopt;
	}

	std::optional<MonkeySwingCatchData> GetMonkeySwingCatchData(const ItemInfo& item, const CollisionInfo& coll)
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
			floorToCeilHeight > FLOOR_TO_CEIL_HEIGHT_MAX) // Floor-to-ceiling height isn't too narrow.
		{
			int animNumber = (item.Animation.ActiveState == LS_JUMP_UP) ? LA_JUMP_UP_TO_MONKEY : LA_REACH_TO_MONKEY;
			int monkeyHeight = pointColl.Position.Ceiling;
			return MonkeySwingCatchData{ animNumber, monkeyHeight };
		}

		return std::nullopt;
	}
}
