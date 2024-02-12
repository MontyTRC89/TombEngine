#include "framework.h"
#include "Game/Lara/States/EdgeHang.h"

#include "Game/camera.h"
#include "Game/collision/AttractorCollision.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/PlayerContext.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Collision::Attractor;
using namespace TEN::Input;

//------Debug
#include "Renderer/Renderer.h"
using namespace TEN::Renderer;
//------

namespace TEN::Entities::Player
{
	// notes:
	// 1. get shimmy context data (ShimmyData with attractor collisions).
	// 2. pass to edge movement handler?

	struct EdgeHangAttractorCollisionData
	{
		AttractorCollisionData Center = {};
		AttractorCollisionData Left	  = {};
		AttractorCollisionData Right  = {};
	};
	
	static std::optional<EdgeHangAttractorCollisionData> GetEdgeHangAttractorCollisions(ItemInfo& item, const CollisionInfo& coll)
	{
		auto& player = GetLaraInfo(item);
		auto& handsAttrac = player.Context.Attractor;

		const auto& points = handsAttrac.Ptr->GetPoints();
		float length = handsAttrac.Ptr->GetLength();

		// Calculate distances along attractor.
		float chainDistCenter = handsAttrac.ChainDistance;
		float chainDistLeft = chainDistCenter - coll.Setup.Radius;
		float chainDistRight = chainDistCenter + coll.Setup.Radius;

		// TODO: Find connecting attractors.

		// Get attractor collisions.
		auto attracCollCenter = GetAttractorCollision(*handsAttrac.Ptr, chainDistCenter, item.Pose.Orientation.y);
		auto attracCollLeft = GetAttractorCollision(*handsAttrac.Ptr, chainDistLeft, item.Pose.Orientation.y);
		auto attracCollRight = GetAttractorCollision(*handsAttrac.Ptr, chainDistRight, item.Pose.Orientation.y);

		// DEBUG----
		g_Renderer.AddDebugLine(attracCollCenter.Proximity.Intersection, attracCollCenter.Proximity.Intersection + Vector3(0, -CLICK(0.5f), 0), Color(1, 1, 0));
		g_Renderer.AddDebugLine(attracCollLeft.Proximity.Intersection, attracCollLeft.Proximity.Intersection + Vector3(0, -CLICK(0.25f), 0), Color(1, 1, 0));
		g_Renderer.AddDebugLine(attracCollRight.Proximity.Intersection, attracCollRight.Proximity.Intersection + Vector3(0, -CLICK(0.25f), 0), Color(1, 1, 0));
		//--------
		
		// Return attractor collisions at three points.
		return EdgeHangAttractorCollisionData
		{
			attracCollCenter,
			attracCollLeft,
			attracCollRight
		};
	}

	static void SetPlayerEdgeHangClimb(ItemInfo& item, const CollisionInfo& coll, const ClimbContextData climbContext)
	{
		auto& player = GetLaraInfo(item);

		// No attractor; detach.
		if (climbContext.AttractorPtr == nullptr)
		{
			player.Control.IsHanging = false;
			player.Context.Attractor.Detach(item);
			return;
		}

		// Get edge hang attractor collisions.
		auto edgeAttracColls = GetEdgeHangAttractorCollisions(item, coll);

		// Calculate target orientation.
		auto targetOrient = Geometry::GetOrientToPoint(edgeAttracColls->Left.Proximity.Intersection, edgeAttracColls->Right.Proximity.Intersection);
		targetOrient = EulerAngles(0, targetOrient.y - ANGLE(90.0f), 0);

		// Calculate target position.
		auto targetPos = edgeAttracColls->Center.Proximity.Intersection;
		if (!Geometry::IsPointInFront(targetPos, edgeAttracColls->Left.Proximity.Intersection, targetOrient) &&
			!Geometry::IsPointInFront(targetPos, edgeAttracColls->Right.Proximity.Intersection, targetOrient))
		{
			targetPos = (edgeAttracColls->Left.Proximity.Intersection + edgeAttracColls->Right.Proximity.Intersection) / 2;
		}

		// Calculate relative position and orientation offsets.
		auto rotMatrix = targetOrient.ToRotationMatrix();
		auto relPosOffset = Vector3::Transform((targetPos - edgeAttracColls->Center.Proximity.Intersection), rotMatrix) + climbContext.RelPosOffset;
		auto relOrientOffset = (targetOrient - EulerAngles(0, edgeAttracColls->Center.HeadingAngle, 0)) + climbContext.RelOrientOffset;

		// Set edge hang parameters.
		player.Control.IsHanging = true;
		player.Context.Attractor.Update(
			item, *climbContext.AttractorPtr, climbContext.ChainDistance,
			relPosOffset, relOrientOffset);
	}
	
	// TODO: Stationary check.
	bool TestPlayerEdgeHang()
	{
		return true;
	}

	// State:	  LS_EDGE_HANG_IDLE (10)
	// Collision: lara_void_func()
	void lara_as_edge_hang_idle(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		item->Animation.IsAirborne = false;
		player.Control.MoveAngle = item->Pose.Orientation.y;
		player.Control.IsClimbingWall = false;
		player.Control.Look.Mode = LookMode::Free;
		coll->Setup.Mode = CollisionProbeMode::FreeFlat;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-45.0f);/*
		player.Context.Attractor.Update(
			*item, *player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance,
			Vector3(0.0f, coll->Setup.Height, -coll->Setup.Radius), EulerAngles::Identity);*/

		HandlePlayerAttractorParent(*item);

		if (item->HitPoints <= 0)
		{
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		if (IsHeld(In::Action) && TestPlayerEdgeHang())
		{
			if (!IsHeld(In::Crouch) && CanEdgeHangToWallClimb(*item, *coll))
			{
				item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
				return;
			}

			if (IsHeld(In::Jump) && CanPerformLedgeJump(*item, *coll))
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
				if (CanClimbLedgeToCrouch(*item, *coll))
				{
					item->Animation.TargetState = LS_EDGE_HANG_TO_CROUCH;
					return;
				}
				else if (CanClimbLedgeToStand(*item, *coll))
				{
					if (IsHeld(In::Crouch))
					{
						item->Animation.TargetState = LS_EDGE_HANG_TO_CROUCH;
					}
					else if (IsHeld(In::Walk) && CanPerformLedgeHandstand(*item, *coll))
					{
						item->Animation.TargetState = LS_HANDSTAND;
					}
					else
					{
						item->Animation.TargetState = LS_GRABBING;
					}

					return;
				}

				auto climbContext = GetEdgeHangShimmyUpContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerEdgeHangClimb(*item, *coll, *climbContext);
					return;
				}
			}
			else if (IsHeld(In::Back))
			{
				auto climbContext = GetEdgeHangShimmyDownContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerEdgeHangClimb(*item, *coll, *climbContext);
					return;
				}
			}

			if (IsHeld(In::Left) || IsHeld(In::StepLeft))
			{
				auto climbContext = GetEdgeHangShimmyLeftContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerEdgeHangClimb(*item, *coll, *climbContext);
					return;
				}
			}
			else if (IsHeld(In::Right) || IsHeld(In::StepRight))
			{
				auto climbContext = GetEdgeHangShimmyRightContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerEdgeHangClimb(*item, *coll, *climbContext);
					return;
				}
			}

			item->Animation.TargetState = LS_EDGE_HANG_IDLE;
			return;
		}

		SetPlayerEdgeHangRelease(*item);
	}

	void lara_as_edge_hang_shimmy_up(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		player.Control.Look.Mode = LookMode::None;

		HandlePlayerAttractorParent(*item);
	}

	void lara_as_edge_hang_shimmy_down(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		player.Control.Look.Mode = LookMode::None;

		HandlePlayerAttractorParent(*item);
	}

	// State:	  LS_EDGE_HANG_SHIMMY_LEFT (30)
	// Collision: lara_void_func()
	void lara_as_edge_hang_shimmy_left(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
		player.Control.Look.Mode = LookMode::Vertical;
		coll->Setup.Mode = CollisionProbeMode::FreeFlat;
		coll->Setup.Radius = LARA_RADIUS;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-45.0f);

		HandlePlayerAttractorParent(*item);

		if (item->HitPoints <= 0)
		{
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		if (IsHeld(In::Action))
		{
			auto climbContext = GetEdgeHangShimmyLeftContext(*item, *coll);
			if (climbContext.has_value())
			{
				item->Animation.TargetState = climbContext->TargetStateID;
				SetPlayerEdgeHangClimb(*item, *coll, *climbContext);

				if ((IsHeld(In::Left) || IsHeld(In::StepLeft)) && !HasStateDispatch(item, item->Animation.TargetState))
					return;
			}

			item->Animation.TargetState = LS_EDGE_HANG_IDLE;
			return;
		}

		SetPlayerEdgeHangRelease(*item);
	}

	// State:	  LS_EDGE_HANG_SHIMMY_RIGHT (31)
	// Collision: lara_void_func()
	void lara_as_edge_hang_shimmy_right(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
		player.Control.Look.Mode = LookMode::Vertical;
		coll->Setup.Mode = CollisionProbeMode::FreeFlat;
		coll->Setup.Radius = LARA_RADIUS;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-45.0f);

		HandlePlayerAttractorParent(*item);

		if (item->HitPoints <= 0)
		{
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		if (IsHeld(In::Action))
		{
			auto climbContext = GetEdgeHangShimmyRightContext(*item, *coll);
			if (climbContext.has_value())
			{
				item->Animation.TargetState = climbContext->TargetStateID;
				SetPlayerEdgeHangClimb(*item, *coll, *climbContext);

				if ((IsHeld(In::Right) || IsHeld(In::StepRight)) && !HasStateDispatch(item, item->Animation.TargetState))
					return;
			}

			item->Animation.TargetState = LS_EDGE_HANG_IDLE;
			return;
		}

		SetPlayerEdgeHangRelease(*item);
	}

	// State:	  LS_EDGE_HANG_SHIMMY_OUTER_LEFT (107), LS_EDGE_HANG_SHIMMY_OUTER_RIGHT (108), LS_EDGE_HANG_SHIMMY_INNER_LEFT (109), LS_EDGE_HANG_SHIMMY_INNER_RIGHT (110)
	// Collision: lara_default_col()
	void lara_as_edge_hang_shimmy_corner(ItemInfo* item, CollisionInfo* coll)
	{
		// Setup
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		Camera.targetAngle = 0;
		Camera.targetElevation = ANGLE(-33.0f);
		Camera.laraNode = LM_TORSO;

		HandlePlayerAttractorParent(*item);
	}

	// State:	  LS_HANDSTAND (54)
	// Collision: lara_default_col()
	void lara_as_edge_hang_handstand(ItemInfo* item, CollisionInfo* coll)
	{
		// Setup
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;

		HandlePlayerAttractorParent(*item);
	}
}
