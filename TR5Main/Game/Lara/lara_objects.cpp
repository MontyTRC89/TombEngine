#include "framework.h"
#include "Game/Lara/lara_objects.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
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

void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 39, 98*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = -ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = WALL_SIZE;

	if (TestLastFrame(item))
		item->TargetState = GetNextAnimState(item);
}

void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 67*/
	/*collison: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = WALL_SIZE;

	if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 1)
		info->gunStatus = LG_HANDS_FREE;
}

// ------
// SWITCH
// ------

void lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*states 40, 126*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;
	Camera.speed = 6;
}

void lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 41*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;
	Camera.speed = 6;
}

void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 95*/
	/*state code: lara_as_controlled_no_look*/
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

void lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 42*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;
}

void lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 43*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;

	if (TestLastFrame(item))
	{
		if (item->ItemFlags[0])
		{
			item->AnimNumber = item->ItemFlags[0];
			item->ActiveState = LS_MISC_CONTROL;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
}

// --------
// PUSHABLE
// --------

void lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 36*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.laraNode = LM_TORSO;
}

void lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 37*/
	/*collision: lara_default_col*/
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.laraNode = LM_TORSO;
}

void lara_as_ppready(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 38*/
	/*collision: lara_default_col*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = ANGLE(75.0f);

	if (!(TrInput & IN_ACTION))
		item->TargetState = LS_IDLE;
}

// ------
// PULLEY
// ------

void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 104*/
	/*collision: lara_default_col*/
	LaraInfo*& info = item->Data;
	ITEM_INFO* pulley = &g_Level.Items[info->interactedItem];

	info->Control.CanLook = false;

	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	if (TrInput & IN_ACTION && pulley->TriggerFlags)
		item->TargetState = LS_PULLEY;
	else
		item->TargetState = LS_IDLE;

	if (item->AnimNumber == LA_PULLEY_PULL &&
		item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 44)
	{
		if (pulley->TriggerFlags)
		{
			if (!pulley->ItemFlags[1])
			{
				pulley->TriggerFlags--;
				if (pulley->TriggerFlags)
				{
					if (pulley->ItemFlags[2])
					{
						pulley->ItemFlags[2] = 0;
						pulley->Status = ITEM_DEACTIVATED;
					}
				}
				else
				{
					pulley->Status = ITEM_DEACTIVATED;
					pulley->ItemFlags[2] = 1;

					if (pulley->ItemFlags[3] >= 0)
						pulley->TriggerFlags = abs(pulley->ItemFlags[3]);
					else
						pulley->ItemFlags[0] = 1;
				}
			}
		}
	}

	if (item->AnimNumber == LA_PULLEY_RELEASE &&
		item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 1)
	{
		info->gunStatus = LG_HANDS_FREE;
	}
}

// --------------
// HORIZONTAL BAR
// --------------

void lara_as_parallelbars(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 128*/
	/*collision: lara_default_col*/
	if (!(TrInput & IN_ACTION) || TrInput & IN_JUMP)
		item->TargetState = LS_BARS_JUMP;
}

void lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 129*/
	/*collision: lara_default_col*/
	ITEM_INFO* barItem = &g_Level.Items[Lara.interactedItem];

	item->Airborne = true;

	if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
	{
		int dist;

		if (item->Position.yRot == barItem->Position.yRot)
			dist = barItem->TriggerFlags / 100 - 2;
		else
			dist = barItem->TriggerFlags % 100 - 2;

		item->VerticalVelocity = -(20 * dist + 64);
		item->Velocity = 20 * dist + 58;
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

void lara_trbalance_mesh(ITEM_INFO* item)
{
	LaraInfo*& info = item->Data;

	item->Position.zRot = info->Control.TightropeControl.Balance / 4;
	info->Control.ExtraTorsoRot.zRot = -info->Control.TightropeControl.Balance;
}

void lara_trbalance_regen(ITEM_INFO* item)
{
	LaraInfo*& info = item->Data;

	if (info->Control.TightropeControl.TimeOnTightrope <= 32)
		info->Control.TightropeControl.TimeOnTightrope = 0;
	else 
		info->Control.TightropeControl.TimeOnTightrope -= 32;

	if (info->Control.TightropeControl.Balance > 0)
	{
		if (info->Control.TightropeControl.Balance <= ANGLE(0.75f))
			info->Control.TightropeControl.Balance = 0;
		else
			info->Control.TightropeControl.Balance -= ANGLE(0.75f);
	}

	if (info->Control.TightropeControl.Balance < 0)
	{
		if (info->Control.TightropeControl.Balance >= -ANGLE(0.75f))
			info->Control.TightropeControl.Balance = 0;
		else
			info->Control.TightropeControl.Balance += ANGLE(0.75f);
	}
}

void lara_trbalance(ITEM_INFO* item)
{
	LaraInfo*& info = item->Data;
	const int factor = ((info->Control.TightropeControl.TimeOnTightrope >> 7) & 0xFF) * 128;

	if (TrInput & IN_LEFT)
		info->Control.TightropeControl.Balance += ANGLE(1.4f);
	if (TrInput & IN_RIGHT)
		info->Control.TightropeControl.Balance -= ANGLE(1.4f);

	if (info->Control.TightropeControl.Balance < 0)
	{
		info->Control.TightropeControl.Balance -= factor;
		if (info->Control.TightropeControl.Balance <= -ANGLE(45.0f))
			info->Control.TightropeControl.Balance = ANGLE(45.0f);

	}
	else if (info->Control.TightropeControl.Balance > 0)
	{
		info->Control.TightropeControl.Balance += factor;
		if (info->Control.TightropeControl.Balance >= ANGLE(45.0f))
			info->Control.TightropeControl.Balance = ANGLE(45.0f);
	}
	else
		info->Control.TightropeControl.Balance = GetRandomControl() & 1 ? -1 : 1;
}

void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LOOK)
		LookUpDown();

	lara_trbalance_regen(item);
	lara_trbalance_mesh(item);

	if (TrInput & IN_FORWARD)
		item->TargetState = LS_TIGHTROPE_FORWARD;
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

void lara_as_trexit(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	lara_trbalance_regen(item);
	lara_trbalance_mesh(item);

	if (item->AnimNumber == LA_TIGHTROPE_END &&
		TestLastFrame(item))
	{
		info->Control.ExtraTorsoRot.zRot = 0;
		item->Position.zRot = 0;
	}
}

void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll) 
{
	LaraInfo*& info = item->Data;

	auto probe = GetCollisionResult(item);
	if (probe.Position.Floor == item->Position.yPos &&
		info->Control.TightropeControl.CanDismount)
	{
		lara_trbalance_regen(item);
		item->TargetState = LS_TIGHTROPE_EXIT;
	}
	
	if (item->TargetState != LS_TIGHTROPE_EXIT &&
	   (TrInput & (IN_BACK | IN_ROLL) || !(TrInput & IN_FORWARD)))
	{
		item->TargetState = LS_TIGHTROPE_IDLE;
	}

	info->Control.TightropeControl.TimeOnTightrope++;
	lara_trbalance(item);
	lara_trbalance_mesh(item);

	if (info->Control.TightropeControl.Balance >= 8000)
		SetAnimation(item, LA_TIGHTROPE_FALL_RIGHT);
	else if (info->Control.TightropeControl.Balance <= -8000)
		SetAnimation(item, LA_TIGHTROPE_FALL_LEFT);
}

void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 122, 123*/
	/*collision: lara_default_col*/
	lara_trbalance_regen(item);
	lara_trbalance_mesh(item);

	if (TestLastFrame(item))
	{
		PHD_VECTOR pos{ 0, 75, 0 };
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
void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 119*/
	/*collision: lara_default_col*/
	if (TrInput & IN_LOOK)
		LookUpDown();

	GetTighRopeFallOff(127);

	if (LaraItem->ActiveState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (Lara.Control.TightropeControl.Fall)
		{
			if (GetRandomControl() & 1)
				item->TargetState = LS_TIGHTROPE_UNBALANCE_RIGHT;
			else
				item->TargetState = LS_TIGHTROPE_UNBALANCE_LEFT;
		}
		else
		{
			if (TrInput & IN_FORWARD)
				item->TargetState = LS_TIGHTROPE_FORWARD;
			else if (TrInput & (IN_ROLL | IN_BACK))
			{
				if (item->AnimNumber == LA_TIGHTROPE_IDLE)
				{
					item->ActiveState = LS_TIGHTROPE_TURN_180;
					item->AnimNumber = LA_TIGHTROPE_TURN_180;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					GetTighRopeFallOff(1);
				}
			}
		}
	}
}

void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 121*/
	/*collision: lara_default_col*/
	if (Lara.Control.TightropeControl.OnCount)
		Lara.Control.TightropeControl.OnCount--;
	else if (Lara.Control.TightropeControl.Off)
	{
		short roomNumber = item->RoomNumber;

		if (GetFloorHeight(GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber),
			item->Position.xPos, item->Position.yPos, item->Position.zPos) == item->Position.yPos)
		{
			Lara.Control.TightropeControl.Off = 0;
			item->TargetState = LS_TIGHTROPE_EXIT;
		}
	}
	else
		GetTighRopeFallOff(127);

	if (item->ActiveState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (TrInput & IN_LOOK)
			LookUpDown();

		if (((TrInput & (IN_BACK | IN_ROLL) || !(TrInput & IN_FORWARD) || Lara.Control.TightropeControl.Fall) &&
			!Lara.Control.TightropeControl.OnCount &&
			!Lara.Control.TightropeControl.Off) &&
			item->TargetState != LS_TIGHTROPE_EXIT)
		{
			item->TargetState = LS_TIGHTROPE_IDLE;
		}
	}
}

void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 122, 123*/
	/*collision: lara_default_col*/
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

		if (Lara.Control.TightropeControl.OnCount > 0)
			Lara.Control.TightropeControl.OnCount--;

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

		if (TrInput & undoInput && Lara.Control.TightropeControl.OnCount == 0)
		{
			item->ActiveState = LS_TIGHTROPE_RECOVER_BALANCE;
			item->TargetState = LS_TIGHTROPE_IDLE;
			item->AnimNumber = undoAnim;
			item->FrameNumber = undoFrame;

			Lara.Control.TightropeControl.Fall--;
		}
		else
		{
			if (TrInput & wrongInput)
			{
				if (Lara.Control.TightropeControl.OnCount < 10)
					Lara.Control.TightropeControl.OnCount += (GetRandomControl() & 3) + 2;
			}
		}
	}
}
#endif

// ----
// ROPE
// ----

void lara_as_ropel(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 90*/
	/*collision: lara_void_func*/
	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
			Lara.Control.RopeControl.Y += ANGLE(1.4f);
		else
			item->TargetState = LS_ROPE_IDLE;
	}
	else
		FallFromRope(item);
}

void lara_as_roper(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_RIGHT)
			Lara.Control.RopeControl.Y -= ANGLE(1.4f);
		else
			item->TargetState = LS_ROPE_IDLE;
	}
	else
		FallFromRope(item);
}

void lara_as_rope(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 111, 114, 115*/
	/*collison: lara_col_rope(111), lara_col_ropefwd(114, 115)*/
	if (!(TrInput & IN_ACTION))
		FallFromRope(item);

	if (TrInput & IN_LOOK)
		LookUpDown();
}

void lara_col_rope(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 111*/
	/*state code: lara_as_rope*/
	if (TrInput & IN_ACTION)
	{
		UpdateRopeSwing(item);

		if (TrInput & IN_SPRINT)
		{
			Lara.Control.RopeControl.DFrame = (g_Level.Anims[LA_ROPE_SWING].frameBase + 32) << 8;
			Lara.Control.RopeControl.Frame = Lara.Control.RopeControl.DFrame;

			item->TargetState = LS_ROPE_SWING;
		}
		else if (TrInput & IN_FORWARD && Lara.Control.RopeControl.Segment > 4)
			item->TargetState = LS_ROPE_UP;
		else if (TrInput & IN_BACK && Lara.Control.RopeControl.Segment < 21)
		{
			item->TargetState = LS_ROPE_DOWN;

			Lara.Control.RopeControl.Flag = 0;
			Lara.Control.RopeControl.Count = 0;
		}
		else if (TrInput & IN_LEFT)
			item->TargetState = LS_ROPE_TURN_CLOCKWISE;
		else if (TrInput & IN_RIGHT)
			item->TargetState = LS_ROPE_TURN_COUNTER_CLOCKWISE;
	}
	else
		FallFromRope(item);
}

void lara_col_ropefwd(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 114, 115*/
	/*state code: lara_as_rope(for both)*/
	Camera.targetDistance = SECTOR(2);

	UpdateRopeSwing(item);

	if (item->AnimNumber == LA_ROPE_SWING)
	{
		if (TrInput & IN_SPRINT)
		{
			int vel;

			if (abs(Lara.Control.RopeControl.LastX) < 9000)
				vel = 192 * (9000 - abs(Lara.Control.RopeControl.LastX)) / 9000;
			else
				vel = 0;

			ApplyVelocityToRope(Lara.Control.RopeControl.Segment - 2,
				item->Position.yRot + (Lara.Control.RopeControl.Direction ? ANGLE(0.0f) : ANGLE(180.0f)),
				vel >> 5);
		}

		if (Lara.Control.RopeControl.Frame > Lara.Control.RopeControl.DFrame)
		{
			Lara.Control.RopeControl.Frame -= (unsigned short)Lara.Control.RopeControl.FrameRate;
			if (Lara.Control.RopeControl.Frame < Lara.Control.RopeControl.DFrame)
				Lara.Control.RopeControl.Frame = Lara.Control.RopeControl.DFrame;
		}
		else if (Lara.Control.RopeControl.Frame < Lara.Control.RopeControl.DFrame)
		{
			Lara.Control.RopeControl.Frame += (unsigned short)Lara.Control.RopeControl.FrameRate;
			if (Lara.Control.RopeControl.Frame > Lara.Control.RopeControl.DFrame)
				Lara.Control.RopeControl.Frame = Lara.Control.RopeControl.DFrame;
		}

		item->FrameNumber = Lara.Control.RopeControl.Frame >> 8;

		if (!(TrInput & IN_SPRINT) &&
			item->FrameNumber == g_Level.Anims[LA_ROPE_SWING].frameBase + 32 &&
			Lara.Control.RopeControl.MaxXBackward < 6750 &&
			Lara.Control.RopeControl.MaxXForward < 6750)
		{
			item->AnimNumber = LA_JUMP_UP_TO_ROPE_END;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

			item->ActiveState = LS_ROPE_IDLE;
			item->TargetState = LS_ROPE_IDLE;
		}

		if (TrInput & IN_JUMP)
			JumpOffRope(item);
	}
	else if (item->FrameNumber == g_Level.Anims[LA_ROPE_IDLE_TO_SWING].frameBase + 15)
		ApplyVelocityToRope(Lara.Control.RopeControl.Segment, item->Position.yRot, 128);
}

void lara_as_climbrope(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 112*/
	/*collision: lara_void_func*/
	if (TrInput & IN_ROLL)
		FallFromRope(item);
	else
	{
		Camera.targetAngle = ANGLE(30.0f);

		if (g_Level.Anims[item->AnimNumber].frameEnd == item->FrameNumber)
		{
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			Lara.Control.RopeControl.Segment -= 2;
		}

		if (!(TrInput & IN_FORWARD) || Lara.Control.RopeControl.Segment <= 4)
			item->TargetState = LS_ROPE_IDLE;
	}
}

void lara_as_climbroped(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 113*/
	/*collision: lara_void_func*/
	LaraClimbRope(item, coll);
}

// -------------
// VERTICAL POLE
// -------------

// State:		LS_POLE_IDLE (99)
// Collision:	lara_col_pole_idle()
void lara_as_pole_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_FREEFALL; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_ACTION)
	{
		if (item->ActiveState == LA_POLE_IDLE) // Hack.
		{
			if (TrInput & IN_LEFT)
			{
				info->Control.TurnRate += LARA_POLE_TURN_RATE;
				if (info->Control.TurnRate > LARA_POLE_TURN_MAX)
					info->Control.TurnRate = LARA_POLE_TURN_MAX;
			}
			else if (TrInput & IN_RIGHT)
			{
				info->Control.TurnRate -= LARA_POLE_TURN_RATE;
				if (info->Control.TurnRate < -LARA_POLE_TURN_MAX)
					info->Control.TurnRate = -LARA_POLE_TURN_MAX;
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
			item->ItemFlags[2] = 0; // Doesn't seem necessary?
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
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
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
	LaraInfo*& info = item->Data;

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
			info->Control.TurnRate += LARA_POLE_TURN_RATE;
			if (info->Control.TurnRate > LARA_POLE_TURN_MAX)
				info->Control.TurnRate = LARA_POLE_TURN_MAX;
		}
		else if (TrInput & IN_RIGHT)
		{
			info->Control.TurnRate -= LARA_POLE_TURN_RATE;
			if (info->Control.TurnRate < -LARA_POLE_TURN_MAX)
				info->Control.TurnRate = -LARA_POLE_TURN_MAX;
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
	LaraInfo*& info = item->Data;

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
			info->Control.TurnRate += LARA_POLE_TURN_RATE;
			if (info->Control.TurnRate > LARA_POLE_TURN_MAX)
				info->Control.TurnRate = LARA_POLE_TURN_MAX;
		}
		else if (TrInput & IN_RIGHT)
		{
			info->Control.TurnRate -= LARA_POLE_TURN_RATE;
			if (info->Control.TurnRate < -LARA_POLE_TURN_MAX)
				info->Control.TurnRate = -LARA_POLE_TURN_MAX;
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

		item->ItemFlags[2] = 0; // Vertical velocity.
		item->TargetState = LS_POLE_IDLE;
		return;
	}

	item->TargetState = LS_POLE_IDLE; // TODO: Dispatch to freefall?
}

// State:		LS_POLE_DOWN (101)
// Control:		lara_as_pole_down()
void lara_col_pole_down(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	GetCollisionInfo(coll, item);

	// Translate Lara down.
	if (item->AnimNumber == LA_POLE_DOWN_END)
		item->ItemFlags[2] -= WALL_SIZE;
	else
		item->ItemFlags[2] += STEP_SIZE;

	// Clamp speed.
	if (item->ItemFlags[2] < 0)
		item->ItemFlags[2] = 0;
	else if (item->ItemFlags[2] > SHRT_MAX / 2)
		item->ItemFlags[2] = SHRT_MAX / 2;

	if (TestLaraPoleCollision(item, coll, false))
		item->Position.yPos += item->ItemFlags[2] >> 8;

	if (coll->Middle.Floor < 0)
		item->Position.yPos += coll->Middle.Floor;
}

// State:		LS_POLE_TURN_CLOCKWISE (102)
// Collision:	lara_col_pole_turn_clockwise()
void lara_as_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

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
			info->Control.TurnRate += LARA_POLE_TURN_RATE;
			if (info->Control.TurnRate > LARA_POLE_TURN_MAX)
				info->Control.TurnRate = LARA_POLE_TURN_MAX;

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
	LaraInfo*& info = item->Data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

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
			info->Control.TurnRate -= LARA_POLE_TURN_RATE;
			if (info->Control.TurnRate < -LARA_POLE_TURN_MAX)
				info->Control.TurnRate = -LARA_POLE_TURN_MAX;

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

void lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	/*state 70*/
	/*collision: lara_void_func*/
	Camera.targetAngle = ANGLE(70.0f);

	if (!(TrInput & IN_ACTION))
	{
		item->TargetState = LS_JUMP_FORWARD;

		AnimateLara(item);

		item->Airborne = true;
		item->Velocity = 100;
		item->VerticalVelocity = 40;
		info->Control.MoveAngle = item->Position.yRot;
	}
}
