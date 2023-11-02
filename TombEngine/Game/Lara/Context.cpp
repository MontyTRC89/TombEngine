#include "framework.h"
#include "Game/Lara/Context.h"

#include "Game/collision/Attractors.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/ContextData.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Internal/TEN/Flow/FlowHandler.h"

using namespace TEN::Collision::Attractors;

// Debug
#include "Renderer/Renderer11.h"
using TEN::Renderer::g_Renderer;
// ---

namespace TEN::Player::Context
{
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
		// 1) Test for slippery slope (if applicable).
		bool isSlipperySlope = setup.TestSlipperySlope ? pointCollFront.Position.FloorSlope : false;
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

	static std::optional<AttractorCollisionData> GetEdgeCatchAttractorCollision(const ItemInfo& item, const CollisionInfo& coll)
	{
		constexpr auto FLOOR_TO_EDGE_HEIGHT_MIN = LARA_HEIGHT_STRETCH;

		auto relOffset = Vector3(0.0f, -coll.Setup.Height, coll.Setup.Radius);
		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
		float radius = std::max<float>(coll.Setup.Radius, item.Animation.Velocity.Length());

		// Get attractor collisions.
		auto refPoint = item.Pose.Position.ToVector3() + Vector3::Transform(relOffset, rotMatrix);
		float detectRadius = OFFSET_RADIUS(radius);
		auto attracColls = GetAttractorCollisions(item, refPoint, detectRadius);

		const AttractorCollisionData* attracCollPtr = nullptr;
		float closestDist = INFINITY;
		bool hasEnd = false;

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Check if attractor is edge type.
			if (!attracColl.Attrac.IsEdge())
				continue;

			// 2) Test if edge slope is slippery.
			if (abs(attracColl.SlopeAngle) >= SLIPPERY_FLOOR_SLOPE_ANGLE)
				continue;

			// 3) Test catch angle.
			if (!attracColl.IsInFront || !attracColl.IsFacingForward ||
				!TestPlayerInteractAngle(item, attracColl.HeadingAngle))
			{
				continue;
			}

			// TODO: Accuracy. Or maybe snap to within bounds, but use offset blending.
			// 4) Test for seam between connecting attractors.
			if (!hasEnd &&
				(attracColl.Proximity.ChainDistance <= EPSILON ||
					(attracColl.Attrac.GetLength() - attracColl.Proximity.ChainDistance) <= EPSILON))
			{
				// Track ends.
				hasEnd = true;

				// 4.1) Test for looped attractor.
				if (!attracColl.Attrac.IsLooped())
					continue;
			}

			// Get point collision off side of edge.
			auto pointColl = GetCollision(
				Vector3i(attracColl.Proximity.Intersection), attracColl.Attrac.GetRoomNumber(),
				attracColl.HeadingAngle, -coll.Setup.Radius);

			// 5) Test if edge is high enough off the ground.
			int floorToEdgeHeight = abs(attracColl.Proximity.Intersection.y - pointColl.Position.Floor);
			if (floorToEdgeHeight <= FLOOR_TO_EDGE_HEIGHT_MIN)
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
				// Track closest attractor.
				if (attracColl.Proximity.Distance < closestDist)
				{
					attracCollPtr = &attracColl;
					closestDist = attracColl.Proximity.Distance;
				}

				continue;
			}
		}

		// No edge found; return nullopt.
		if (attracCollPtr == nullptr)
			return std::nullopt;

		return *attracCollPtr;
	}

	static std::optional<EdgeCatchData> GetLedgeCatchData(const ItemInfo& item, const CollisionInfo& coll)
	{
		const auto& player = GetLaraInfo(item);

		// Get edge catch attractor collision.
		auto attracColl = GetEdgeCatchAttractorCollision(item, coll);
		if (!attracColl.has_value())
			return std::nullopt;

		// TODO: Accuracy.
		// Calculate heading angle.
		auto pointLeft = attracColl->Attrac.GetPointAtChainDistance(attracColl->Proximity.ChainDistance - coll.Setup.Radius);
		auto pointRight = attracColl->Attrac.GetPointAtChainDistance(attracColl->Proximity.ChainDistance + coll.Setup.Radius);
		short headingAngle = Geometry::GetOrientToPoint(pointLeft, pointRight).y - ANGLE(90.0f);

		// Return edge catch data.
		return EdgeCatchData
		{
			&attracColl->Attrac,
			EdgeType::Ledge,
			attracColl->Proximity.Intersection,
			attracColl->Proximity.ChainDistance,
			headingAngle
		};
	}

	static std::optional<EdgeCatchData> GetClimbableWallEdgeCatchData(ItemInfo& item, CollisionInfo& coll)
	{
		constexpr auto WALL_STEP_HEIGHT = CLICK(1);

		const auto& player = GetLaraInfo(item);

		// 1) Check for climbable wall flag.
		if (!player.Control.CanClimbLadder)
			return std::nullopt;

		// 2) Test for valid ledge on climbable wall.
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
			return EdgeCatchData
			{
				nullptr,
				EdgeType::ClimbableWall,
				offset
			};
		}
		
		return std::nullopt;
	}

	std::optional<EdgeCatchData> GetEdgeCatchData(ItemInfo& item, CollisionInfo& coll)
	{
		// 1) Get and return ledge catch data (if valid).
		auto ledgeCatchData = GetLedgeCatchData(item, coll);
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
			return MonkeySwingCatchData{ monkeyHeight };
		}

		return std::nullopt;
	}

	// TODO
	std::optional<ShimmyData> GetShimmyData(const ItemInfo& item, const CollisionInfo& coll)
	{
		return std::nullopt;
	}
}
