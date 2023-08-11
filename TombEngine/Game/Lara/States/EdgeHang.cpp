#include "framework.h"
#include "Game/Lara/States/EdgeHang.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/Context.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

//------Debug
#include "Renderer/Renderer11.h"
using namespace TEN::Renderer;
//------

namespace TEN::Player
{
	// notes:
	// 1. get shimmy context data (ShimmyData with attractor collisions).
	// 2. pass to edge movement handler?

	struct EdgeHangAttractorCollisionData
	{
		AttractorCollisionData Center;
		AttractorCollisionData Left;
		AttractorCollisionData Right;
	};

	static std::optional<AttractorCollisionData> GetConnectingEdgeAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																					 const Attractor& currentAttrac, const Vector3& refPoint)
	{
		constexpr auto CONNECT_DIST_THRESHOLD = BLOCK(1 / 64.0f);

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(item, refPoint, CONNECT_DIST_THRESHOLD);

		const AttractorCollisionData* attracCollPtr = nullptr;
		float closestDist = INFINITY;

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1. Check if attractor is new.
			if (&attracColl.Attrac == &currentAttrac)
				continue;

			// 2) Check if attractor is edge type.
			if (!attracColl.Attrac.IsEdge())
				continue;

			// Track closest attractor.
			if (attracColl.Proximity.Distance < closestDist)
			{
				attracCollPtr = &attracColl;
				closestDist = attracColl.Proximity.Distance;
			}
		}

		if (attracCollPtr == nullptr)
			return std::nullopt;

		return *attracCollPtr;
	}

	static std::optional<EdgeHangAttractorCollisionData> GetEdgeHangAttractorCollisions(const ItemInfo& item, const CollisionInfo& coll,
																						float sideOffset = 0.0f)
	{
		const auto& player = GetLaraInfo(item);
		const auto& handsAttrac = player.Context.HandsAttractor;

		const auto& points = handsAttrac.AttracPtr->GetPoints();
		float length = handsAttrac.AttracPtr->GetLength();

		// Calculate projected distances along attractor.
		float chainDistCenter = handsAttrac.ChainDistance + sideOffset;
		float chainDistLeft = chainDistCenter - coll.Setup.Radius;
		float chainDistRight = chainDistCenter + coll.Setup.Radius;

		bool isLooped = handsAttrac.AttracPtr->IsLooped();

		// TODO: Horrible, organise later.
		// Get connecting attractors just in case.
		auto connectingAttracCollCenter = GetConnectingEdgeAttractorCollision(item, coll, *player.Context.HandsAttractor.AttracPtr, (chainDistCenter <= 0.0f) ? points.front() : points.back());
		auto connectingAttracCollLeft = GetConnectingEdgeAttractorCollision(item, coll, *player.Context.HandsAttractor.AttracPtr, points.front());
		auto connectingAttracCollRight = GetConnectingEdgeAttractorCollision(item, coll, *player.Context.HandsAttractor.AttracPtr, points.back());

		// Get points.
		auto pointCenter = handsAttrac.AttracPtr->GetPointAtChainDistance(chainDistCenter);
		if (!isLooped)
		{
			if (connectingAttracCollCenter.has_value())
			{
				// Get point at connecting attractor.
				if (chainDistCenter <= 0.0f)
				{
					float transitLineDist = connectingAttracCollCenter->Proximity.ChainDistance + chainDistCenter;
					pointCenter = connectingAttracCollCenter->Attrac.GetPointAtChainDistance(transitLineDist);
				}
				else if (chainDistCenter >= length)
				{
					float transitLineDist = connectingAttracCollCenter->Proximity.ChainDistance + (chainDistCenter - length);
					pointCenter = connectingAttracCollCenter->Attrac.GetPointAtChainDistance(transitLineDist);
				}
			}
			else
			{
				// Get point within boundary of current attractor.
				if (chainDistLeft <= 0.0f && !connectingAttracCollLeft.has_value())
				{
					pointCenter = handsAttrac.AttracPtr->GetPointAtChainDistance(coll.Setup.Radius);
				}
				else if (chainDistRight >= length && !connectingAttracCollRight.has_value())
				{
					pointCenter = handsAttrac.AttracPtr->GetPointAtChainDistance(length - coll.Setup.Radius);
				}

				// TODO: Unreachable segments on current attractor. Too steep, angle difference to great.
			}
		}

		auto pointLeft = ((chainDistLeft <= 0.0f) && !isLooped && connectingAttracCollLeft.has_value()) ?
			connectingAttracCollLeft->Attrac.GetPointAtChainDistance(connectingAttracCollLeft->Proximity.ChainDistance + chainDistLeft) :
			handsAttrac.AttracPtr->GetPointAtChainDistance(chainDistLeft);
		auto pointRight = ((chainDistRight >= length) && !isLooped && connectingAttracCollRight.has_value()) ?
			connectingAttracCollRight->Attrac.GetPointAtChainDistance(connectingAttracCollRight->Proximity.ChainDistance + (chainDistRight - length)) :
			handsAttrac.AttracPtr->GetPointAtChainDistance(chainDistRight);

		auto basePos = item.Pose.Position.ToVector3();
		auto orient = item.Pose.Orientation;

		// Get attractor collisions.
		auto attracCollCenter = ((chainDistCenter <= 0.0f || chainDistCenter >= length) && connectingAttracCollCenter.has_value()) ?
			connectingAttracCollCenter->Attrac.GetCollision(basePos, orient, pointCenter) :
			handsAttrac.AttracPtr->GetCollision(basePos, orient, pointCenter);

		auto attracCollLeft = ((chainDistLeft <= 0.0f) && !isLooped && connectingAttracCollLeft.has_value()) ?
			connectingAttracCollLeft->Attrac.GetCollision(basePos, orient, pointLeft) :
			handsAttrac.AttracPtr->GetCollision(basePos, orient, pointLeft);
		auto attracCollRight = ((chainDistRight >= length) && !isLooped && connectingAttracCollRight.has_value()) ?
			connectingAttracCollRight->Attrac.GetCollision(basePos, orient, pointLeft) :
			handsAttrac.AttracPtr->GetCollision(basePos, orient, pointRight);

		// ----------Debug
		constexpr auto COLOR_MAGENTA = Vector4(1, 0, 1, 1);
		g_Renderer.AddLine3D(attracCollCenter.Proximity.IntersectPoint, attracCollCenter.Proximity.IntersectPoint + Vector3(0.0f, -150.0f, 0.0f), COLOR_MAGENTA);
		g_Renderer.AddLine3D(attracCollLeft.Proximity.IntersectPoint, attracCollLeft.Proximity.IntersectPoint + Vector3(0.0f, -100.0f, 0.0f), COLOR_MAGENTA);
		g_Renderer.AddLine3D(attracCollRight.Proximity.IntersectPoint, attracCollRight.Proximity.IntersectPoint + Vector3(0.0f, -100.0f, 0.0f), COLOR_MAGENTA);

		short angleDelta = Geometry::GetShortestAngle(attracCollCenter.HeadingAngle, (sideOffset >= 0.0f) ? attracCollRight.HeadingAngle : attracCollLeft.HeadingAngle);
		g_Renderer.PrintDebugMessage("Angle delta: %.3f", TO_DEGREES(angleDelta));
		//------------
		
		// Return attractor collisions at three points.
		return EdgeHangAttractorCollisionData
		{
			attracCollCenter,
			attracCollLeft,
			attracCollRight
		};
	}

	void HandlePlayerEdgeMovement(ItemInfo& item, const CollisionInfo& coll, bool isGoingRight)
	{
		constexpr auto ORIENT_LERP_ALPHA = 0.5f;

		auto& player = GetLaraInfo(item);

		// Check for hands attractor.
		if (player.Context.HandsAttractor.AttracPtr == nullptr)
		{
			player.Control.IsHanging = false;
			return;
		}

		// Get edge hang attractor collisions.
		float sideOffset = item.Animation.Velocity.z * (isGoingRight ? 1 : -1);
		auto edgeAttracColls = GetEdgeHangAttractorCollisions(item, coll, sideOffset);
		if (!edgeAttracColls.has_value())
		{
			player.Control.IsHanging = false;
			return;
		}

		// Test segment slope angle.
		if (edgeAttracColls->Center.SlopeAngle >= SLIPPERY_SLOPE_ANGLE)
		{
			player.Control.IsHanging = false;
			return;
		}

		// Calculate heading angle.
		auto orient = Geometry::GetOrientToPoint(edgeAttracColls->Left.Proximity.IntersectPoint, edgeAttracColls->Right.Proximity.IntersectPoint);
		auto headingAngle = orient.y - ANGLE(90.0f);

		// Align orientation.
		player.Context.TargetOrientation = EulerAngles(0, headingAngle, 0);
		item.Pose.Orientation.Lerp(player.Context.TargetOrientation, ORIENT_LERP_ALPHA);

		// Determine target point (correctly handles positioning at inner bends).
		auto targetPoint = edgeAttracColls->Center.Proximity.IntersectPoint;
		if (!Geometry::IsPointInFront(targetPoint, edgeAttracColls->Left.Proximity.IntersectPoint, player.Context.TargetOrientation) &&
			!Geometry::IsPointInFront(targetPoint, edgeAttracColls->Right.Proximity.IntersectPoint, player.Context.TargetOrientation))
		{
			targetPoint = (edgeAttracColls->Left.Proximity.IntersectPoint + edgeAttracColls->Right.Proximity.IntersectPoint) / 2;
		}

		// Align position.
		auto rotMatrix = Matrix::CreateRotationY(TO_RAD(headingAngle));
		auto relOffset = Vector3(0.0f, coll.Setup.Height, -coll.Setup.Radius);
		item.Pose.Position = targetPoint + Vector3::Transform(relOffset, rotMatrix);

		// Set edge hang parameters.
		player.Control.IsHanging = true;
		player.Context.HandsAttractor.Set(edgeAttracColls->Center.Attrac, edgeAttracColls->Center.Proximity.ChainDistance);
	}

	// State:	  LS_HANG_IDLE (10)
	// Collision: lara_col_hang_idle()
	void lara_as_hang_idle(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		item->Animation.IsAirborne = false;
		player.Control.MoveAngle = item->Pose.Orientation.y;
		player.Control.IsClimbingLadder = false;
		player.Control.Look.Mode = LookMode::Free;
		coll->Setup.Mode = CollisionProbeMode::FreeFlat;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-45.0f);

		if (item->HitPoints <= 0)
		{
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		HandlePlayerEdgeMovement(*item, *coll, true);

		if (IsHeld(In::Action) && player.Control.IsHanging)
		{
			if (TestLaraClimbIdle(item, coll))
			{
				item->Animation.TargetState = LS_LADDER_IDLE;
				return;
			}

			if (IsHeld(In::Jump) && Context::CanPerformLedgeJump(*item, *coll))
			{
				if (IsHeld(In::Back))
				{
					item->Animation.TargetState = LS_JUMP_FORWARD;
				}
				else
				{
					item->Animation.TargetState = LS_JUMP_UP;
				}

				return;
			}

			if (IsHeld(In::Forward))
			{
				if (Context::CanClimbLedgeToCrouch(*item, *coll))
				{
					item->Animation.TargetState = LS_HANG_TO_CROUCH;
					return;
				}
				else if (Context::CanClimbLedgeToStand(*item, *coll))
				{
					if (IsHeld(In::Crouch))
					{
						item->Animation.TargetState = LS_HANG_TO_CROUCH;
					}
					else if (IsHeld(In::Walk) && Context::CanPerformLedgeHandstand(*item, *coll))
					{
						item->Animation.TargetState = LS_HANDSTAND;
					}
					else
					{
						item->Animation.TargetState = LS_GRABBING;
					}

					return;
				}

				if (Context::CanShimmyUp(*item, *coll))
				{
					// TODO: State dispatch.
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
					return;
				}
			}
			else if (IsHeld(In::Back))
			{
				if (Context::CanShimmyDown(*item, *coll))
				{
					// TODO: State dispatch.
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
					return;
				}
			}

			if (IsHeld(In::Left) || IsHeld(In::StepLeft))
			{
				if (Context::CanShimmyLeft(*item, *coll) && HasStateDispatch(item, LS_SHIMMY_LEFT))
				{
					item->Animation.TargetState = LS_SHIMMY_LEFT;
					return;
				}

				auto cornerShimmyState = GetPlayerCornerShimmyState(*item, *coll);
				if (cornerShimmyState.has_value())
				{
					item->Animation.TargetState = *cornerShimmyState;
					return;
				}
			}
			else if (IsHeld(In::Right) || IsHeld(In::StepRight))
			{
				if (Context::CanShimmyRight(*item, *coll) && HasStateDispatch(item, LS_SHIMMY_RIGHT))
				{
					item->Animation.TargetState = LS_SHIMMY_RIGHT;
					return;
				}

				auto cornerShimmyState = GetPlayerCornerShimmyState(*item, *coll);
				if (cornerShimmyState.has_value())
				{
					item->Animation.TargetState = *cornerShimmyState;
					return;
				}
			}

			item->Animation.TargetState = LS_HANG_IDLE;
			return;
		}

		SetPlayerEdgeHangRelease(*item);
	}

	// State:	LS_HANG_IDLE (10)
	// Control: lara_as_hang_idle()
	void lara_col_hang_idle(ItemInfo* item, CollisionInfo* coll)
	{
		//HandlePlayerEdgeHang(*item, *coll);
	}

	// State:	  LS_SHIMMY_LEFT (30)
	// Collision: lara_col_shimmy_left()
	void lara_as_shimmy_left(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
		coll->Setup.Mode = CollisionProbeMode::FreeFlat;
		coll->Setup.Radius = LARA_RADIUS;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-45.0f);

		if (item->HitPoints <= 0)
		{
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		HandlePlayerEdgeMovement(*item, *coll, false);

		if (IsHeld(In::Action) && player.Control.IsHanging)
		{
			if (IsHeld(In::Left) || IsHeld(In::StepLeft))
			{
				item->Animation.TargetState = LS_SHIMMY_LEFT;
				return;
			}

			item->Animation.TargetState = LS_HANG_IDLE;
			return;
		}

		SetPlayerEdgeHangRelease(*item);
	}

	// State:	LS_SHIMMY_LEFT (30)
	// Control: lara_as_shimmy_left()
	void lara_col_shimmy_left(ItemInfo* item, CollisionInfo* coll)
	{
		//auto& player = GetLaraInfo(*item);

		//HandlePlayerEdgeHang(*item, *coll);
		//player.Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
	}

	// State:	  LS_SHIMMY_RIGHT (31)
	// Collision: lara_col_shimmy_right()
	void lara_as_shimmy_right(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
		coll->Setup.Mode = CollisionProbeMode::FreeFlat;
		coll->Setup.Radius = LARA_RADIUS;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-45.0f);

		if (item->HitPoints <= 0)
		{
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		HandlePlayerEdgeMovement(*item, *coll, true);

		if (IsHeld(In::Action) && player.Control.IsHanging)
		{
			if (IsHeld(In::Right) || IsHeld(In::StepRight))
			{
				item->Animation.TargetState = LS_SHIMMY_RIGHT;
				return;
			}

			item->Animation.TargetState = LS_HANG_IDLE;
			return;
		}

		SetPlayerEdgeHangRelease(*item);
	}

	// State:	LS_SHIMMY_RIGHT (31)
	// Control: lara_as_shimmy_right()
	void lara_col_shimmy_right(ItemInfo* item, CollisionInfo* coll)
	{
		//auto& player = GetLaraInfo(*item);

		//HandlePlayerEdgeHang(*item, *coll);
		//player.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
	}

	// State:	  LS_SHIMMY_OUTER_LEFT (107), LS_SHIMMY_OUTER_RIGHT (108), LS_SHIMMY_INNER_LEFT (109), LS_SHIMMY_INNER_RIGHT (110)
	// Collision: lara_default_col()
	void lara_as_shimmy_corner(ItemInfo* item, CollisionInfo* coll)
	{
		// Setup
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-33.0f);
		Camera.laraNode = LM_TORSO;

		SetPlayerCornerShimmyEnd(*item, *coll, TestLastFrame(item));
	}

	// State:	  LS_HANDSTAND (54)
	// Collision: lara_default_col()
	void lara_as_handstand(ItemInfo* item, CollisionInfo* coll)
	{
		// Setup
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
	}
}
