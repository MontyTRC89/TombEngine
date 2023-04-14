#include "framework.h"
#include "Game/Lara/lara_tests.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/items.h"
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

using namespace TEN::Floordata;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

struct EdgeHangAttractorCollisionData
{
	std::optional<AttractorCollisionData> Center = {};
	std::optional<AttractorCollisionData> Left	 = {};
	std::optional<AttractorCollisionData> Right	 = {};
};

bool TestPlayerInteractAngle(const ItemInfo& item, short testAngle)
{
	return (abs(short(testAngle - item.Pose.Orientation.y)) <= PLAYER_INTERACT_ANGLE_CONSTRAINT);
}

static std::optional<AttractorCollisionData> GetBestEdgeHangAttractorColl(const ItemInfo& item, const CollisionInfo& coll,
																		  const std::vector<AttractorCollisionData>& attracColls)
{
	const AttractorCollisionData* attracCollPtr = nullptr;
	float closestDist = INFINITY;
	bool hasFoundCorner = false;

	// Assess attractor collision.
	for (const auto& attracColl : attracColls)
	{
		// 1) Check if attractor is edge type.
		if (!attracColl.AttractorPtr->IsEdge())
			continue;

		// 2) Check if edge is within range and in front.
		if (!attracColl.IsIntersected || !attracColl.IsInFront)
			continue;

		// TODO: Test if this works.
		// 3) Test if target point is lone corner.
		if (attracColl.DistanceFromEnd <= EPSILON && !hasFoundCorner)
		{
			hasFoundCorner = true;
			continue;
		}

		// 4) Test if edge slope is slippery.
		if (abs(attracColl.SlopeAngle) >= SLIPPERY_SLOPE_ANGLE)
			continue;

		// Get point collision off side of edge.
		auto pointCollOffSide = GetCollision(
			Vector3i(attracColl.TargetPoint), attracColl.AttractorPtr->GetRoomNumber(),
			attracColl.HeadingAngle, -coll.Setup.Radius);

		// 5) Test if edge is too low to the ground.
		int floorToEdgeHeight = abs(attracColl.TargetPoint.y - pointCollOffSide.Position.Floor);
		if (floorToEdgeHeight <= LARA_HEIGHT_STRETCH)
			continue;

		// 6) Track closest attractor.
		if (attracColl.Distance < closestDist)
		{
			attracCollPtr = &attracColl;
			closestDist = attracColl.Distance;
			continue;
		}
	}

	// No edge found, return nullopt.
	if (attracCollPtr == nullptr)
		return std::nullopt;

	return *attracCollPtr;
}

static EdgeHangAttractorCollisionData GetEdgeHangAttractorCollisionData(const ItemInfo& item, const CollisionInfo& coll)
{
	auto& player = GetLaraInfo(item);

	// Get nearby attractor pointers.
	auto attracPtrs = GetNearbyAttractorPtrs(item);

	auto basePos = item.Pose.Position.ToVector3();
	auto rotMatrix = Matrix::CreateRotationY(TO_RAD(player.Context.TargetOrientation.y));
	float range = OFFSET_RADIUS(coll.Setup.Radius);

	// Get center attractor collision.
	auto relOffsetCenter = Vector3(0.0f, -coll.Setup.Height, coll.Setup.Radius);
	auto refPointCenter = basePos + Vector3::Transform(relOffsetCenter, rotMatrix);
	auto attracCollsCenter = GetAttractorCollisions(item, attracPtrs, refPointCenter, range);
	auto attracCollCenter = GetBestEdgeHangAttractorColl(item, coll, attracCollsCenter);

	// Get left attractor collision.
	auto relOffsetLeft = Vector3(-coll.Setup.Radius, -coll.Setup.Height, coll.Setup.Radius);
	auto refPointLeft = basePos + Vector3::Transform(relOffsetLeft, rotMatrix);
	auto attracCollsLeft = GetAttractorCollisions(item, attracPtrs, refPointLeft, range);
	auto attracCollLeft = GetBestEdgeHangAttractorColl(item, coll, attracCollsLeft);

	// Get right attractor collision.
	auto relOffsetRight = Vector3(coll.Setup.Radius, -coll.Setup.Height, coll.Setup.Radius);
	auto refPointRight = basePos + Vector3::Transform(relOffsetRight, rotMatrix);
	auto attracCollsRight = GetAttractorCollisions(item, attracPtrs, refPointRight, range);
	auto attracCollRight = GetBestEdgeHangAttractorColl(item, coll, attracCollsRight);

	// Return edge attractor collision at three points.
	return EdgeHangAttractorCollisionData{ attracCollCenter, attracCollLeft, attracCollRight };
}

bool HandlePlayerEdgeHang(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	// Get edge attractor collision at three points.
	auto edgeAttracColl = GetEdgeHangAttractorCollisionData(*item, *coll);

	// Check if edge was found.
	if (!edgeAttracColl.Center.has_value())
	{
		SetPlayerEdgeHangRelease(*item);
		return false;
	}
	
	// TODO: Handle correction at attractor ends.
	if (!edgeAttracColl.Left.has_value() ||
		!edgeAttracColl.Right.has_value())
	{
		SetPlayerEdgeHangRelease(*item);
		return false;
	}

	auto orient = Geometry::GetOrientToPoint(edgeAttracColl.Left->TargetPoint, edgeAttracColl.Right->TargetPoint);
	auto headingAngle = orient.y - ANGLE(90.0f);

	// TODO: Works on reflex transition, but not an obtuse one.
	auto targetPoint = edgeAttracColl.Center->TargetPoint;

	// Align orientation.
	player.Context.TargetOrientation = EulerAngles(0, headingAngle, 0);
	item->Pose.Orientation.Lerp(player.Context.TargetOrientation, 0.9f);

	// Align position.
	auto rotMatrix = Matrix::CreateRotationY(TO_RAD(headingAngle));
	auto relOffset = Vector3(0.0f, coll->Setup.Height, -coll->Setup.Radius);
	item->Pose.Position = targetPoint + Vector3::Transform(relOffset, rotMatrix);

	return true;

	// -----------------------------------------

	short moveAngle = player.Control.MoveAngle;

	// Determine direction of player's shimmy. 0 if hanging still.
	int climbDirection = 0;
	if (player.Control.MoveAngle == short(item->Pose.Orientation.y - ANGLE(90.0f)))
	{
		climbDirection = -1;
	}
	else if (player.Control.MoveAngle == short(item->Pose.Orientation.y + ANGLE(90.0f)))
	{
		climbDirection = 1;
	}

	// Temporarily move player slightly closer to wall for more precise collision results.
	auto prevPose = item->Pose;
	TranslateItem(item, item->Pose.Orientation.y, coll->Setup.Radius * 0.5f);

	// Get height difference with side spaces (left or right, depending on movement direction)
	int heightDelta = LaraFloorFront(item, player.Control.MoveAngle, coll->Setup.Radius * 1.4f);

	// Set stopped flag if floor height is above footspace, which is step size.
	bool isStopped = (heightDelta < CLICK(0.5f));

	// Set stopped flag if ceiling height is below headspace, which is step size.
	if (LaraCeilingFront(item, player.Control.MoveAngle, coll->Setup.Radius * 1.5f, 0) > -950)
		isStopped = true;

	// Restore backup pose after collision tests.
	item->Pose = prevPose;

	// Player collision setup.
	player.Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = player.Control.MoveAngle;

	// When player is about to move, use larger embed offset to stabilize diagonal shimmy.
	int embedOffset = 4;
	if (IsHeld(In::Left) || IsHeld(In::Right))
		embedOffset = 32;

	TranslateItem(item, item->Pose.Orientation.y, embedOffset);
	GetCollisionInfo(coll, item);

	bool canHang = false;

	// Ladder case.
	if (player.Control.CanClimbLadder)
	{
		if (IsHeld(In::Action) && item->HitPoints > 0)
		{
			player.Control.MoveAngle = moveAngle;

			if (!TestLaraHangOnClimbableWall(item, coll))
			{
				if (item->Animation.AnimNumber != LA_LADDER_TO_HANG_RIGHT &&
					item->Animation.AnimNumber != LA_LADDER_TO_HANG_LEFT)
				{
					LaraSnapToEdgeOfBlock(item, coll, GetQuadrant(item->Pose.Orientation.y));
					item->Pose.Position.y = coll->Setup.OldPosition.y;
					SetAnimation(item, LA_HANG_IDLE);
				}

				canHang = true;
			}
			else
			{
				if (item->Animation.ActiveState == LS_HANG_IDLE && TestLaraClimbIdle(item, coll))
					item->Animation.TargetState = LS_LADDER_IDLE;
			}
		}
		// Death or Action release. Unused block?
		else
		{
			SetPlayerEdgeHangRelease(*item);
		}
	}
	// Regular case.
	else
	{
		auto pointColl = GetCollision(item, item->Pose.Orientation.y, -coll->Setup.Radius);
		int relFloorHeight = pointColl.Position.Floor - item->Pose.Position.y;

		if ((IsHeld(In::Action) && item->HitPoints > 0 && relFloorHeight >= 0) ||
			(item->Animation.AnimNumber == LA_LEDGE_JUMP_UP_START || item->Animation.AnimNumber == LA_LEDGE_JUMP_BACK_START)) // TODO: Unhardcode this in a later refactor. @Sezz 2022.10.21)
		{
			if (isStopped && heightDelta > 0 && climbDirection != 0 &&
				((climbDirection > 0) == (coll->MiddleLeft.Floor > coll->MiddleRight.Floor)))
			{
				isStopped = false;
			}

			auto verticalShift = coll->Front.Floor - GameBoundingBox(item).Y1;
			int x = item->Pose.Position.x;
			int z = item->Pose.Position.z;

			player.Control.MoveAngle = moveAngle;

			if (climbDirection != 0)
			{
				float sinMoveAngle = phd_sin(player.Control.MoveAngle);
				float cosMoveAngle = phd_cos(player.Control.MoveAngle);
				auto testShift = Vector2(sinMoveAngle, cosMoveAngle) * coll->Setup.Radius;

				x += testShift.x;
				z += testShift.y;
			}

			if (TestLaraNearClimbableWall(item, GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).BottomBlock))
			{
				// Ignore vertical shift if climbable wall is encountered in next block.
				if (!TestLaraHangOnClimbableWall(item, coll))
					verticalShift = 0;
			}
			else if (!TestValidLedge(item, coll, true))
			{
				if ((climbDirection < 0 && coll->FrontLeft.Floor  != coll->Front.Floor) ||
					(climbDirection > 0 && coll->FrontRight.Floor != coll->Front.Floor))
				{
					isStopped = true;
				}
			}

			if (!isStopped &&
				coll->Middle.Ceiling < 0 && coll->CollisionType == CT_FRONT && !coll->HitStatic &&
				abs(verticalShift) < SLOPE_DIFFERENCE && TestPlayerInteractAngle(*item, coll->NearestLedgeAngle))
			{
				if (item->Animation.Velocity.z != 0.0f)
					AlignEntityToEdge(item, coll);

				item->Pose.Position.y += verticalShift;
			}
			else
			{
				item->Pose.Position = coll->Setup.OldPosition;

				// Stop shimmying.
				if (item->Animation.ActiveState == LS_SHIMMY_LEFT ||
					item->Animation.ActiveState == LS_SHIMMY_RIGHT)
				{
					SetAnimation(item, LA_HANG_IDLE);
				}

				// TODO
				/*if (item->Animation.ActiveState == LS_SHIMMY_LEFT)
				{
					SetAnimation(item, LA_SHIMMY_LEFT_TO_IDLE);
				}
				else if (item->Animation.ActiveState == LS_SHIMMY_RIGHT)
				{
					SetAnimation(item, LA_SHIMMY_RIGHT_TO_IDLE);
				}*/

				canHang = true;
			}
		}
		// Death, incorrect ledge, or Action release.
		else
		{
			SetAnimation(item, LA_EDGE_HANG_RELEASE_TO_JUMP_UP);
			item->Animation.IsAirborne = true;
			item->Animation.Velocity = PLAYER_RELEASE_VELOCITY;
			item->Pose.Position += coll->Shift;
			player.Control.HandStatus = HandStatus::Free;
		}
	}

	return canHang;
}

// Test if ledge in front of entity is valid to climb.
bool TestValidLedge(const ItemInfo* item, const CollisionInfo* coll, bool ignoreHeadroom, bool heightLimit)
{
	// Get point collision.
	auto pointCollLeft = GetCollision(item, coll->NearestLedgeAngle - ANGLE(90.0f), coll->Setup.Radius, -coll->Setup.Height);
	auto pointCollRight = GetCollision(item, coll->NearestLedgeAngle + ANGLE(90.0f), coll->Setup.Radius, -coll->Setup.Height);

	//g_Renderer.AddSphere(pointCollLeft.Coordinates.ToVector3(), 20, Vector4::One);
	//g_Renderer.AddSphere(pointCollRight.Coordinates.ToVector3(), 20, Vector4::One);

	// If any front point collision intersects entity bounds, return false, because there is a material intersection.
	// This check helps to filter out cases when the player is formally facing corner but ledge check returns true
	// because probe distance is fixed.
	int vTestPos = item->Pose.Position.y - CLICK(0.5f);
	if (pointCollLeft.Position.Floor < vTestPos ||
		pointCollRight.Position.Floor < vTestPos)
	{
		return false;
	}

	int vPos = item->Pose.Position.y - coll->Setup.Height;
	if (pointCollLeft.Position.Ceiling > vPos ||
		pointCollRight.Position.Ceiling > vPos)
	{
		return false;
	}

	//g_Renderer.AddDebugSphere(Vector3(item->pos.Position.x + xl, left, item->pos.Position.z + zl), 64, Vector4::One, RENDERER_DEBUG_PAGE::LARA_STATS);
	//g_Renderer.AddDebugSphere(Vector3(item->pos.Position.x + xr, right, item->pos.Position.z + zr), 64, Vector4::One, RENDERER_DEBUG_PAGE::LARA_STATS);

	// Determine ledge probe offset.
	// We use 0.2f radius extents here for two purposes. First - we can't guarantee that shifts weren't already applied
	// and misfire may occur. Second - it guarantees the player won't land on a very thin edge of diagonal geometry.

	// Get floor heights at both points.
	int leftHeight = GetCollision(item, coll->NearestLedgeAngle, coll->Setup.Radius * 1.2f, -coll->Setup.Height, -coll->Setup.Radius).Position.Floor;
	int rightHeight = GetCollision(item, coll->NearestLedgeAngle, coll->Setup.Radius * 1.2f, -coll->Setup.Height, coll->Setup.Radius).Position.Floor;

	// limit vertical search range only to nearest height (if specified).
	if (heightLimit)
	{
		if (abs(leftHeight - vPos) > CLICK(0.5f) ||
			abs(rightHeight - vPos) > CLICK(0.5f))
		{
			return false;
		}
	}

	// Test if slope angle is beyond threshold.
	// TODO: Compare slope, aspect, and player angles instead.
	float slopeDelta = ((float)STEPUP_HEIGHT / (float)BLOCK(1)) * (coll->Setup.Radius * 2);
	if (abs(leftHeight - rightHeight) >= slopeDelta)
		return false;

	// Test if ledge is outside distance threshold.
	if (abs(coll->NearestLedgeDistance) > OFFSET_RADIUS(coll->Setup.Radius))
		return false;

	// Test if ledge is within angle threshold.
	if (!TestPlayerInteractAngle(*item, coll->NearestLedgeAngle))
		return false;

	if (!ignoreHeadroom)
	{
		int headroom = (coll->Front.Floor + coll->Setup.Height) - coll->Middle.Ceiling;
		if (headroom < CLICK(1))
			return false;
	}

	return true;
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
		item->Pose.Position.z += coll2.Shift.z;
		break;

	case EAST:
	case WEST:
		item->Pose.Position.x += coll2.Shift.x;
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

bool TestLaraValidHangPosition(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Get incoming ledge height and own Lara's upper bound.
	// First one will be negative while first one is positive.
	// Difference between two indicates difference in height between ledges.
	auto frontFloor = GetCollision(item, lara->Control.MoveAngle, coll->Setup.Radius + CLICK(0.5f), -LARA_HEIGHT).Position.Floor;
	auto laraUpperBound = item->Pose.Position.y - coll->Setup.Height;

	// If difference is above 1/2 click, return false (ledge is out of reach).
	if (abs(frontFloor - laraUpperBound) > CLICK(0.5f))
		return false;

	// Embed Lara into wall to make collision test succeed
	item->Pose.Position.x += phd_sin(item->Pose.Orientation.y) * 8;
	item->Pose.Position.z += phd_cos(item->Pose.Orientation.y) * 8;

	// Setup new GCI call
	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -CLICK(2);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Mode = CollisionProbeMode::FreeFlat;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;

	GetCollisionInfo(coll, item);

	// Filter out narrow ceiling spaces, no collision cases and statics in front.
	if (coll->Middle.Ceiling >= 0 || coll->CollisionType != CT_FRONT || coll->HitStatic)
		return false;

	// Finally, do ordinary ledge checks (slope difference etc.)
	return TestValidLedge(item, coll);
}

Context::CornerType TestLaraHangCorner(ItemInfo* item, CollisionInfo* coll, short testAngle)
{
	auto* lara = GetLaraInfo(item);

	// Check for hang idle state.
	if (item->Animation.ActiveState != LS_HANG_IDLE)
		return Context::CornerType::None;

	// Check if static is in the way.
	if (coll->HitStatic)
		return Context::CornerType::None;

	// INNER CORNER TESTS

	// Backup previus player data and frontal collision.
	auto prevPose = item->Pose;
	short prevMoveAngle = lara->Control.MoveAngle;

	auto cornerResult = TestItemAtNextCornerPosition(item, coll, testAngle, false);

	// Do further tests only if test angle is equal to resulting edge angle.
	if (cornerResult.Success)
	{
		// Store next position.
		item->Pose = cornerResult.RealPositionResult;
		lara->Context.NextCornerPos.Position = Vector3i(
			item->Pose.Position.x,
			GetCollision(item, item->Pose.Orientation.y, coll->Setup.Radius * 16, -(coll->Setup.Height + CLICK(0.5f))).Position.Floor + LARA_HEIGHT_STRETCH,
			item->Pose.Position.z);
		lara->Context.NextCornerPos.Orientation.y = item->Pose.Orientation.y;
		lara->Control.MoveAngle = item->Pose.Orientation.y;

		item->Pose = cornerResult.ProbeResult;
		auto result = TestLaraValidHangPosition(item, coll);

		// Restore original entity data.
		item->Pose = prevPose;
		lara->Control.MoveAngle = prevMoveAngle;

		if (result)
			return Context::CornerType::Inner;

		if (lara->Control.CanClimbLadder)
		{
			auto& angleSet = testAngle > 0 ? LeftExtRightIntTab : LeftIntRightExtTab;
			if (GetClimbFlags(lara->Context.NextCornerPos.Position.x, item->Pose.Position.y, lara->Context.NextCornerPos.Position.z, item->RoomNumber) & (short)angleSet[GetQuadrant(item->Pose.Orientation.y)])
			{
				// Restore original vertical position for climbable wall tests because
				// there's no need to snap to ledge height in such cases.
				lara->Context.NextCornerPos.Position.y = item->Pose.Position.y;
				return Context::CornerType::Inner;
			}
		}
	}

	// Restore original entity data.
	item->Pose = prevPose;
	lara->Control.MoveAngle = prevMoveAngle;

	// OUTER CORNER TESTS

	// Test for material obstacles blocking outer corner pathway.
	if ((LaraFloorFront(item, item->Pose.Orientation.y + testAngle, coll->Setup.Radius + CLICK(1)) < 0) ||
		(LaraCeilingFront(item, item->Pose.Orientation.y + testAngle, coll->Setup.Radius + CLICK(1), coll->Setup.Height) > 0))
	{
		return Context::CornerType::None;
	}

	// Conduct ray test to check for last chance of ppossible diagonal vs. non-diagonal cases.
	if (!LaraPositionOnLOS(item, item->Pose.Orientation.y + testAngle, coll->Setup.Radius + CLICK(1)))
	{
		return Context::CornerType::None;
	}

	cornerResult = TestItemAtNextCornerPosition(item, coll, testAngle, true);

	// Additional test for material obstacles blocking outer corner pathway.
	if ((LaraFloorFront(item, item->Pose.Orientation.y, 0) < 0) ||
		(LaraCeilingFront(item, item->Pose.Orientation.y, 0, coll->Setup.Height) > 0))
	{
		cornerResult.Success = false;
	}

	// Do further tests only if test angle is equal to resulting edge angle.
	if (cornerResult.Success)
	{
		// Store next position.
		item->Pose = cornerResult.RealPositionResult;
		lara->Context.NextCornerPos.Position = Vector3i(
			item->Pose.Position.x,
			GetCollision(item, item->Pose.Orientation.y, coll->Setup.Radius * 1.25f, -(LARA_HEIGHT_STRETCH + LARA_HEADROOM)).Position.Floor + LARA_HEIGHT_STRETCH,
			item->Pose.Position.z);
		lara->Context.NextCornerPos.Orientation.y = item->Pose.Orientation.y;
		lara->Control.MoveAngle = item->Pose.Orientation.y;
		lara->Context.TargetOrientation = lara->Context.NextCornerPos.Orientation;

		item->Pose = cornerResult.ProbeResult;
		auto result = TestLaraValidHangPosition(item, coll);

		// Restore original entity data.
		item->Pose = prevPose;
		lara->Control.MoveAngle = prevMoveAngle;

		if (result)
			return Context::CornerType::Outer;

		if (lara->Control.CanClimbLadder)
		{
			auto& angleSet = testAngle > 0 ? LeftIntRightExtTab : LeftExtRightIntTab;
			if (GetClimbFlags(lara->Context.NextCornerPos.Position.x, item->Pose.Position.y, lara->Context.NextCornerPos.Position.z, item->RoomNumber) & (short)angleSet[GetQuadrant(item->Pose.Orientation.y)])
			{
				// Restore original vertical position for climbable wall tests because
				// there's no need to snap to ledge height in such cases.
				lara->Context.NextCornerPos.Position.y = item->Pose.Position.y;
				return Context::CornerType::Outer;
			}
		}
	}

	// Restore original entity data.
	item->Pose = prevPose;
	lara->Control.MoveAngle = prevMoveAngle;

	return Context::CornerType::None;
}

Context::CornerShimmyData TestItemAtNextCornerPosition(ItemInfo* item, CollisionInfo* coll, short testAngle, bool isOuter)
{
	auto* lara = GetLaraInfo(item);

	// Determine real turning angle.
	short turnAngle = isOuter ? testAngle : -testAngle;

	// Backup previous pose into array.
	Pose poses[3] = { item->Pose, item->Pose, item->Pose };

	// Do a two-step rotation check. First step is real resulting position, and second step is probing
	// position. We need this because checking at exact ending position does not always return
	// correct results with nearest ledge angle.

	for (int i = 0; i < 2; i++)
	{
		// Determine collision box anchor point and rotate collision box around this anchor point.
		// Then determine new test position from centerpoint of new collision box position.

		// Move entity back slightly to compensate for possible edge ledge cases.
		float radiusCoeff = isOuter ? -0.2f : 0.2f;
		poses[i].Translate(poses[i].Orientation.y, -(coll->Setup.Radius * radiusCoeff));

		// Move entity at distance of full collision diameter + half radius margin to movement direction.
		radiusCoeff = (i == 0) ? 2.0f : 2.5f;
		poses[i].Translate(lara->Control.MoveAngle, coll->Setup.Radius * radiusCoeff);

		// Determine anchor point.
		short someAngle = ANGLE(90.0f * -std::copysign(1, testAngle));
		auto anchor = Geometry::TranslatePoint(poses[i].Position, poses[i].Orientation.y, coll->Setup.Radius);
		anchor = Geometry::TranslatePoint(anchor, poses[i].Orientation.y + someAngle, coll->Setup.Radius);

		// Determine distance from anchor point to new entity position.
		auto dist = Vector2(poses[i].Position.x, poses[i].Position.z) - Vector2(anchor.x, anchor.z);
		float sinTurnAngle = phd_sin(turnAngle);
		float cosTurnAngle = phd_cos(turnAngle);

		// Move entity to new anchor point.
		poses[i].Position.x = (dist.x * cosTurnAngle) - (dist.y * sinTurnAngle) + anchor.x;
		poses[i].Position.z = (dist.x * sinTurnAngle) + (dist.y * cosTurnAngle) + anchor.z;

		// Virtually rotate entity to new angle.
		short newAngle = poses[i].Orientation.y - turnAngle;
		poses[i].Orientation.y = newAngle;

		// Snap to nearest ledge, if any.
		item->Pose = poses[i];
		AlignEntityToEdge(item, coll, item->Pose.Orientation.y);

		// Copy resulting position to array and restore original entity position.
		poses[i] = item->Pose;
		item->Pose = poses[2];

		//g_Renderer.AddSphere(poses[i].Position.ToVector3(), 20, Vector4::One);

		// Both passes finished; construct result data.
		if (i == 1)
		{
			bool success = (newAngle == poses[i].Orientation.y);
			return Context::CornerShimmyData{ success, poses[0], poses[1] };
		}
	}

	return Context::CornerShimmyData{};
}

bool TestLaraHangSideways(ItemInfo* item, CollisionInfo* coll, short testAngle)
{
	constexpr auto SIDEWAY_SHIMMY_TEST_DIST = 16;

	auto& player = GetLaraInfo(*item);

	auto prevPose = item->Pose;
	player.Control.MoveAngle = item->Pose.Orientation.y + testAngle;
	TranslateItem(item, player.Control.MoveAngle, SIDEWAY_SHIMMY_TEST_DIST);
	coll->Setup.OldPosition.y = item->Pose.Position.y;

	bool canHang = HandlePlayerEdgeHang(item, coll);
	item->Pose = prevPose;
	return !canHang;
}

bool TestLaraWall(ItemInfo* item, int distance, int height, int side)
{
	float s = phd_sin(item->Pose.Orientation.y);
	float c = phd_cos(item->Pose.Orientation.y);

	auto start = GameVector(
		item->Pose.Position.x + (side * c),
		item->Pose.Position.y + height,
		item->Pose.Position.z + (-side * s),
		item->RoomNumber);

	auto end = GameVector(
		item->Pose.Position.x + (distance * s) + (side * c),
		item->Pose.Position.y + height,
		item->Pose.Position.z + (distance * c) + (-side * s),
		item->RoomNumber);

	return !LOS(&start, &end);
}

bool TestLaraFacingCorner(ItemInfo* item, short angle, int distance)
{
	short angleLeft = angle - ANGLE(15.0f);
	short angleRight = angle + ANGLE(15.0f);

	auto origin = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z,
		item->RoomNumber);

	auto target0 = GameVector(
		item->Pose.Position.x + distance * phd_sin(angleLeft),
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z + distance * phd_cos(angleLeft),
		item->RoomNumber);

	auto target1 = GameVector(
		item->Pose.Position.x + distance * phd_sin(angleRight),
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z + distance * phd_cos(angleRight),
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

bool TestLaraWaterStepOut(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (coll->CollisionType == CT_FRONT ||
		coll->Middle.FloorSlope ||
		coll->Middle.Floor >= 0)
	{
		return false;
	}

	if (coll->Middle.Floor >= -CLICK(0.5f))
		SetAnimation(item, LA_STAND_IDLE);
	else
	{
		SetAnimation(item, LA_ONWATER_TO_WADE_1_STEP);
		item->Animation.TargetState = LS_IDLE;
	}

	item->Pose.Position.y += coll->Middle.Floor + CLICK(2.75f) - 9;
	UpdateLaraRoom(item, -(STEPUP_HEIGHT - 3));

	item->Pose.Orientation.x = 0;
	item->Pose.Orientation.z = 0;
	item->Animation.Velocity.z = 0;
	item->Animation.Velocity.y = 0;
	item->Animation.IsAirborne = false;
	lara->Control.WaterStatus = WaterStatus::Wade;

	return true;
}

bool TestLaraWaterClimbOut(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (coll->CollisionType != CT_FRONT || !(TrInput & IN_ACTION))
		return false;

	if (lara->Control.HandStatus != HandStatus::Free &&
		(lara->Control.HandStatus != HandStatus::WeaponReady || lara->Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return false;
	}

	if (coll->Middle.Ceiling > -STEPUP_HEIGHT)
		return false;

	int frontFloor = coll->Front.Floor + LARA_HEIGHT_TREAD;
	if (frontFloor <= -CLICK(2) ||
		frontFloor > CLICK(1.25f) - 4)
	{
		return false;
	}

	if (!TestValidLedge(item, coll))
		return false;

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

	if (!(TrInput & IN_ACTION) || !lara->Control.CanClimbLadder || coll->CollisionType != CT_FRONT)
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
		item->Pose.Position.z = (item->Pose.Position.z & -SECTOR(1)) + LARA_RADIUS + 1;
		break;

	case WEST:
		item->Pose.Position.x = (item->Pose.Position.x & -SECTOR(1)) + LARA_RADIUS + 1;
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
	auto* lara = GetLaraInfo(item);

	auto probe = GetCollision(item);
	int waterDepth = GetWaterDepth(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, probe.RoomNumber);

	if (waterDepth == NO_HEIGHT)
	{
		item->Animation.Velocity.y = 0;
		item->Pose.Position = coll->Setup.OldPosition;
	}
	// Height check was at CLICK(2) before, but changed to this 
	// because now player surfaces at head level, not mid-body level.
	else if (waterDepth <= (LARA_HEIGHT - LARA_HEADROOM / 2))
	{
		SetAnimation(item, LA_UNDERWATER_TO_STAND);
		item->Animation.TargetState = LS_IDLE;
		item->Pose.Position.y = probe.Position.Floor;
		item->Pose.Orientation.x = 0;
		item->Pose.Orientation.z = 0;
		item->Animation.IsAirborne = false;
		item->Animation.Velocity = Vector3::Zero;
		lara->Control.WaterStatus = WaterStatus::Wade;
	}
}

#ifndef NEW_TIGHTROPE
void GetTightropeFallOff(ItemInfo* item, int regularity)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0 || item->HitStatus)
		SetAnimation(item, LA_TIGHTROPE_FALL_LEFT);

	if (!lara->Control.Tightrope.Fall && !(GetRandomControl() & regularity))
		lara->Control.Tightrope.Fall = 2 - ((GetRandomControl() & 0xF) != 0);
}
#endif

bool TestLaraWeaponType(LaraWeaponType refWeaponType, const std::vector<LaraWeaponType>& weaponTypeList)
{
	for (const auto& weaponType : weaponTypeList)
	{
		if (weaponType == refWeaponType)
			return true;
	}

	return false;
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

bool IsStandingWeapon(ItemInfo* item, LaraWeaponType weaponType)
{
	return (TestLaraWeaponType(weaponType, StandingWeaponTypes) || GetLaraInfo(item)->Weapons[(int)weaponType].HasLasersight);
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
	static const std::vector<int> runningJumpQueuableStates
	{
		LS_RUN_FORWARD,
		LS_SPRINT,
		LS_STEP_UP,
		LS_STEP_DOWN
	};
	return TestState(state, runningJumpQueuableStates);
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

bool TestLaraTurn180(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.WaterStatus == WaterStatus::Wade || TestEnvironment(ENV_FLAG_SWAMP, item))
		return true;

	return false;
}

bool TestLaraPose(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return false;

	if (!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		lara->Control.HandStatus == HandStatus::Free &&				// Hands are free.
		(lara->Control.Weapon.GunType != LaraWeaponType::Flare ||	// Flare is not being handled.
			lara->Flare.Life) &&
		lara->Vehicle == NO_ITEM)									// Not in a vehicle.
	{
		return true;
	}

	return false;
}

bool TestLaraKeepLow(ItemInfo* item, CollisionInfo* coll)
{
	// HACK: coll->Setup.Radius is only set to LARA_RADIUS_CRAWL
	// in the collision function, then reset by LaraAboveWater().
	// For tests called in control functions, then, it will store the wrong radius. @Sezz 2021.11.05
	int radius = (item->Animation.ActiveState == LS_CROUCH_IDLE ||
		item->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
		item->Animation.ActiveState == LS_CROUCH_TURN_RIGHT)
		? LARA_RADIUS_CRAWL : LARA_RADIUS;

	auto probeFront = GetCollision(item, item->Pose.Orientation.y, radius, -coll->Setup.Height);
	auto probeBack = GetCollision(item, item->Pose.Orientation.y + ANGLE(180.0f), radius, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item, 0.0f, 0.0f, -LARA_HEIGHT / 2);

	// Assess middle.
	if (abs(probeMiddle.Position.Ceiling - probeMiddle.Position.Floor) < LARA_HEIGHT ||	// Middle space is low enough.
		abs(coll->Middle.Ceiling - LARA_HEIGHT_CRAWL) < LARA_HEIGHT)					// Consider statics overhead detected by GetCollisionInfo().
	{
		return true;
	}

	// TODO: Check whether < or <= and > or >=.

	// Assess front.
	if (abs(probeFront.Position.Ceiling - probeFront.Position.Floor) < LARA_HEIGHT &&		// Front space is low enough.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > LARA_HEIGHT_CRAWL && // Front space not a clamp.
		abs(probeFront.Position.Floor - probeMiddle.Position.Floor) <= (CLICK(1) - 1) &&	// Front is withing upper/lower floor bounds.
		probeFront.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	// Assess back.
	if (abs(probeBack.Position.Ceiling - probeBack.Position.Floor) < LARA_HEIGHT &&		  // Back space is low enough.
		abs(probeBack.Position.Ceiling - probeBack.Position.Floor) > LARA_HEIGHT_CRAWL && // Back space not a clamp.
		abs(probeBack.Position.Floor - probeMiddle.Position.Floor) <= (CLICK(1) - 1) &&	  // Back is withing upper/lower floor bounds.
		probeBack.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraSlide(ItemInfo* item, CollisionInfo* coll)
{
	int yPos = item->Pose.Position.y;
	auto pointColl = GetCollision(item, 0, 0, -(coll->Setup.Height / 2));

	if (abs(pointColl.Position.Floor - yPos) <= STEPUP_HEIGHT &&
		pointColl.Position.FloorSlope &&
		!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		return true;
	}

	return false;
}

bool TestLaraLand(ItemInfo* item, CollisionInfo* coll)
{
	int heightFromFloor = GetCollision(item).Position.Floor - item->Pose.Position.y;

	if (item->Animation.IsAirborne && item->Animation.Velocity.y >= 0 &&
		(heightFromFloor <= item->Animation.Velocity.y ||
			TestEnvironment(ENV_FLAG_SWAMP, item)))
	{
		return true;
	}
	
	return false;
}

bool TestLaraFall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int yPos = item->Pose.Position.y;
	auto pointColl = GetCollision(item, 0, 0, -(coll->Setup.Height / 2));

	if ((pointColl.Position.Floor - yPos) <= STEPUP_HEIGHT ||
		lara->Control.WaterStatus == WaterStatus::Wade)	// TODO: This causes a legacy floor snap bug when Lara wades off a ledge into a dry room. @Sezz 2021.09.26
	{
		return false;
	}

	return true;
}

bool TestLaraMonkeyFall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	auto probe = GetCollision(item);

	if (!lara->Control.CanMonkeySwing ||					// No monkey sector.
		(probe.Position.Ceiling - y) > CLICK(1.25f) ||		// Outside lower bound.
		(probe.Position.Ceiling - y) < -CLICK(1.25f) ||		// Outside upper bound.
		(probe.Position.CeilingSlope &&						// Is ceiling slope (if applicable).
			!g_GameFlow->HasOverhangClimb()) ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraStep(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (abs(coll->Middle.Floor) > 0 &&
		(coll->Middle.Floor <= STEPUP_HEIGHT ||					// Within lower floor bound...
			lara->Control.WaterStatus == WaterStatus::Wade) &&		// OR Lara is wading.
		coll->Middle.Floor >= -STEPUP_HEIGHT &&					// Within upper floor bound.
		coll->Middle.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraStepUp(ItemInfo* item, CollisionInfo* coll)
{
	if (coll->Middle.Floor < -CLICK(0.5f) &&	// Within lower floor bound.
		coll->Middle.Floor >= -STEPUP_HEIGHT)	// Within upper floor bound.
	{
		return true;
	}

	return false;
}

bool TestLaraStepDown(ItemInfo* item, CollisionInfo* coll)
{
	if (coll->Middle.Floor <= STEPUP_HEIGHT &&	// Within lower floor bound.
		coll->Middle.Floor > CLICK(0.5f))		// Within upper floor bound.
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll)
{
	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	auto probe = GetCollision(item);

	if ((probe.Position.Ceiling - y) <= CLICK(1.25f) &&		// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(1.25f) &&	// Within upper ceiling bound.
		probe.Position.Ceiling != NO_HEIGHT)
	{
		return true;
	}

	return false;
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

bool TestLaraRunForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in run state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y,
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, true, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWalkForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in walk state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y,
		STEPUP_HEIGHT, -STEPUP_HEIGHT
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWalkBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in walk back state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(180.0f),
		STEPUP_HEIGHT, -STEPUP_HEIGHT
	};

	return TestLaraMoveTolerance(item, coll, testSetup);	
}

bool TestLaraRunBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in hop back state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(180.0f),
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepLeft(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in step left state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y - ANGLE(90.0f),
		int(CLICK(0.8f)), int(-CLICK(0.8f))
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepRight(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in step right state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(90.0f),
		int(CLICK(0.8f)), int(-CLICK(0.8f))
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWadeForwardSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in wade forward state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y,
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWalkBackSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using UpperFloorBound defined in walk back state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(180.0f),
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepLeftSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using UpperFloorBound defined in step left state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y - ANGLE(90.0f),
		NO_LOWER_BOUND, int(-CLICK(0.8f)),
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepRightSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using UpperFloorBound defined in step right state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(90.0f),
		NO_LOWER_BOUND, int(-CLICK(0.8f)),
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraCrawlForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in crawl state collision functions.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y,
		CLICK(1) - 1, -(CLICK(1) - 1)
	};

	return TestLaraMoveTolerance(item, coll, testSetup, true);
}

bool TestLaraCrawlBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in crawl state collision functions.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(180.0f),
		CLICK(1) - 1, -(CLICK(1) - 1)
	};

	return TestLaraMoveTolerance(item, coll, testSetup, true);
}

bool TestLaraCrouchRoll(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Assess water depth.
	if (lara->WaterSurfaceDist < -CLICK(1))
		return false;

	// Assess continuity of path.
	int distance = 0;
	auto probeA = GetCollision(item);
	while (distance < SECTOR(1))
	{
		distance += CLICK(1);
		auto probeB = GetCollision(item, item->Pose.Orientation.y, distance, -LARA_HEIGHT_CRAWL);

		if (abs(probeA.Position.Floor - probeB.Position.Floor) > (CLICK(1) - 1) ||			// Ensure floor height difference is within a threshold.
			abs(probeB.Position.Ceiling - probeB.Position.Floor) <= LARA_HEIGHT_CRAWL ||	// Avoid clamps.
			probeB.Position.FloorSlope)														// Avoid slopes.
		{
			return false;
		}

		probeA = probeB;
	}

	return true;
}

bool TestLaraCrouch(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if ((lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(item, lara->Control.Weapon.GunType)) &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchToCrawl(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		lara->Control.HandStatus == HandStatus::Free &&				// Hands are free.
		(lara->Control.Weapon.GunType != LaraWeaponType::Flare ||	// Not handling flare. TODO: Should be allowed, but the flare animation bugs out right now. @Sezz 2022.03.18
			lara->Flare.Life))
	{
		return true;
	}

	return false;
}

bool TestLaraFastTurn(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if ((lara->Control.HandStatus == HandStatus::WeaponReady && lara->Control.Weapon.GunType != LaraWeaponType::Torch) ||
		(lara->Control.HandStatus == HandStatus::WeaponDraw && lara->Control.Weapon.GunType != LaraWeaponType::Flare))
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

VaultTestResult TestLaraVaultTolerance(ItemInfo* item, CollisionInfo* coll, VaultTestSetup testSetup)
{
	auto* lara = GetLaraInfo(item);

	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
	bool swampTooDeep = testSetup.CheckSwampDepth ? (isSwamp && lara->WaterSurfaceDist < -CLICK(3)) : isSwamp;
	int y = isSwamp ? item->Pose.Position.y : probeMiddle.Position.Floor; // HACK: Avoid cheese when in the midst of performing a step. Can be done better. @Sezz 2022.04.08	

	// Check swamp depth (if applicable).
	if (swampTooDeep)
		return VaultTestResult{ false };

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
		return VaultTestResult{ false };

	// Assess point/room collision.
	if ((probeFront.Position.Floor - y) < testSetup.LowerFloorBound &&							// Within lower floor bound.
		(probeFront.Position.Floor - y) >= testSetup.UpperFloorBound &&							// Within upper floor bound.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMin &&	// Within clamp min.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMax &&	// Within clamp max.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= testSetup.GapMin)		// Gap is optically permissive.
	{
		return VaultTestResult{ true, probeFront.Position.Floor };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraVault2Steps(ItemInfo* item, CollisionInfo* coll)
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
	testResult.Height += CLICK(2);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault3Steps(ItemInfo* item, CollisionInfo* coll)
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
	testResult.Height += CLICK(3);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault1StepToCrouch(ItemInfo* item, CollisionInfo* coll)
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
	testResult.Height += CLICK(1);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault2StepsToCrouch(ItemInfo* item, CollisionInfo* coll)
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
	testResult.Height += CLICK(2);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault3StepsToCrouch(ItemInfo* item, CollisionInfo* coll)
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
	testResult.Height += CLICK(3);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraLedgeAutoJump(ItemInfo* item, CollisionInfo* coll)
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
	testResult.SetBusyHands = false;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = true;
	return testResult;
}

VaultTestResult TestLaraLadderAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Check ledge angle.
	if (!TestPlayerInteractAngle(*item, coll->NearestLedgeAngle))
		return VaultTestResult{ false };

	if (lara->Control.CanClimbLadder &&								// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(6.5f) &&		// Within lowest middle ceiling bound. (Synced with TestLaraLadderMount())
		((probeFront.Position.Floor - y) <= -CLICK(6.5f) ||			// Floor height is appropriate, OR
			(probeFront.Position.Ceiling - y) > -CLICK(6.5f)) &&		// Ceiling height is appropriate. (Synced with TestLaraLadderMount())
		coll->NearestLedgeDistance <= coll->Setup.Radius)			// Appropriate distance from wall.
	{
		return VaultTestResult{ true, probeMiddle.Position.Ceiling, false, true, true };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraLadderMount(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Check ledge angle.
	if (!TestPlayerInteractAngle(*item, coll->NearestLedgeAngle))
		return VaultTestResult{ false };

	if (lara->Control.CanClimbLadder &&							// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(4.5f) &&	// Within lower middle ceiling bound.
		(probeMiddle.Position.Ceiling - y) > -CLICK(6.5f) &&	// Within upper middle ceiling bound.
		(probeMiddle.Position.Floor - y) > -CLICK(6.5f) &&		// Within upper middle floor bound. (Synced with TestLaraAutoJump())
		(probeFront.Position.Ceiling - y) <= -CLICK(4.5f) &&	// Within lowest front ceiling bound.
		coll->NearestLedgeDistance <= coll->Setup.Radius)		// Appropriate distance from wall.
	{
		return VaultTestResult{ true, NO_HEIGHT, true, true, false };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraMonkeyAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item);

	if (lara->Control.CanMonkeySwing &&							// Monkey swing sector flag set.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_MONKEY &&	// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(7))				// Within upper ceiling bound.
	{
		return VaultTestResult{ true, probe.Position.Ceiling, false, false, true };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraVault(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) || lara->Control.HandStatus != HandStatus::Free)
		return VaultTestResult{ false };

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && lara->WaterSurfaceDist < -CLICK(3))
		return VaultTestResult{ false };

	VaultTestResult vaultResult;

	// Attempt ledge vault.
	if (TestValidLedge(item, coll))
	{
		// Vault to crouch up one step.
		vaultResult = TestLaraVault1StepToCrouch(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_VAULT_1_STEP_CROUCH;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to stand up two steps.
		vaultResult = TestLaraVault2Steps(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_VAULT_2_STEPS;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to crouch up two steps.
		vaultResult = TestLaraVault2StepsToCrouch(item, coll);
		if (vaultResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			vaultResult.TargetState = LS_VAULT_2_STEPS_CROUCH;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to stand up three steps.
		vaultResult = TestLaraVault3Steps(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_VAULT_3_STEPS;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to crouch up three steps.
		vaultResult = TestLaraVault3StepsToCrouch(item, coll);
		if (vaultResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			vaultResult.TargetState = LS_VAULT_3_STEPS_CROUCH;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Auto jump to ledge.
		vaultResult = TestLaraLedgeAutoJump(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_AUTO_JUMP;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}
	}

	// TODO: Move ladder checks here when ladders are less prone to breaking.
	// In this case, they fail due to a reliance on ShiftItem(). @Sezz 2021.02.05

	// Auto jump to monkey swing.
	vaultResult = TestLaraMonkeyAutoJump(item, coll);
	if (vaultResult.Success &&
		g_GameFlow->HasMonkeyAutoJump())
	{
		vaultResult.TargetState = LS_AUTO_JUMP;
		vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
		return vaultResult;
	}
	
	return VaultTestResult{ false };
}

// Temporary solution to ladder mounts until ladders stop breaking whenever anyone tries to do anything with them. -- Sezz 2022.02.05
bool TestAndDoLaraLadderClimb(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_FORWARD) || lara->Control.HandStatus != HandStatus::Free)
		return false;

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && lara->WaterSurfaceDist < -CLICK(3))
		return false;

	// Auto jump to ladder.
	auto vaultResult = TestLaraLadderAutoJump(item, coll);
	if (vaultResult.Success)
	{
		// TODO: Somehow harmonise CalculatedJumpVelocity to work for both ledge and ladder auto jumps, because otherwise there will be a need for an odd workaround in the future.
		lara->Context.CalcJumpVelocity = -3 - sqrt(-9600 - 12 * std::max((vaultResult.Height - item->Pose.Position.y + CLICK(0.2f)), -CLICK(7.1f)));
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
	if (vaultResult.Success &&
		TestLaraClimbIdle(item, coll))
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
	if (!(TrInput & (IN_ACTION | IN_JUMP)))
		return CrawlVaultTestResult{ false };

	// Crawl vault exit down 1 step.
	auto crawlVaultResult = TestLaraCrawlExitDownStep(item, coll);
	if (crawlVaultResult.Success)
	{
		if (TrInput & IN_CROUCH && TestLaraCrawlDownStep(item, coll).Success)
			crawlVaultResult.TargetState = LS_CRAWL_STEP_DOWN;
		else [[likely]]
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_STEP_DOWN;

		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault exit jump.
	crawlVaultResult = TestLaraCrawlExitJump(item, coll);
	if (crawlVaultResult.Success)
	{
		if (TrInput & IN_WALK)
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_FLIP;
		else [[likely]]
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

bool TestLaraJumpTolerance(ItemInfo* item, CollisionInfo* coll, JumpTestSetup testSetup)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item, testSetup.Angle, testSetup.Distance, -coll->Setup.Height);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
	bool isWading = testSetup.CheckWadeStatus ? (lara->Control.WaterStatus == WaterStatus::Wade) : false;

	// Discard walls.
	if (probe.Position.Floor == NO_HEIGHT)
		return false;

	// Check for swamp or wade status (if applicable).
	if (isSwamp || isWading)
		return false;

	// Assess point/room collision.
	if (!TestLaraFacingCorner(item, testSetup.Angle, testSetup.Distance) &&					// Avoid jumping through corners.
		(probe.Position.Floor - y) >= -STEPUP_HEIGHT &&										// Within highest floor bound.
		((probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||	// Within lowest ceiling bound... 
			((probe.Position.Ceiling - y) < -coll->Setup.Height &&								// OR ceiling is level with Lara's head
				(probe.Position.Floor - y) >= CLICK(0.5f))))										// AND there is a drop below.
	{
		return true;
	}

	return false;
}

bool TestLaraRunJumpForward(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.y,
		int(CLICK(1.5f))
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpForward(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.y
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpBack(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(180.0f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpLeft(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.y - ANGLE(90.0f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpRight(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.y + ANGLE(90.0f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpUp(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		0,
		0,
		false
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraSlideJump(ItemInfo* item, CollisionInfo* coll)
{
	return true;

	// TODO: Broken on diagonal slides?
	if (g_GameFlow->HasSlideExtended())
	{
		auto pointColl = GetCollision(item);

		short directionAngle = GetLaraSlideDirection(item, coll);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(GetSurfaceNormal(pointColl.FloorTilt, true));
		return (abs(short(coll->Setup.ForwardAngle - directionAngle)) <= abs(slopeAngle));
	}

	return true;
}

bool TestLaraCrawlspaceDive(ItemInfo* item, CollisionInfo* coll)
{
	auto probe = GetCollision(item, coll->Setup.ForwardAngle, coll->Setup.Radius, -coll->Setup.Height);
	
	if (abs(probe.Position.Ceiling - probe.Position.Floor) < LARA_HEIGHT ||
		TestLaraKeepLow(item, coll))
	{
		return true;
	}

	return false;
}

bool TestLaraLedgeJump(ItemInfo* item, CollisionInfo* coll)
{
	static const int minLedgeHeight = CLICK(2);

	if (!g_GameFlow->HasLedgeJumps())
		return false;

	int vPos = item->Pose.Position.y - coll->Setup.Height;
	auto pointColl = GetCollision(item);

	// Assess point collision.
	if ((pointColl.Position.Ceiling - vPos) >= -coll->Setup.Height) // Ceiling isn't too low.
		return false;

	auto origin = GameVector(
		item->Pose.Position.x,
		(item->Pose.Position.y - coll->Setup.Height) + minLedgeHeight,
		item->Pose.Position.z,
		item->RoomNumber
	);
	auto target = GameVector(
		Geometry::TranslatePoint(Vector3i(origin.x, origin.y, origin.z), item->Pose.Orientation.y, OFFSET_RADIUS(coll->Setup.Radius)),
		item->RoomNumber
	);

	// Assess ray collision.
	if (LOS(&origin, &target)) // Ledge isn't too thin.
		return false;

	return true;
}

bool TestLaraTightropeDismount(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	auto probe = GetCollision(item);

	if (probe.Position.Floor == item->Pose.Position.y &&
		lara->Control.Tightrope.CanDismount)
	{
		return true;
	}

	return false;
}

bool TestLaraPoleCollision(ItemInfo* item, CollisionInfo* coll, bool goingUp, float offset)
{
	static constexpr auto poleProbeCollRadius = 16.0f;

	bool atLeastOnePoleCollided = false;

	if (GetCollidedObjects(item, SECTOR(1), true, CollidedItems, nullptr, false) &&
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

		//g_Renderer.AddDebugSphere(sphere.Center, 16.0f, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LARA_STATS);

		int i = 0;
		while (CollidedItems[i] != nullptr)
		{
			auto*& object = CollidedItems[i];
			i++;

			if (object->ObjectNumber != ID_POLEROPE)
				continue;

			auto poleBox = GameBoundingBox(object).ToBoundingOrientedBox(object->Pose);
			poleBox.Extents = poleBox.Extents + Vector3(coll->Setup.Radius, 0.0f, coll->Setup.Radius);

			//g_Renderer.AddDebugBox(poleBox, Vector4(0, 0, 1, 1), RENDERER_DEBUG_PAGE::LARA_STATS);

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
