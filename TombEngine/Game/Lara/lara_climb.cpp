#include "framework.h"
#include "Game/Lara/lara_climb.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

constexpr auto WALL_CLIMB_TEST_MARGIN	= 8;
constexpr auto WALL_CLIMB_TEST_DISTANCE = CLICK(0.5f) - WALL_CLIMB_TEST_MARGIN;
constexpr auto WALL_CLIMB_CLIMB_SHIFT	= 70;

// --------
// HELPERS:
// --------

static void LaraDoClimbLeftRight(ItemInfo* item, CollisionInfo* coll, int result, int shift)
{
	if (result == 1)
	{
		if (IsHeld(In::Left))
			item->Animation.TargetState = LS_WALL_CLIMB_LEFT;
		else if (IsHeld(In::Right))
			item->Animation.TargetState = LS_WALL_CLIMB_RIGHT;
		else
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;

		item->Pose.Position.y += shift;
		return;
	}

	if (result != 0)
	{
		item->Animation.TargetState = LS_EDGE_HANG_IDLE;

		do
		{
			AnimateItem(item);
		} while (item->Animation.ActiveState != LS_EDGE_HANG_IDLE);

		item->Pose.Position.x = coll->Setup.PrevPosition.x;
		item->Pose.Position.z = coll->Setup.PrevPosition.z;

		return;
	}

	item->Pose.Position.x = coll->Setup.PrevPosition.x;
	item->Pose.Position.z = coll->Setup.PrevPosition.z;

	item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
	item->Animation.ActiveState = LS_WALL_CLIMB_IDLE;

	if (coll->Setup.PrevState != LS_WALL_CLIMB_IDLE)
	{	
		SetAnimation(item, LA_WALL_CLIMB_IDLE);
		return;
	}

	if (IsHeld(In::Left))
	{
		short troomnumber = item->RoomNumber;
		int dx = int(sin(TO_RAD(item->Pose.Orientation.y - ANGLE(90.0f))) * 10);
		int dz = int(cos(TO_RAD(item->Pose.Orientation.y - ANGLE(90.0f))) * 10);
		int height = GetFloorHeight(GetFloor(item->Pose.Position.x + dx, item->Pose.Position.y, item->Pose.Position.z + dz, &troomnumber),
			item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) - item->Pose.Position.y;
		if (height < CLICK(1.5f))
		{
			item->Animation.TargetState = LS_WALL_CLIMB_DISMOUNT_LEFT;
			item->Animation.ActiveState = LS_MISC_CONTROL;
		}
	}
	else if (IsHeld(In::Right))
	{
		short troomnumber = item->RoomNumber;
		int dx = int(sin(TO_RAD(item->Pose.Orientation.y + ANGLE(90.0f))) * 10);
		int dz = int(cos(TO_RAD(item->Pose.Orientation.y + ANGLE(90.0f))) * 10);
		int height = GetFloorHeight(
			GetFloor(item->Pose.Position.x + dx, item->Pose.Position.y, item->Pose.Position.z + dz, &troomnumber),
			item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) - item->Pose.Position.y;

		if (height < CLICK(1.5f))
		{
			item->Animation.TargetState = LS_WALL_CLIMB_DISMOUNT_RIGHT;
			item->Animation.ActiveState = LS_MISC_CONTROL;
		}
	}

	// Cornering.

	item->Animation.AnimNumber = coll->Setup.PrevAnimNumber;
	item->Animation.FrameNumber = coll->Setup.PrevFrameNumber;

	AnimateItem(item);
}

static int LaraTestClimb(ItemInfo* item, int xOffset, int yOffset, int zOffset, int xFront, int zFront, int itemHeight, int* shift)
{
	auto* lara = GetLaraInfo(item);

	*shift = 0;
	int hang = 1;

	int x = item->Pose.Position.x + xOffset;
	int y = item->Pose.Position.y + yOffset;
	int z = item->Pose.Position.z + zOffset;

	auto probeUp = GetCollision(x, y - CLICK(0.5f), z, item->RoomNumber);
	auto probeDown = GetCollision(x, y, z, item->RoomNumber);

	if (/*!lara->Control.CanClimbLadder && !TestLaraNearClimbableWall(*item, probeDown.BottomBlock)*/true)
		return 0;

	int height = probeUp.Position.Floor;

	if (height == NO_HEIGHT)
		return 0;

	height -= (CLICK(0.5f) + y + itemHeight);
	if (height < -WALL_CLIMB_CLIMB_SHIFT)
		return 0;

	if (height < 0)
		*shift = height;

	int ceiling = probeDown.Position.Ceiling - y;
	if (ceiling > WALL_CLIMB_CLIMB_SHIFT)
		return 0;

	if (ceiling > 0)
	{
		if (*shift)
			return 0;
		*shift = ceiling;
	}

	if ((itemHeight + height) < 900)
		hang = 0;

	int dz = zFront + z;
	int dx = xFront + x;

	auto probeFront = GetCollision(dx, y, dz, item->RoomNumber);
	height = probeFront.Position.Floor;

	if (height != NO_HEIGHT)
		height -= y;

	if (height <= WALL_CLIMB_CLIMB_SHIFT)
	{
		if (height > 0)
		{
			if (*shift < 0)
				return 0;

			if (height > *shift)
				*shift = height;
		}

		auto probeTop = GetCollision(x, y + itemHeight, z, item->RoomNumber);
		auto probeTopFront = GetCollision(dx, y + itemHeight, dz, probeTop.RoomNumber);
		ceiling = probeTopFront.Position.Ceiling;

		if (ceiling == NO_HEIGHT)
			return 1;

		if (ceiling - y <= height)
			return 1;

		if (ceiling - y >= CLICK(2))
			return 1;

		if (ceiling - y <= 442)
			return -(hang != 0);

		if (*shift > 0)
			return -(hang != 0);

		*shift = ceiling - y - CLICK(2);
		return 1;
	}

	ceiling = probeFront.Position.Ceiling - y;
	if (ceiling >= CLICK(2))
		return 1;

	if (ceiling > CLICK(2) - WALL_CLIMB_CLIMB_SHIFT)
	{
		if (*shift > 0)
			return -(hang != 0);

		*shift = ceiling - CLICK(2);
		return 1;
	}

	if (ceiling > 0)
		return -(hang != 0);

	if (ceiling <= -WALL_CLIMB_CLIMB_SHIFT || !hang || *shift > 0)
		return 0;

	if (*shift > ceiling)
		*shift = ceiling;

	return -1;
}

static int LaraTestClimbPos(ItemInfo* item, int front, int right, int origin, int height, int* shift)
{
	int x;
	int z;
	int xFront = 0;
	int zFront = 0;

	switch (GetQuadrant(item->Pose.Orientation.y))
	{
	case NORTH:
		x = right;
		z = front;
		zFront = CLICK(1);
		break;

	case EAST:
		x = front;
		z = -right;
		xFront = CLICK(1);
		break;

	case SOUTH:
		x = -right;
		z = -front;
		zFront = -CLICK(1);
		break;

	case WEST:
	default:
		x = -front;
		z = right;
		xFront = -CLICK(1);
		break;
	}

	return LaraTestClimb(item, x, origin, z, xFront, zFront, height, shift);
}

static int LaraTestClimbUpPos(ItemInfo* item, int front, int right, int* shift, int* ledge)
{
	auto probePos = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, front, -CLICK(3), right);
	auto probeOffset = Geometry::TranslatePoint(Vector3i::Zero, item->Pose.Orientation.y, 4.0f);

	*shift = 0;

	// Test center.
	auto pointColl = GetCollision(item);
	int vPos = item->Pose.Position.y - CLICK(4);
	if ((pointColl.Position.Ceiling - vPos) > WALL_CLIMB_CLIMB_SHIFT)
		return 0;

	pointColl = GetCollision(probePos.x, probePos.y, probePos.z, item->RoomNumber);
	int ceiling = (CLICK(1) - probePos.y) + pointColl.Position.Ceiling;

	pointColl = GetCollision(probePos.x + probeOffset.x, probePos.y, probePos.z + probeOffset.z, pointColl.RoomNumber);
	int height = pointColl.Position.Floor;

	if (height == NO_HEIGHT)
	{
		*ledge = NO_HEIGHT;
	}
	else
	{
		height -= probePos.y;
		*ledge = height;
	}

	if (ceiling > WALL_CLIMB_CLIMB_SHIFT)
		return 0;

	if (ceiling > 0)
		*shift = ceiling;

	if (height == NO_HEIGHT)
	{
		return 1;
	}
	else
	{
		if (height <= CLICK(0.5f))
		{
			if (height > 0 && height > *shift)
				*shift = height;

			pointColl = GetCollision(probePos.x, probePos.y + CLICK(2), probePos.z, item->RoomNumber);
			pointColl = GetCollision(probePos.x + probeOffset.x, probePos.y + CLICK(2), probePos.z + probeOffset.z, pointColl.RoomNumber);

			ceiling = pointColl.Position.Ceiling - probePos.y;
			if (ceiling <= height)
				return 1;

			if (ceiling >= CLICK(2))
				return 1;					
			else
				return 0;
		}
		else
		{
			ceiling = GetCollision(probePos.x + probeOffset.x, probePos.y, probePos.z + probeOffset.z, pointColl.RoomNumber).Position.Ceiling - probePos.y;
			if (ceiling < CLICK(2))
			{
				if ((height - ceiling) <= LARA_HEIGHT)
				{
					if ((height - ceiling) < CLICK(2))
						return 0;

					*shift = height;
					return -2;
				}
				else
				{
					*shift = height;
					return -1;
				}
			}
			else
			{
				return 1;
			}
		}
	}

	return -2;
}

static bool LaraCheckForLetGo(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0.0f;

	if ((IsHeld(In::Action) && item->HitPoints > 0) ||
		item->Animation.AnimNumber == LA_ONWATER_TO_LADDER) // HACK: Can't release on this anim.
	{
		return false;
	}

	SetPlayerEdgeHangRelease(*item);
	ResetPlayerFlex(item);
	return true;
}

// -----------------------------
// WALL CLIMB
// Control & Collision Functions
// -----------------------------

void lara_col_wall_climb_end(ItemInfo* item, CollisionInfo* coll)
{
	return;
}

void lara_as_wall_climb_end(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(-45.0f);
}

void lara_col_wall_climb_down(ItemInfo* item, CollisionInfo* coll)
{
	if (LaraCheckForLetGo(item, coll) || !TestAnimNumber(*item, LA_WALL_CLIMB_DOWN))
		return;

	int shiftLeft = 0;
	int shiftRight = 0;
	int resultRight = LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE, -CLICK(2), CLICK(2), &shiftRight);
	int resultLeft = LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE), -CLICK(2), CLICK(2), &shiftLeft);

	if (IsHeld(In::Back) &&
		resultRight != 0 && resultLeft != 0 &&
		resultRight != -2 && resultLeft != -2)
	{
		if (shiftRight && shiftLeft)
		{
			if (shiftRight < 0 != shiftLeft < 0)
			{
				item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
				return;
			}

			if (shiftRight < 0 && shiftRight < shiftLeft ||
				shiftRight > 0 && shiftRight > shiftLeft)
			{
				shiftLeft = shiftRight;
			}
		}

		if (resultRight == -1 || resultLeft == -1)
		{
			SetAnimation(item, LA_WALL_CLIMB_IDLE);
			item->Animation.TargetState = LS_EDGE_HANG_IDLE;
		}
		else
		{
			item->Animation.TargetState = LS_WALL_CLIMB_DOWN;
		}

		return;
	}

	item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
}

void lara_as_wall_climb_down(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = ANGLE(-45.0f);

	// Overhang hook.
	//SlopeClimbDownExtra(item, coll);
}

void lara_col_wall_climb_up(ItemInfo* item, CollisionInfo* coll)
{
	if (LaraCheckForLetGo(item, coll) && TestAnimNumber(*item, LA_WALL_CLIMB_UP))
		return;

	int shiftRight = 0;
	int shiftLeft = 0;
	int ledgeRight = 0;
	int ledgeLeft = 0;
	int resultRight = LaraTestClimbUpPos(item, coll->Setup.Radius, coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE, &shiftRight, &ledgeRight);
	int resultLeft = LaraTestClimbUpPos(item, coll->Setup.Radius, -(coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE), &shiftLeft, &ledgeLeft);

	if (IsHeld(In::Forward) && resultRight && resultLeft)
	{
		if (resultRight < 0 || resultLeft < 0)
		{
			item->Animation.TargetState = LS_WALL_CLIMB_IDLE;

			if (abs(ledgeRight - ledgeLeft) <= WALL_CLIMB_TEST_DISTANCE)
			{
				if (resultRight != -1 || resultLeft != -1)
				{
					item->Animation.TargetState = LS_WALL_CLIMB_TO_CROUCH;
					item->Animation.RequiredState = LS_CROUCH_IDLE;
				}
				else
				{
					item->Animation.TargetState = LS_GRABBING;
					item->Pose.Position.y += (ledgeRight + ledgeLeft) / 2 - CLICK(1);
				}
			}
		}
		else
		{
			item->Animation.TargetState = LS_WALL_CLIMB_UP;
		}
	}
	else
	{
		item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
	}
}

void lara_as_wall_climb_up(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = ANGLE(30.0f);
}

void lara_col_wall_climb_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!LaraCheckForLetGo(item, coll))
	{
		int shift = 0;
		lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
		LaraDoClimbLeftRight(item, coll, LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE, -CLICK(2), CLICK(2), &shift), shift);
	}
}

void lara_as_wall_climb_right(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(30.0f);
	Camera.targetElevation = ANGLE(-15.0f);

	if (!(IsHeld(In::Right) || IsHeld(In::StepRight)))
		item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
}

void lara_col_wall_climb_left(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!LaraCheckForLetGo(item, coll))
	{
		int shift = 0;
		lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
		LaraDoClimbLeftRight(item, coll, LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE), -CLICK(2), CLICK(2), &shift), shift);
	}
}

void lara_as_wall_climb_left(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(-30.0f);
	Camera.targetElevation = ANGLE(-15.0f);

	if (!(IsHeld(In::Left) || IsHeld(In::StepLeft)))
		item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
}

void lara_col_wall_climb_idle(ItemInfo* item, CollisionInfo* coll)
{
	int yShift;
	int resultRight, resultLeft;
	int ledgeRight, ledgeLeft;

	if (LaraCheckForLetGo(item, coll) || item->Animation.AnimNumber != LA_WALL_CLIMB_IDLE)
		return;

	if (!IsHeld(In::Forward))
	{
		if (!IsHeld(In::Back))
			return;

		if (item->Animation.TargetState == LS_EDGE_HANG_IDLE)
			return;

		item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
		item->Pose.Position.y += CLICK(1);
		
		resultRight = LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE, -CLICK(2), CLICK(2), &ledgeRight);
		resultLeft = LaraTestClimbPos(item, coll->Setup.Radius, -WALL_CLIMB_TEST_DISTANCE - coll->Setup.Radius, -CLICK(2), CLICK(2), &ledgeLeft);
		
		item->Pose.Position.y -= CLICK(1);
		
		if (!resultRight || !resultLeft || resultLeft == -2 || resultRight == -2)
			return;

		yShift = ledgeLeft;

		if (ledgeRight && ledgeLeft)
		{
			if (ledgeLeft < 0 != ledgeRight < 0)
				return;
			if (ledgeRight < 0 == ledgeRight < ledgeLeft)
				yShift = ledgeRight;
		}

		if (resultRight == 1 && resultLeft == 1)
		{
			item->Animation.TargetState = LS_WALL_CLIMB_DOWN;
			item->Pose.Position.y += yShift;
		}
		else
			item->Animation.TargetState = LS_EDGE_HANG_IDLE;
	}
	else if (item->Animation.TargetState != LS_GRABBING)
	{
		int shiftRight, shiftLeft;

		item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
		resultRight = LaraTestClimbUpPos(item, coll->Setup.Radius, coll->Setup.Radius + WALL_CLIMB_TEST_DISTANCE, &shiftRight, &ledgeRight);
		resultLeft = LaraTestClimbUpPos(item, coll->Setup.Radius, -WALL_CLIMB_TEST_DISTANCE - coll->Setup.Radius, &shiftLeft, &ledgeLeft);

		// Overhang + ladder-to-monkey hook.
		if (!resultRight || !resultLeft)
		{
			if (LadderMonkeyExtra(item, coll))
				return;
		}

		// Added check to avoid climbing through bridges.
		if (resultRight == 0 && resultLeft == 0)
			return;

		if (resultRight >= 0 && resultLeft >= 0)
		{
			yShift = shiftLeft;

			if (shiftRight)
			{
				if (shiftLeft)
				{
					if (shiftLeft < 0 != shiftRight < 0)
						return;
					if (shiftRight < 0 == shiftRight < shiftLeft)
						yShift = shiftRight;
				}
				else
					yShift = shiftRight;
			}

			// HACK: Prevent climbing inside sloped ceilings. Breaks overhang even more, but that shouldn't matter since we'll be doing it over. -- Sezz 2022.05.13
			int y = item->Pose.Position.y - (coll->Setup.Height + CLICK(0.5f));
			auto probe = GetCollision(item, 0, 0, -(coll->Setup.Height + CLICK(0.5f)));
			if ((probe.Position.Ceiling - y) < 0)
			{
				item->Animation.TargetState = LS_WALL_CLIMB_UP;
				item->Pose.Position.y += yShift;
			}
		}
		else if (abs(ledgeLeft - ledgeRight) <= WALL_CLIMB_TEST_DISTANCE)
		{
			if (resultRight == -1 && resultLeft == -1)
			{
				item->Animation.TargetState = LS_GRABBING;
				item->Pose.Position.y += (ledgeRight + ledgeLeft) / 2 - CLICK(1);
			}
			else
			{
				item->Animation.TargetState = LS_WALL_CLIMB_TO_CROUCH;
				item->Animation.RequiredState = LS_CROUCH_IDLE;
			}
		}
	}
}

void lara_as_wall_climb_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Look.Mode = LookMode::Free;
	lara->Control.IsClimbingLadder = true;
	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;
	Camera.targetElevation = ANGLE(-20.0f);

	if (item->Animation.AnimNumber == LA_WALL_CLIMB_DISMOUNT_LEFT_START)
		Camera.targetAngle = ANGLE(-60.0f);

	if (item->Animation.AnimNumber == LA_WALL_CLIMB_DISMOUNT_RIGHT_START)
		Camera.targetAngle = ANGLE(60.0f);

	if (IsHeld(In::Left) || IsHeld(In::StepLeft))
	{
		item->Animation.TargetState = LS_WALL_CLIMB_LEFT;
		lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
	}
	else if (IsHeld(In::Right) || IsHeld(In::StepRight))
	{
		item->Animation.TargetState = LS_WALL_CLIMB_RIGHT;
		lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
	}
	else if (IsHeld(In::Jump))
	{
		if (item->Animation.AnimNumber == LA_WALL_CLIMB_IDLE)
		{
			item->Animation.TargetState = LS_JUMP_BACK;
			lara->Control.HandStatus = HandStatus::Free;
			lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
		}
	}

	// Overhang hook.
	SlopeClimbExtra(item, coll);
}

void lara_as_wall_climb_dismount_left(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(-60.0f);
	Camera.targetElevation = ANGLE(-15.0f);

	item->Pose.Orientation.y -= ANGLE(90.0f);
}

void lara_as_wall_climb_dismount_right(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(60.0f);
	Camera.targetElevation = ANGLE(-15.0f);

	item->Pose.Orientation.y += ANGLE(90.0f);
}
