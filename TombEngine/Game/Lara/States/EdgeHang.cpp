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
#include "Renderer/Renderer11.h"
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

	static std::optional<AttractorCollisionData> GetConnectingEdgeAttractorCollision(const ItemInfo& item, const CollisionInfo& coll,
																					 Attractor& currentAttrac, float currentChainDist,
																					 const Vector3& probePoint)
	{
		constexpr auto CONNECT_DIST_THRESHOLD = BLOCK(1 / 64.0f);
		constexpr auto CORNER_ANGLE_MAX		  = ANGLE(30.0f);

		// Get attractor collisions.
		auto attracColls = GetAttractorCollisions(probePoint, item.RoomNumber, item.Pose.Orientation.y, CONNECT_DIST_THRESHOLD);
		auto currentAttracColl = GetAttractorCollision(currentAttrac, currentChainDist, item.Pose.Orientation.y);

		// Assess attractor collision.
		for (const auto& attracColl : attracColls)
		{
			// 1) Filter out current attractor.
			if (attracColl.AttractorPtr == &currentAttrac)
				continue;

			// 2) Check if attractor is edge type.
			if (attracColl.AttractorPtr->GetType() != AttractorType::Edge &&
				attracColl.AttractorPtr->GetType() != AttractorType::WallEdge)
			{
				continue;
			}

			// 3) Test sharp corner.
			if (Geometry::GetShortestAngle(attracColl.HeadingAngle, currentAttracColl.HeadingAngle) > CORNER_ANGLE_MAX)
				continue;

			// Return closest connecting attractor.
			return attracColl;
		}

		// No connecting attractor found; return nullopt.
		return std::nullopt;
	}

	static std::optional<EdgeHangAttractorCollisionData> GetEdgeHangAttractorCollisions(ItemInfo& item, const CollisionInfo& coll,
																						float sideOffset = 0.0f)
	{
		auto& player = GetLaraInfo(item);
		auto& handsAttrac = player.Context.Attractor;

		const auto& points = handsAttrac.Ptr->GetPoints();
		float length = handsAttrac.Ptr->GetLength();

		// Calculate projected distances along attractor.
		float chainDistCenter = handsAttrac.ChainDistance + sideOffset;
		float chainDistLeft = chainDistCenter - coll.Setup.Radius;
		float chainDistRight = chainDistCenter + coll.Setup.Radius;

		bool isLooped = handsAttrac.Ptr->IsLooped();

		// TODO: Horrible, organise later.
		// Get connecting attractors just in case.
		auto connectingAttracCollCenter = GetConnectingEdgeAttractorCollision(item, coll, *player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance, (chainDistCenter <= 0.0f) ? points.front() : points.back());
		auto connectingAttracCollLeft = GetConnectingEdgeAttractorCollision(item, coll, *player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance, points.front());
		auto connectingAttracCollRight = GetConnectingEdgeAttractorCollision(item, coll, *player.Context.Attractor.Ptr, player.Context.Attractor.ChainDistance, points.back());

		// Get points.
		auto pointCenter = handsAttrac.Ptr->GetIntersectionAtChainDistance(chainDistCenter);
		if (!isLooped)
		{
			if (connectingAttracCollCenter.has_value())
			{
				// Get point at connecting attractor.
				if (chainDistCenter <= 0.0f)
				{
					float transitLineDist = connectingAttracCollCenter->Proximity.ChainDistance + chainDistCenter;
					pointCenter = connectingAttracCollCenter->AttractorPtr->GetIntersectionAtChainDistance(transitLineDist);
				}
				else if (chainDistCenter >= length)
				{
					float transitLineDist = connectingAttracCollCenter->Proximity.ChainDistance + (chainDistCenter - length);
					pointCenter = connectingAttracCollCenter->AttractorPtr->GetIntersectionAtChainDistance(transitLineDist);
				}
			}
			else
			{
				// Get point within boundary of current attractor.
				if (chainDistLeft <= 0.0f && !connectingAttracCollLeft.has_value())
				{
					pointCenter = handsAttrac.Ptr->GetIntersectionAtChainDistance(coll.Setup.Radius);
				}
				else if (chainDistRight >= length && !connectingAttracCollRight.has_value())
				{
					pointCenter = handsAttrac.Ptr->GetIntersectionAtChainDistance(length - coll.Setup.Radius);
				}

				// TODO: Unreachable segments on current attractor. Too steep, angle difference to great.
			}
		}

		auto pointLeft = ((chainDistLeft <= 0.0f) && !isLooped && connectingAttracCollLeft.has_value()) ?
			connectingAttracCollLeft->AttractorPtr->GetIntersectionAtChainDistance(connectingAttracCollLeft->Proximity.ChainDistance + chainDistLeft) :
			handsAttrac.Ptr->GetIntersectionAtChainDistance(chainDistLeft);
		auto pointRight = ((chainDistRight >= length) && !isLooped && connectingAttracCollRight.has_value()) ?
			connectingAttracCollRight->AttractorPtr->GetIntersectionAtChainDistance(connectingAttracCollRight->Proximity.ChainDistance + (chainDistRight - length)) :
			handsAttrac.Ptr->GetIntersectionAtChainDistance(chainDistRight);

		auto headingAngle = item.Pose.Orientation.y;

		// Get attractor collisions.
		auto attracCollCenter = ((chainDistCenter <= 0.0f || chainDistCenter >= length) && connectingAttracCollCenter.has_value()) ?
			GetAttractorCollision(*connectingAttracCollCenter->AttractorPtr, chainDistCenter, headingAngle) :
			GetAttractorCollision(*handsAttrac.Ptr, chainDistCenter, headingAngle);

		auto attracCollLeft = ((chainDistLeft <= 0.0f) && !isLooped && connectingAttracCollLeft.has_value()) ?
			GetAttractorCollision(*connectingAttracCollLeft->AttractorPtr, chainDistLeft, headingAngle) :
			GetAttractorCollision(*handsAttrac.Ptr, chainDistLeft, headingAngle);

		auto attracCollRight = ((chainDistRight >= length) && !isLooped && connectingAttracCollRight.has_value()) ?
			GetAttractorCollision(*connectingAttracCollRight->AttractorPtr, chainDistRight, headingAngle) :
			GetAttractorCollision(*handsAttrac.Ptr, chainDistRight, headingAngle);

		// ----------Debug
		constexpr auto COLOR_MAGENTA = Vector4(1, 0, 1, 1);
		g_Renderer.AddDebugLine(attracCollCenter.Proximity.Intersection, attracCollCenter.Proximity.Intersection + Vector3(0.0f, -150.0f, 0.0f), COLOR_MAGENTA, RendererDebugPage::AttractorStats);
		g_Renderer.AddDebugLine(attracCollLeft.Proximity.Intersection, attracCollLeft.Proximity.Intersection + Vector3(0.0f, -100.0f, 0.0f), COLOR_MAGENTA, RendererDebugPage::AttractorStats);
		g_Renderer.AddDebugLine(attracCollRight.Proximity.Intersection, attracCollRight.Proximity.Intersection + Vector3(0.0f, -100.0f, 0.0f), COLOR_MAGENTA, RendererDebugPage::AttractorStats);

		short angleDelta = Geometry::GetShortestAngle(attracCollCenter.HeadingAngle, (sideOffset >= 0.0f) ? attracCollRight.HeadingAngle : attracCollLeft.HeadingAngle);
		g_Renderer.PrintDebugMessage("Angle delta: %.3f", TO_DEGREES(angleDelta), RendererDebugPage::AttractorStats);
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
		auto& player = GetLaraInfo(item);

		// End hang if hands attractor doesn't exist.
		if (player.Context.Attractor.Ptr == nullptr)
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
		if (edgeAttracColls->Center.SlopeAngle >= ILLEGAL_FLOOR_SLOPE_ANGLE)
		{
			player.Control.IsHanging = false;
			return;
		}

		// Calculate target orientation.
		auto targetOrient = Geometry::GetOrientToPoint(edgeAttracColls->Left.Proximity.Intersection, edgeAttracColls->Right.Proximity.Intersection);
		targetOrient.y -= ANGLE(90.0f);

		// Calculate target position.
		auto targetPos = edgeAttracColls->Center.Proximity.Intersection;
		if (!Geometry::IsPointInFront(targetPos, edgeAttracColls->Left.Proximity.Intersection, targetOrient) &&
			!Geometry::IsPointInFront(targetPos, edgeAttracColls->Right.Proximity.Intersection, targetOrient))
		{
			targetPos = (edgeAttracColls->Left.Proximity.Intersection + edgeAttracColls->Right.Proximity.Intersection) / 2;
		}

		// Calculate relative position and orientation offsets.
		auto rotMatrix = targetOrient.ToRotationMatrix();
		auto relPosOffset = Vector3(0.0f, coll.Setup.Height, -coll.Setup.Radius) +
							Vector3::Transform(targetPos - edgeAttracColls->Center.Proximity.Intersection, rotMatrix);
		auto relOrientOffset = targetOrient - EulerAngles(0, edgeAttracColls->Center.HeadingAngle, 0);

		// Set edge hang parameters.
		player.Control.IsHanging = true;
		player.Context.Attractor.Update(
			item, *edgeAttracColls->Center.AttractorPtr, edgeAttracColls->Center.Proximity.ChainDistance,
			relPosOffset, relOrientOffset);

		HandlePlayerAttractorParent(item);
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
					item->Animation.TargetState = LS_HANG_TO_CROUCH;
					return;
				}
				else if (CanClimbLedgeToStand(*item, *coll))
				{
					if (IsHeld(In::Crouch))
					{
						item->Animation.TargetState = LS_HANG_TO_CROUCH;
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

				if (CanShimmyUp(*item, *coll))
				{
					// TODO: State dispatch.
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
					return;
				}
			}
			else if (IsHeld(In::Back))
			{
				if (CanShimmyDown(*item, *coll))
				{
					// TODO: State dispatch.
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
					return;
				}
			}

			if (IsHeld(In::Left) || IsHeld(In::StepLeft))
			{
				if (CanShimmyLeft(*item, *coll) && HasStateDispatch(item, LS_SHIMMY_LEFT))
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
				if (CanShimmyRight(*item, *coll) && HasStateDispatch(item, LS_SHIMMY_RIGHT))
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

		HandlePlayerAttractorParent(*item);
	}
}
