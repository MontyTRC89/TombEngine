#include "framework.h"
#include "Game/Lara/lara_climb.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/PlayerContext.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Entities::Player;
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

void lara_as_wall_climb_right(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(30.0f);
	Camera.targetElevation = ANGLE(-15.0f);

	if (!(IsHeld(In::Right) || IsHeld(In::StepRight)))
		item->Animation.TargetState = LS_WALL_CLIMB_IDLE;
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
