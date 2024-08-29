#include "framework.h"
#include "Game/Lara/States/WallClimb.h"

#include "Game/camera.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/Context/Context.h"
#include "Specific/Input/Input.h"

// temp
#include <Game/Setup.h>

using namespace TEN::Input;

namespace TEN::Player
{
	void PlayerStateWallClimbIdle(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup.
		player.Control.Look.Mode = LookMode::Free;
		player.Control.IsClimbingWall = true;
		coll->Setup.Height = PLAYER_HEIGHT_WALL_CLIMB;
		coll->Setup.EnableSpasm = false;
		coll->Setup.EnableObjectPush = false;
		Camera.targetElevation = ANGLE(-20.0f);

		// HACK
		if (item->Animation.AnimNumber != LA_STAND_TO_WALL_CLIMB)
		{
			player.Context.Attractor.Update(
				*item, *player.Context.Attractor.Attractor, player.Context.Attractor.PathDistance,
				Vector3(0.0f, coll->Setup.Height, -coll->Setup.Radius), EulerAngles::Identity);
		}

		// TODO: Add GetWallClimbIdleContext() purely for the alignment?

		HandlePlayerAttractorParent(*item);
		
		// Death.
		if (item->HitPoints <= 0)
		{
			item->Animation.TargetState = LS_DEATH;
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		// Climb.
		if (IsHeld(In::Action))
		{
			// Hang.
			if (IsHeld(In::Crouch) && CanWallClimbToEdgeHang(*item, *coll))
			{
				item->Animation.TargetState = LS_EDGE_HANG_IDLE;
				return;
			}

			// Jump.
			if (IsHeld(In::Jump))
			{
				item->Animation.TargetState = LS_JUMP_BACK;
				return;
			}

			// Move up/down.
			if (IsHeld(In::Forward))
			{
				auto climbContext = GetWallClimbUpContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}
			else if (IsHeld(In::Back))
			{
				auto climbContext = GetWallClimbDownContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}

			// Move left/right.
			if (IsHeld(In::Left) || IsHeld(In::StepLeft))
			{
				auto climbContext = GetWallClimbLeftContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}
			else if (IsHeld(In::Right) || IsHeld(In::StepRight))
			{
				auto climbContext = GetWallClimbRightContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}

			// Reset.
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
			return;
		}

		// Reset.
		item->Animation.TargetState = LS_REACH;
		SetPlayerEdgeHangRelease(*item);
	}

	void PlayerStateWallClimbUp(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup.
		player.Control.Look.Mode = LookMode::Horizontal;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableSpasm = false;
		coll->Setup.EnableObjectPush = false;
		Camera.targetElevation = ANGLE(30.0f);

		HandlePlayerAttractorParent(*item);

		// Death.
		if (item->HitPoints <= 0)
		{
			item->Animation.TargetState = LS_DEATH;
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		// Climb.
		if (IsHeld(In::Action))
		{
			// Hang.
			if (IsHeld(In::Crouch) && CanWallClimbToEdgeHang(*item, *coll))
			{
				item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
				return;
			}

			// Jump.
			if (IsHeld(In::Jump))
			{
				item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
				return;
			}

			// Move up.
			if (IsHeld(In::Forward))
			{
				auto climbContext = GetWallClimbUpContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}

			// Reset.
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
			return;
		}

		// Reset.
		item->Animation.TargetState = LS_REACH;
		SetPlayerEdgeHangRelease(*item);
	}

	void PlayerStateWallClimbDown(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.Look.Mode = LookMode::Horizontal;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableSpasm = false;
		coll->Setup.EnableObjectPush = false;
		Camera.targetElevation = ANGLE(-45.0f);

		HandlePlayerAttractorParent(*item);

		// Death.
		if (item->HitPoints <= 0)
		{
			item->Animation.TargetState = LS_DEATH;
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		// Climb.
		if (IsHeld(In::Action))
		{
			// Hang.
			if (IsHeld(In::Crouch) && CanWallClimbToEdgeHang(*item, *coll))
			{
				item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
				return;
			}

			// Jump.
			if (IsHeld(In::Jump))
			{
				item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
				return;
			}

			// Move down.
			if (IsHeld(In::Back))
			{
				auto climbContext = GetWallClimbDownContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}

			// Reset.
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
			return;
		}

		// Reset.
		item->Animation.TargetState = LS_REACH;
		SetPlayerEdgeHangRelease(*item);
	}

	void PlayerStateWallClimbLeft(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.Look.Mode = LookMode::Vertical;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableSpasm = false;
		coll->Setup.EnableObjectPush = false;
		Camera.targetAngle = ANGLE(-30.0f);
		Camera.targetElevation = ANGLE(-20.0f);

		// Death.
		if (item->HitPoints <= 0)
		{
			item->Animation.TargetState = LS_DEATH;
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		// Climb.
		if (IsHeld(In::Action))
		{
			// Move left.
			if (IsHeld(In::Left) || IsHeld(In::StepLeft))
			{
				auto climbContext = GetWallClimbLeftContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}

			// Reset.
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
			return;
		}

		// Reset.
		item->Animation.TargetState = LS_REACH;
		SetPlayerEdgeHangRelease(*item);
	}

	void PlayerStateWallClimbRight(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.Look.Mode = LookMode::Vertical;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableSpasm = false;
		coll->Setup.EnableObjectPush = false;
		Camera.targetAngle = ANGLE(30.0f);
		Camera.targetElevation = ANGLE(-20.0f);

		// Death.
		if (item->HitPoints <= 0)
		{
			item->Animation.TargetState = LS_DEATH;
			SetPlayerEdgeHangRelease(*item);
			return;
		}

		// Climb.
		if (IsHeld(In::Action))
		{
			// Move right.
			if (IsHeld(In::Right) || IsHeld(In::StepRight))
			{
				auto climbContext = GetWallClimbRightContext(*item, *coll);
				if (climbContext.has_value())
				{
					item->Animation.TargetState = climbContext->TargetStateID;
					SetPlayerClimb(*item, *climbContext);
					return;
				}
			}

			// Reset.
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
			return;
		}

		// Reset.
		item->Animation.TargetState = LS_REACH;
		SetPlayerEdgeHangRelease(*item);
	}

	void PlayerStateWallClimbDismountLeft(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.Look.Mode = LookMode::None;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		Camera.targetAngle = ANGLE(-30.0f);
		Camera.targetElevation = ANGLE(-20.0f);
	}

	void PlayerStateWallClimbDismountRight(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.Look.Mode = LookMode::None;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		Camera.targetAngle = ANGLE(30.0f);
		Camera.targetElevation = ANGLE(-20.0f);
	}

	void lara_as_wall_climb_end(ItemInfo* item, CollisionInfo* coll)
	{
		auto& player = GetLaraInfo(*item);

		// Setup
		player.Control.Look.Mode = LookMode::None;
		player.Control.IsClimbingWall = true;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;
		Camera.flags = CF_FOLLOW_CENTER;
		Camera.targetAngle = ANGLE(-20.0f);
	}
}
