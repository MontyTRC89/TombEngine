#include "framework.h"
#include "Game/Lara/lara_objects.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Objects/Generic/Object/rope.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Entities::Generic;

// -----------------------------------
// MISCELLANEOUS INTERACTABLE OBJECTS
// State Control & Collision Functions
// -----------------------------------

// ------
// PICKUP
// ------

// State:		LS_PICKUP (39), LS_MISC_CONTROL (89)
// Collision:	lara_default_col()
void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = -ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = SECTOR(1);

	if (TestLastFrame(item))
		item->TargetState = GetNextAnimState(item);
}

// State:		LS_PICKUP_FLARE (67)
// Collision:	lara_default_col()
void lara_as_pickup_flare(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = SECTOR(1);

	if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 1)
		lara->Control.HandStatus = HandStatus::Free;
}

// ------
// SWITCH
// ------

// State:		LS_SWITCH_DOWN (40), LS_DOVE_SWITCH (126)
// Collision:	lara_default_col()
void lara_as_switch_on(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);
	Camera.speed = 6;
}

// State:		LS_SWITCH_DOWN (40), LS_DOVE_SWITCH (126)
// Collision:	lara_default_col()
void lara_as_switch_off(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);
	Camera.speed = 6;
}

// State:	LS_ROUND_HANDLE (95)
// Control:	lara_as_controlled_no_look()
void lara_col_turn_switch(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->Setup.OldPosition.x != item->Position.xPos || coll->Setup.OldPosition.z != item->Position.zPos)
	{
		if (item->AnimNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_CONTINUE)
		{
			item->Position.yRot -= ANGLE(90.0f);
			item->AnimNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}

		if (item->AnimNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_CONTINUE)
		{
			item->Position.yRot += ANGLE(90.0f);
			item->AnimNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_END;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
}

// ----------
// RECEPTACLE
// ----------

// State:		LS_USE_KEY (42)
// Collision:	lara_default_col()
void lara_as_use_key(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);
}

// State:		LS_USE_PUZZLE (43)
// Collision:	lara_default_col()
void lara_as_use_puzzle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = SECTOR(1);

	if (TestLastFrame(item) && item->ItemFlags[0])
	{
		item->ActiveState = LS_MISC_CONTROL;
		item->AnimNumber = item->ItemFlags[0];
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	}
}

// --------
// PUSHABLE
// --------

// State:		LS_PUSHABLE_PUSH (36)
// Collision:	lara_default_col()
void lara_as_pushable_push(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_TORSO;
}

// State:		LS_PUSHABLE_PULL (37)
// Collision:	lara_default_col()
void lara_as_pushable_pull(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_TORSO;
}

// State:		LS_PUSHABLE_GRAB (38)
// Collision:	lara_default_col()
void lara_as_pushable_grab(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(75.0f);

	if (!(TrInput & IN_ACTION))
		item->TargetState = LS_IDLE;
}

// ------
// PULLEY
// ------

// State:		LS_PULLEY (104)
// Collision:	lara_default_col()
void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);
	auto* pulleyItem = &g_Level.Items[lara->interactedItem];

	lara->Control.CanLook = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	if (TrInput & IN_ACTION && pulleyItem->TriggerFlags)
		item->TargetState = LS_PULLEY;
	else
		item->TargetState = LS_IDLE;

	if (item->AnimNumber == LA_PULLEY_PULL &&
		item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 44)
	{
		if (pulleyItem->TriggerFlags)
		{
			if (!pulleyItem->ItemFlags[1])
			{
				pulleyItem->TriggerFlags--;
				if (pulleyItem->TriggerFlags)
				{
					if (pulleyItem->ItemFlags[2])
					{
						pulleyItem->ItemFlags[2] = 0;
						pulleyItem->Status = ITEM_DEACTIVATED;
					}
				}
				else
				{
					pulleyItem->Status = ITEM_DEACTIVATED;
					pulleyItem->ItemFlags[2] = 1;

					if (pulleyItem->ItemFlags[3] >= 0)
						pulleyItem->TriggerFlags = abs(pulleyItem->ItemFlags[3]);
					else
						pulleyItem->ItemFlags[0] = 1;
				}
			}
		}
	}

	if (item->AnimNumber == LA_PULLEY_RELEASE &&
		item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 1)
	{
		lara->Control.HandStatus = HandStatus::Free;
	}
}

// --------------
// HORIZONTAL BAR
// --------------

// State:		LS_HORIZONTAL_BAR_SWING (128)
// Collision:	lara_default_col()
void lara_as_horizontal_bar_swing(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || TrInput & IN_JUMP)
		item->TargetState = LS_HORIZONTAL_BAR_LEAP;
}

// State:		LS_HORIZONTAL_BAR_LEAP (129)
// Collision:	lara_default_col()
void lara_as_horizontal_bar_leap(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);
	auto* barItem = &g_Level.Items[lara->interactedItem];

	item->Airborne = true;

	if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
	{
		int distance;
		if (item->Position.yRot == barItem->Position.yRot)
			distance = (barItem->TriggerFlags / 100) - 2;
		else
			distance = (barItem->TriggerFlags % 100) - 2;

		item->Velocity = (20 * distance) + 58;
		item->VerticalVelocity = -(20 * distance + 64);
	}

	if (TestLastFrame(item))
	{
		SetAnimation(item, LA_REACH);
		item->Position.xPos += 700 * phd_sin(item->Position.yRot);
		item->Position.yPos -= 361;
		item->Position.zPos += 700 * phd_cos(item->Position.yRot);
	}
}

// ---------
// TIGHTROPE
// ---------

#ifdef NEW_TIGHTROPE
// State:		LS_TIGHTROPE_IDLE (119)
// Collision:	lara_default_col()
void lara_as_tightrope_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LOOK)
		LookUpDown(item);

	DoLaraTightropeBalanceRegen(item);
	DoLaraTightropeLean(item);

	if (TrInput & IN_FORWARD)
		item->TargetState = LS_TIGHTROPE_WALK;
	else if (TrInput & (IN_ROLL | IN_BACK))
	{
		if (item->AnimNumber == LA_TIGHTROPE_IDLE)
		{
			item->ActiveState = LS_TIGHTROPE_TURN_180;
			item->AnimNumber = LA_TIGHTROPE_TURN_180;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
}

// State:		LS_TIGHTROPE_DISMOUNT (125)
// Collision:	lara_default_col()
void lara_as_tightrope_dismount(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	DoLaraTightropeBalanceRegen(item);
	DoLaraTightropeLean(item);

	if (item->AnimNumber == LA_TIGHTROPE_END &&
		TestLastFrame(item))
	{
		item->Position.zRot = 0;
		lara->ExtraTorsoRot.zRot = 0;
	}
}

// State:		LS_TIGHTROPE_WALK (121)
// Collision:	lara_default_col()
void lara_as_tightrope_walk(ITEM_INFO* item, COLL_INFO* coll) 
{
	auto* lara = GetLaraInfo(item);

	auto probe = GetCollisionResult(item);
	if (probe.Position.Floor == item->Position.yPos &&
		lara->Control.TightropeControl.CanDismount)
	{
		item->TargetState = LS_TIGHTROPE_DISMOUNT;
		DoLaraTightropeBalanceRegen(item);
	}
	
	if ((TrInput & (IN_BACK | IN_ROLL) || !(TrInput & IN_FORWARD)) &&
		item->TargetState != LS_TIGHTROPE_DISMOUNT)
	{
		item->TargetState = LS_TIGHTROPE_IDLE;
	}

	lara->Control.TightropeControl.TimeOnTightrope++;
	DoLaraTightropeBalance(item);
	DoLaraTightropeLean(item);

	if (lara->Control.TightropeControl.Balance >= 8000)
		SetAnimation(item, LA_TIGHTROPE_FALL_RIGHT);
	else if (lara->Control.TightropeControl.Balance <= -8000)
		SetAnimation(item, LA_TIGHTROPE_FALL_LEFT);
}

// State:		TIGHTROPE_UNBALANCE_LEFT (122), TIGHTROPE_UNBALANCE_RIGHT (123)
// Collision:	lara_default_col()
void lara_as_tightrope_fall(ITEM_INFO* item, COLL_INFO* coll)
{
	DoLaraTightropeBalanceRegen(item);
	DoLaraTightropeLean(item);

	if (TestLastFrame(item))
	{
		PHD_VECTOR pos = { 0, 75, 0 };
		GetLaraJointPosition(&pos, LM_RFOOT);

		item->Position.xPos = pos.x;
		item->Position.yPos = pos.y;
		item->Position.zPos = pos.z;

		item->TargetState = LS_FREEFALL;
		item->ActiveState = LS_FREEFALL;
		item->AnimNumber = LA_FREEFALL;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

		item->VerticalVelocity = 81;
		Camera.targetspeed = 16;
	}
}

#else
// State:		LS_TIGHTROPE_IDLE (119)
// Collision:	lara_default_col()
void lara_as_tightrope_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LOOK)
		LookUpDown(item);

	GetTightropeFallOff(item, 127);

	if (item->ActiveState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (lara->Control.TightropeControl.Fall)
		{
			if (GetRandomControl() & 1)
				item->TargetState = LS_TIGHTROPE_UNBALANCE_RIGHT;
			else
				item->TargetState = LS_TIGHTROPE_UNBALANCE_LEFT;
		}
		else
		{
			if (TrInput & IN_FORWARD)
				item->TargetState = LS_TIGHTROPE_WALK;
			else if (TrInput & (IN_ROLL | IN_BACK))
			{
				if (item->AnimNumber == LA_TIGHTROPE_IDLE)
				{
					item->ActiveState = LS_TIGHTROPE_TURN_180;
					item->AnimNumber = LA_TIGHTROPE_TURN_180;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					GetTightropeFallOff(item, 1);
				}
			}
		}
	}
}

// State:		LS_TIGHTROPE_WALK (121)
// Collision:	lara_default_col()
void lara_as_tightrope_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	if (lara->Control.TightropeControl.OnCount)
		lara->Control.TightropeControl.OnCount--;
	else if (lara->Control.TightropeControl.Off)
	{
		short roomNumber = item->RoomNumber;

		if (GetFloorHeight(GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber),
			item->Position.xPos, item->Position.yPos, item->Position.zPos) == item->Position.yPos)
		{
			lara->Control.TightropeControl.Off = 0;
			item->TargetState = LS_TIGHTROPE_DISMOUNT;
		}
	}
	else
		GetTightropeFallOff(item, 127);

	if (item->ActiveState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (TrInput & IN_LOOK)
			LookUpDown(item);

		if (((TrInput & (IN_BACK | IN_ROLL) || !(TrInput & IN_FORWARD) || lara->Control.TightropeControl.Fall) &&
			!lara->Control.TightropeControl.OnCount &&
			!lara->Control.TightropeControl.Off) &&
			item->TargetState != LS_TIGHTROPE_DISMOUNT)
		{
			item->TargetState = LS_TIGHTROPE_IDLE;
		}
	}
}

// State:		TIGHTROPE_UNBALANCE_LEFT (122), TIGHTROPE_UNBALANCE_RIGHT (123)
// Collision:	lara_default_col()
void lara_as_tightrope_fall(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->AnimNumber == LA_TIGHTROPE_FALL_LEFT || item->AnimNumber == LA_TIGHTROPE_FALL_RIGHT)
	{
		if (TestLastFrame(item, item->AnimNumber))
		{
			PHD_VECTOR pos = { 0, 0, 0 };
			GetLaraJointPosition(&pos, LM_RFOOT);

			item->Position.xPos = pos.x;
			item->Position.yPos = pos.y + 75;
			item->Position.zPos = pos.z;

			item->TargetState = LS_FREEFALL;
			item->ActiveState = LS_FREEFALL;
			item->AnimNumber = LA_FREEFALL;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

			item->VerticalVelocity = 81;
			Camera.targetspeed = 16;
		}
	}
	else
	{
		int undoInput, wrongInput;
		int undoAnim, undoFrame;

		if (lara->Control.TightropeControl.OnCount > 0)
			lara->Control.TightropeControl.OnCount--;

		if (item->AnimNumber == LA_TIGHTROPE_UNBALANCE_LEFT)
		{
			undoInput = IN_RIGHT;
			wrongInput = IN_LEFT;
			undoAnim = LA_TIGHTROPE_RECOVER_LEFT;
		}
		else if (item->AnimNumber == LA_TIGHTROPE_UNBALANCE_RIGHT)
		{
			undoInput = IN_LEFT;
			wrongInput = IN_RIGHT;
			undoAnim = LA_TIGHTROPE_RECOVER_RIGHT;
		}
		else
			return;

		undoFrame = g_Level.Anims[item->AnimNumber].frameEnd + g_Level.Anims[undoAnim].frameBase - item->FrameNumber;

		if (TrInput & undoInput && lara->Control.TightropeControl.OnCount == 0)
		{
			item->ActiveState = LS_TIGHTROPE_RECOVER_BALANCE;
			item->TargetState = LS_TIGHTROPE_IDLE;
			item->AnimNumber = undoAnim;
			item->FrameNumber = undoFrame;

			lara->Control.TightropeControl.Fall--;
		}
		else
		{
			if (TrInput & wrongInput)
			{
				if (lara->Control.TightropeControl.OnCount < 10)
					lara->Control.TightropeControl.OnCount += (GetRandomControl() & 3) + 2;
			}
		}
	}
}
#endif

// ----
// ROPE
// ----

// State:		LS_ROPE_TURN_CLOCKWISE (90)
// Collision:	lara_void_func()
void lara_as_rope_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
			lara->Control.RopeControl.Y += ANGLE(1.4f);
		else
			item->TargetState = LS_ROPE_IDLE;
	}
	else
		FallFromRope(item);
}

// State:		LS_ROPE_TURN_COUNTER_CLOCKWISE (91)
// Collision:	lara_void_func()
void lara_as_rope_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_RIGHT)
			lara->Control.RopeControl.Y -= ANGLE(1.4f);
		else
			item->TargetState = LS_ROPE_IDLE;
	}
	else
		FallFromRope(item);
}

// State:		LS_ROPE_IDLE (111), LS_ROPE_SWING (114), LS_ROPE_UNKNOWN (115)
// Collision:	lara_vcol_rope_idle() (111), lara_col_rope_swing() (114, 115)
void lara_as_rope_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION))
		FallFromRope(item);

	if (TrInput & IN_LOOK)
		LookUpDown(item);
}

// State:	LS_ROPE_IDLE (111)
// Control:	lara_as_rope_idle()
void lara_col_rope_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_ACTION)
	{
		UpdateRopeSwing(item);

		if (TrInput & IN_SPRINT)
		{
			item->TargetState = LS_ROPE_SWING;
			lara->Control.RopeControl.DFrame = (g_Level.Anims[LA_ROPE_SWING].frameBase + 32) << 8;
			lara->Control.RopeControl.Frame = lara->Control.RopeControl.DFrame;
		}
		else if (TrInput & IN_FORWARD && lara->Control.RopeControl.Segment > 4)
			item->TargetState = LS_ROPE_UP;
		else if (TrInput & IN_BACK && lara->Control.RopeControl.Segment < 21)
		{
			item->TargetState = LS_ROPE_DOWN;
			lara->Control.RopeControl.Flag = 0;
			lara->Control.RopeControl.Count = 0;
		}
		else if (TrInput & IN_LEFT)
			item->TargetState = LS_ROPE_TURN_CLOCKWISE;
		else if (TrInput & IN_RIGHT)
			item->TargetState = LS_ROPE_TURN_COUNTER_CLOCKWISE;
	}
	else
		FallFromRope(item);
}

// State:	LS_ROPE_SWING (114), LS_ROPE_UNKNOWN (115)
// Control:	lara_as_rope_idle()
void lara_col_rope_swing(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	Camera.targetDistance = SECTOR(2);

	UpdateRopeSwing(item);

	if (item->AnimNumber == LA_ROPE_SWING)
	{
		if (TrInput & IN_SPRINT)
		{
			int velocity;

			if (abs(lara->Control.RopeControl.LastX) < 9000)
				velocity = 192 * (9000 - abs(lara->Control.RopeControl.LastX)) / 9000;
			else
				velocity = 0;

			ApplyVelocityToRope(
				lara->Control.RopeControl.Segment - 2,
				item->Position.yRot + (lara->Control.RopeControl.Direction ? 0 : ANGLE(180.0f)),
				velocity >> 5);
		}

		if (lara->Control.RopeControl.Frame > lara->Control.RopeControl.DFrame)
		{
			lara->Control.RopeControl.Frame -= (unsigned short)lara->Control.RopeControl.FrameRate;
			if (lara->Control.RopeControl.Frame < lara->Control.RopeControl.DFrame)
				lara->Control.RopeControl.Frame = lara->Control.RopeControl.DFrame;
		}
		else if (lara->Control.RopeControl.Frame < lara->Control.RopeControl.DFrame)
		{
			lara->Control.RopeControl.Frame += (unsigned short)lara->Control.RopeControl.FrameRate;
			if (lara->Control.RopeControl.Frame > lara->Control.RopeControl.DFrame)
				lara->Control.RopeControl.Frame = lara->Control.RopeControl.DFrame;
		}

		item->FrameNumber = lara->Control.RopeControl.Frame >> 8;

		if (!(TrInput & IN_SPRINT) &&
			item->FrameNumber == g_Level.Anims[LA_ROPE_SWING].frameBase + 32 &&
			lara->Control.RopeControl.MaxXBackward < 6750 &&
			lara->Control.RopeControl.MaxXForward < 6750)
		{
			item->TargetState = LS_ROPE_IDLE;
			item->ActiveState = LS_ROPE_IDLE;
			item->AnimNumber = LA_JUMP_UP_TO_ROPE_END;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}

		if (TrInput & IN_JUMP)
			JumpOffRope(item);
	}
	else if (item->FrameNumber == g_Level.Anims[LA_ROPE_IDLE_TO_SWING].frameBase + 15)
		ApplyVelocityToRope(lara->Control.RopeControl.Segment, item->Position.yRot, 128);
}

// State:	LS_ROPE_UP (112)
// Control:	lara_void_func()
void lara_as_rope_up(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_ROLL)
		FallFromRope(item);
	else
	{
		Camera.targetAngle = ANGLE(30.0f);

		if (g_Level.Anims[item->AnimNumber].frameEnd == item->FrameNumber)
		{
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			lara->Control.RopeControl.Segment -= 2;
		}

		if (!(TrInput & IN_FORWARD) || lara->Control.RopeControl.Segment <= 4)
			item->TargetState = LS_ROPE_IDLE;
	}
}

// State:	LS_ROPE_DOWN (113)
// Control:	lara_void_func()
void lara_as_rope_down(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraClimbRope(item, coll);
}

// -------------
// VERTICAL POLE
// -------------

// State:		LS_POLE_IDLE (99)
// Collision:	lara_col_pole_idle()
void lara_as_pole_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_FREEFALL; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TrInput & IN_ACTION)
	{
		if (item->ActiveState == LA_POLE_IDLE) // Hack.
		{
			if (TrInput & IN_LEFT)
			{
				lara->Control.TurnRate += LARA_POLE_TURN_RATE;
				if (lara->Control.TurnRate > LARA_POLE_TURN_MAX)
					lara->Control.TurnRate = LARA_POLE_TURN_MAX;
			}
			else if (TrInput & IN_RIGHT)
			{
				lara->Control.TurnRate -= LARA_POLE_TURN_RATE;
				if (lara->Control.TurnRate < -LARA_POLE_TURN_MAX)
					lara->Control.TurnRate = -LARA_POLE_TURN_MAX;
			}
		}

		// TODO: Add forward jump.
		if (TrInput & IN_JUMP)
		{
			item->TargetState = LS_JUMP_BACK;
			return;
		}

		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->TargetState = LS_POLE_UP;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->TargetState = LS_POLE_DOWN;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->TargetState = LS_POLE_TURN_CLOCKWISE;
			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->TargetState = LS_POLE_TURN_COUNTER_CLOCKWISE;
			return;
		}

		item->TargetState = LS_POLE_IDLE;
		return;
	}

	GetCollisionInfo(coll, item); // HACK: Lara may step off poles in mid-air upon reload without this.
	if (coll->Middle.Floor <= 0 &&
		item->AnimNumber != LA_POLE_JUMP_BACK) // Hack.
	{
		item->TargetState = LS_IDLE;
		return;
	}
	else if (item->AnimNumber == LA_POLE_IDLE)
	{
		item->TargetState = LS_FREEFALL;

		// TODO: This shouldn't be required, but the set position command doesn't move Lara correctly.
		item->Position.xPos -= phd_sin(item->Position.yRot) * 64;
		item->Position.zPos -= phd_cos(item->Position.yRot) * 64;
	}
}

// State:		LS_POLE_IDLE (99)
// Control:		lara_as_pole_idle()
void lara_col_pole_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.FloorSlopeIsWall = true;
	GetCollisionInfo(coll, item);

	// TODO: There's a visible snap if Lara hits the ground at a high velocity.
	if (coll->Middle.Floor < 0)
		item->Position.yPos += coll->Middle.Floor;
}

// State:		LS_POLE_UP (100)
// Collision:	lara_col_pole_up()
void lara_as_pole_up(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
		{
			lara->Control.TurnRate += LARA_POLE_TURN_RATE;
			if (lara->Control.TurnRate > LARA_POLE_TURN_MAX)
				lara->Control.TurnRate = LARA_POLE_TURN_MAX;
		}
		else if (TrInput & IN_RIGHT)
		{
			lara->Control.TurnRate -= LARA_POLE_TURN_RATE;
			if (lara->Control.TurnRate < -LARA_POLE_TURN_MAX)
				lara->Control.TurnRate = -LARA_POLE_TURN_MAX;
		}

		if (TrInput & IN_JUMP)
		{
			item->TargetState = LS_POLE_IDLE;
			return;
		}

		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->TargetState = LS_POLE_UP;
			return;
		}

		item->TargetState = LS_POLE_IDLE;
		return;
	}

	item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to freefall?
}

// State:		LS_POLE_UP (100)
// Control:		lara_as_pole_up()
void lara_col_pole_up(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_pole_idle(item, coll);
}

// State:		LS_POLE_DOWN (101)
// Collision:	lara_col_pole_down()
void lara_as_pole_down(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	// TODO: In WAD.
	SoundEffect(SFX_TR4_LARA_POLE_LOOP, &item->Position, 0);

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
		{
			lara->Control.TurnRate += LARA_POLE_TURN_RATE;
			if (lara->Control.TurnRate > LARA_POLE_TURN_MAX)
				lara->Control.TurnRate = LARA_POLE_TURN_MAX;
		}
		else if (TrInput & IN_RIGHT)
		{
			lara->Control.TurnRate -= LARA_POLE_TURN_RATE;
			if (lara->Control.TurnRate < -LARA_POLE_TURN_MAX)
				lara->Control.TurnRate = -LARA_POLE_TURN_MAX;
		}

		if (TrInput & IN_JUMP)
		{
			item->TargetState = LS_POLE_IDLE;
			return;
		}

		if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->TargetState = LS_POLE_DOWN;
			return;
		}

		item->TargetState = LS_POLE_IDLE;
		item->VerticalVelocity = 0;
		return;
	}

	item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to freefall?
}

// State:		LS_POLE_DOWN (101)
// Control:		lara_as_pole_down()
void lara_col_pole_down(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	GetCollisionInfo(coll, item);

	// TODO: Pitch modulation might be a fun idea.

	if (item->AnimNumber == LA_POLE_DOWN_END)
		item->VerticalVelocity -= 8;
	else
		item->VerticalVelocity += 1;

	if (item->VerticalVelocity < 0)
		item->VerticalVelocity = 0;
	else if (item->VerticalVelocity > 64)
		item->VerticalVelocity = 64;
	
	// TODO: Do something about that ugly snap at the bottom.
	if (coll->Middle.Floor + item->VerticalVelocity < 0)
		item->Position.yPos += coll->Middle.Floor;
	else if (TestLaraPoleCollision(item, coll, false))
		item->Position.yPos += item->VerticalVelocity;
}

// State:		LS_POLE_TURN_CLOCKWISE (102)
// Collision:	lara_col_pole_turn_clockwise()
void lara_as_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to climp up.
			return;
		}
		else if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to climb down.
			return;
		}

		if (TrInput & IN_LEFT)
		{
			lara->Control.TurnRate += LARA_POLE_TURN_RATE;
			if (lara->Control.TurnRate > LARA_POLE_TURN_MAX)
				lara->Control.TurnRate = LARA_POLE_TURN_MAX;

			item->TargetState = LS_POLE_TURN_CLOCKWISE;
			return;
		}

		item->TargetState = LS_POLE_IDLE;
		return;
	}

	item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to freefall.
}

// State:		LS_POLE_TURN_CLOCKWISE (102)
// Control:		lara_as_pole_turn_clockwise()
void lara_col_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_pole_idle(item, coll);
}

// State:		LS_POLE_TURN_COUNTER_CLOCKWISE (103)
// Collision:	lara_col_pole_turn_counter_clockwise()
void lara_as_pole_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to climb up.
			return;
		}
		else if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to climb down.
			return;
		}

		if (TrInput & IN_RIGHT)
		{
			lara->Control.TurnRate -= LARA_POLE_TURN_RATE;
			if (lara->Control.TurnRate < -LARA_POLE_TURN_MAX)
				lara->Control.TurnRate = -LARA_POLE_TURN_MAX;

			item->TargetState = LS_POLE_TURN_COUNTER_CLOCKWISE;
			return;
		}

		item->TargetState = LS_POLE_IDLE;
		return;
	}

	item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to freefall.
}

// State:		LS_POLE_TURN_COUNTER_CLOCKWISE (103)
// Control:		lara_col_pole_turn_counter_clockwise()
void lara_col_pole_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_pole_idle(item, coll);
}

// --------
// ZIP-LINE
// --------

// State:		LS_ZIP_LINE (70)
// Collision:	lara_void_func()
void lara_as_zip_line(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	Camera.targetAngle = ANGLE(70.0f);

	if (!(TrInput & IN_ACTION))
	{
		item->TargetState = LS_JUMP_FORWARD;
		AnimateLara(item);

		item->Airborne = true;
		item->Velocity = 100;
		item->VerticalVelocity = 40;
		lara->Control.MoveAngle = item->Position.yRot;
	}
}
