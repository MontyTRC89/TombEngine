#include "framework.h"
#include "lara.h"
#include "input.h"
#include "level.h"
#include "Sound/sound.h"
#include "animation.h"
#include "rope.h"
#include "camera.h"
#include "collide.h"
#include "items.h"
#include "control/control.h"
#include "Objects/Generic/Object/rope.h"
#include "Game/Lara/lara_tests.h"

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
	LaraInfo*& info = item->data;

	/*state 39, 98*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = -ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = WALL_SIZE;

	if (TestLastFrame(item))
		item->goalAnimState = GetNextAnimState(item);
}

void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 67*/
	/*collison: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = ANGLE(130.0f);
	Camera.targetElevation = -ANGLE(15.0f);
	Camera.targetDistance = WALL_SIZE;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
		info->gunStatus = LG_NO_ARMS;
}

// ------
// SWITCH
// ------

void lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*states 40, 126*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;
	Camera.speed = 6;
}

void lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 41*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;
	Camera.speed = 6;
}

void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 95*/
	/*state code: lara_as_controlledl*/
	if (coll->Setup.OldPosition.x != item->pos.xPos || coll->Setup.OldPosition.z != item->pos.zPos)
	{
		if (item->animNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_CONTINUE)
		{
			item->pos.yRot -= ANGLE(90.0f);

			item->animNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		if (item->animNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_CONTINUE)
		{
			item->pos.yRot += ANGLE(90.0f);

			item->animNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_END;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
}

// ----------
// RECEPTACLE
// ----------

void lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 42*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;
}

void lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 43*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = -ANGLE(80.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.targetDistance = WALL_SIZE;

	if (TestLastFrame(item))
	{
		if (item->itemFlags[0])
		{
			item->animNumber = item->itemFlags[0];
			item->currentAnimState = LS_MISC_CONTROL;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
}

// --------
// PUSHABLE
// --------

void lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 36*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = -ANGLE(25.0f);
	Camera.laraNode = LM_TORSO;
}

void lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 37*/
	/*collision: lara_default_col*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
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
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = ANGLE(75.0f);

	if (!(TrInput & IN_ACTION))
		item->goalAnimState = LS_STOP;
}

// ------
// PULLEY
// ------

void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 104*/
	/*collision: lara_default_col*/
	LaraInfo*& info = item->data;
	ITEM_INFO* pulley = &g_Level.Items[info->interactedItem];

	info->look = false;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

	if (TrInput & IN_ACTION && pulley->triggerFlags)
		item->goalAnimState = LS_PULLEY;
	else
		item->goalAnimState = LS_STOP;

	if (item->animNumber == LA_PULLEY_PULL &&
		item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 44)
	{
		if (pulley->triggerFlags)
		{
			if (!pulley->itemFlags[1])
			{
				pulley->triggerFlags--;
				if (pulley->triggerFlags)
				{
					if (pulley->itemFlags[2])
					{
						pulley->itemFlags[2] = 0;
						pulley->status = ITEM_DEACTIVATED;
					}
				}
				else
				{
					pulley->status = ITEM_DEACTIVATED;
					pulley->itemFlags[2] = 1;

					if (pulley->itemFlags[3] >= 0)
						pulley->triggerFlags = abs(pulley->itemFlags[3]);
					else
						pulley->itemFlags[0] = 1;
				}
			}
		}
	}

	if (item->animNumber == LA_PULLEY_RELEASE &&
		item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		info->gunStatus = LG_NO_ARMS;
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
		item->goalAnimState = LS_BARS_JUMP;
}

void lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 129*/
	/*collision: lara_default_col*/
	ITEM_INFO* barItem = &g_Level.Items[Lara.interactedItem];

	item->gravityStatus = true;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
	{
		int dist;

		if (item->pos.yRot == barItem->pos.yRot)
			dist = barItem->triggerFlags / 100 - 2;
		else
			dist = barItem->triggerFlags % 100 - 2;

		item->fallspeed = -(20 * dist + 64);
		item->speed = 20 * dist + 58;
	}

	if (TestLastFrame(item))
	{
		SetAnimation(item, LA_REACH);
		item->pos.xPos += 700 * phd_sin(item->pos.yRot);
		item->pos.yPos -= 361;
		item->pos.zPos += 700 * phd_cos(item->pos.yRot);
	}
}

// ---------
// TIGHTROPE
// ---------

#ifdef NEW_TIGHTROPE

void lara_trbalance_mesh(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;

	item->pos.zRot = info->tightrope.balance / 4;
	info->torsoZrot = -info->tightrope.balance;
}

void lara_trbalance_regen(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;

	if (info->tightrope.timeOnTightrope <= 32)
		info->tightrope.timeOnTightrope = 0;
	else 
		info->tightrope.timeOnTightrope -= 32;

	if (info->tightrope.balance > 0)
	{
		if (info->tightrope.balance <= ANGLE(0.75f))
			info->tightrope.balance = 0;
		else
			info->tightrope.balance -= ANGLE(0.75f);
	}

	if (info->tightrope.balance < 0)
	{
		if (info->tightrope.balance >= -ANGLE(0.75f))
			info->tightrope.balance = 0;
		else
			info->tightrope.balance += ANGLE(0.75f);
	}
}

void lara_trbalance(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;
	const int factor = ((info->tightrope.timeOnTightrope >> 7) & 0xFF) * 128;

	if (TrInput & IN_LEFT)
		info->tightrope.balance += ANGLE(1.4f);
	if (TrInput & IN_RIGHT)
		info->tightrope.balance -= ANGLE(1.4f);

	if (info->tightrope.balance < 0)
	{
		info->tightrope.balance -= factor;
		if (info->tightrope.balance <= -ANGLE(45.0f))
			info->tightrope.balance = ANGLE(45.0f);

	}
	else if (info->tightrope.balance > 0)
	{
		info->tightrope.balance += factor;
		if (info->tightrope.balance >= ANGLE(45.0f))
			info->tightrope.balance = ANGLE(45.0f);
	}
	else
		info->tightrope.balance = GetRandomControl() & 1 ? -1 : 1;
}

void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LOOK)
		LookUpDown();

	lara_trbalance_regen(item);
	lara_trbalance_mesh(item);

	if (TrInput & IN_FORWARD)
		item->goalAnimState = LS_TIGHTROPE_FORWARD;
	else if (TrInput & (IN_ROLL | IN_BACK))
	{
		if (item->animNumber == LA_TIGHTROPE_IDLE)
		{
			item->currentAnimState = LS_TIGHTROPE_TURN_180;
			item->animNumber = LA_TIGHTROPE_TURN_180;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
}

void lara_as_trexit(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	lara_trbalance_regen(item);
	lara_trbalance_mesh(item);

	if (item->animNumber == LA_TIGHTROPE_END &&
		TestLastFrame(item))
	{
		info->torsoZrot = 0;
		item->pos.zRot = 0;
	}
}

void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll) 
{
	LaraInfo*& info = item->data;

	auto probe = GetCollisionResult(item);
	if (probe.Position.Floor == item->pos.yPos &&
		info->tightrope.canGoOff)
	{
		lara_trbalance_regen(item);
		item->goalAnimState = LS_TIGHTROPE_EXIT;
	}
	
	if (item->goalAnimState != LS_TIGHTROPE_EXIT &&
	   (TrInput & (IN_BACK | IN_ROLL) || !(TrInput & IN_FORWARD)))
	{
		item->goalAnimState = LS_TIGHTROPE_IDLE;
	}

	info->tightrope.timeOnTightrope++;
	lara_trbalance(item);
	lara_trbalance_mesh(item);

	if (info->tightrope.balance >= 8000)
		SetAnimation(item, LA_TIGHTROPE_FALL_RIGHT);
	else if (info->tightrope.balance <= -8000)
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

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;

		item->goalAnimState = LS_FREEFALL;
		item->currentAnimState = LS_FREEFALL;
		item->animNumber = LA_FREEFALL;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->fallspeed = 81;
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

	if (LaraItem->currentAnimState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (Lara.tightRopeFall)
		{
			if (GetRandomControl() & 1)
				item->goalAnimState = LS_TIGHTROPE_UNBALANCE_RIGHT;
			else
				item->goalAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
		}
		else
		{
			if (TrInput & IN_FORWARD)
				item->goalAnimState = LS_TIGHTROPE_FORWARD;
			else if (TrInput & (IN_ROLL | IN_BACK))
			{
				if (item->animNumber == LA_TIGHTROPE_IDLE)
				{
					item->currentAnimState = LS_TIGHTROPE_TURN_180;
					item->animNumber = LA_TIGHTROPE_TURN_180;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
	if (Lara.tightRopeOnCount)
		Lara.tightRopeOnCount--;
	else if (Lara.tightRopeOff)
	{
		short roomNumber = item->roomNumber;

		if (GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
			item->pos.xPos, item->pos.yPos, item->pos.zPos) == item->pos.yPos)
		{
			Lara.tightRopeOff = 0;
			item->goalAnimState = LS_TIGHTROPE_EXIT;
		}
	}
	else
		GetTighRopeFallOff(127);

	if (item->currentAnimState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (TrInput & IN_LOOK)
			LookUpDown();

		if ((Lara.tightRopeFall || (TrInput & (IN_BACK | IN_ROLL) || !(TrInput & IN_FORWARD)) && !Lara.tightRopeOnCount && !Lara.tightRopeOff) &&
			item->goalAnimState != LS_TIGHTROPE_EXIT)
		{
			item->goalAnimState = LS_TIGHTROPE_IDLE;
		}
	}
}

void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 122, 123*/
	/*collision: lara_default_col*/
	if (item->animNumber == LA_TIGHTROPE_FALL_LEFT || item->animNumber == LA_TIGHTROPE_FALL_RIGHT)
	{
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;

			GetLaraJointPosition(&pos, LM_RFOOT);

			item->pos.xPos = pos.x;
			item->pos.yPos = pos.y + 75;
			item->pos.zPos = pos.z;

			item->goalAnimState = LS_FREEFALL;
			item->currentAnimState = LS_FREEFALL;
			item->animNumber = LA_FREEFALL;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

			item->fallspeed = 81;
			Camera.targetspeed = 16;
		}
	}
	else
	{
		int undoInp, wrongInput;
		int undoAnim, undoFrame;

		if (Lara.tightRopeOnCount > 0)
			Lara.tightRopeOnCount--;

		if (item->animNumber == LA_TIGHTROPE_UNBALANCE_LEFT)
		{
			undoInp = IN_RIGHT;
			wrongInput = IN_LEFT;
			undoAnim = LA_TIGHTROPE_RECOVER_LEFT;
		}
		else if (item->animNumber == LA_TIGHTROPE_UNBALANCE_RIGHT)
		{
			undoInp = IN_LEFT;
			wrongInput = IN_RIGHT;
			undoAnim = LA_TIGHTROPE_RECOVER_RIGHT;
		}
		else
			return;

		undoFrame = g_Level.Anims[item->animNumber].frameEnd + g_Level.Anims[undoAnim].frameBase - item->frameNumber;

		if (TrInput & undoInp && Lara.tightRopeOnCount == 0)
		{
			item->currentAnimState = LS_TIGHTROPE_RECOVER_BALANCE;
			item->goalAnimState = LS_TIGHTROPE_IDLE;
			item->animNumber = undoAnim;
			item->frameNumber = undoFrame;

			Lara.tightRopeFall--;
		}
		else
		{
			if (TrInput & wrongInput)
			{
				if (Lara.tightRopeOnCount < 10)
					Lara.tightRopeOnCount += (GetRandomControl() & 3) + 2;
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
			Lara.ropeY += ANGLE(1.4f);
		else
			item->goalAnimState = LS_ROPE_IDLE;
	}
	else
		FallFromRope(item);
}

void lara_as_roper(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_RIGHT)
			Lara.ropeY -= ANGLE(1.4f);
		else
			item->goalAnimState = LS_ROPE_IDLE;
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
			Lara.ropeDFrame = (g_Level.Anims[LA_ROPE_SWING].frameBase + 32) << 8;
			Lara.ropeFrame = Lara.ropeDFrame;

			item->goalAnimState = LS_ROPE_SWING;
		}
		else if (TrInput & IN_FORWARD && Lara.ropeSegment > 4)
			item->goalAnimState = LS_ROPE_UP;
		else if (TrInput & IN_BACK && Lara.ropeSegment < 21)
		{
			item->goalAnimState = LS_ROPE_DOWN;

			Lara.ropeFlag = 0;
			Lara.ropeCount = 0;
		}
		else if (TrInput & IN_LEFT)
			item->goalAnimState = LS_ROPE_TURN_CLOCKWISE;
		else if (TrInput & IN_RIGHT)
			item->goalAnimState = LS_ROPE_TURN_COUNTER_CLOCKWISE;
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

	if (item->animNumber == LA_ROPE_SWING)
	{
		if (TrInput & IN_SPRINT)
		{
			int vel;

			if (abs(Lara.ropeLastX) < 9000)
				vel = 192 * (9000 - abs(Lara.ropeLastX)) / 9000;
			else
				vel = 0;

			ApplyVelocityToRope(Lara.ropeSegment - 2,
				item->pos.yRot + (Lara.ropeDirection ? ANGLE(0.0f) : ANGLE(180.0f)),
				vel >> 5);
		}

		if (Lara.ropeFrame > Lara.ropeDFrame)
		{
			Lara.ropeFrame -= (unsigned short)Lara.ropeFrameRate;
			if (Lara.ropeFrame < Lara.ropeDFrame)
				Lara.ropeFrame = Lara.ropeDFrame;
		}
		else if (Lara.ropeFrame < Lara.ropeDFrame)
		{
			Lara.ropeFrame += (unsigned short)Lara.ropeFrameRate;
			if (Lara.ropeFrame > Lara.ropeDFrame)
				Lara.ropeFrame = Lara.ropeDFrame;
		}

		item->frameNumber = Lara.ropeFrame >> 8;

		if (!(TrInput & IN_SPRINT) &&
			item->frameNumber == g_Level.Anims[LA_ROPE_SWING].frameBase + 32 &&
			Lara.ropeMaxXBackward < 6750 &&
			Lara.ropeMaxXForward < 6750)
		{
			item->animNumber = LA_JUMP_UP_TO_ROPE_END;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

			item->currentAnimState = LS_ROPE_IDLE;
			item->goalAnimState = LS_ROPE_IDLE;
		}

		if (TrInput & IN_JUMP)
			JumpOffRope(item);
	}
	else if (item->frameNumber == g_Level.Anims[LA_ROPE_IDLE_TO_SWING].frameBase + 15)
		ApplyVelocityToRope(Lara.ropeSegment, item->pos.yRot, 128);
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

		if (g_Level.Anims[item->animNumber].frameEnd == item->frameNumber)
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			Lara.ropeSegment -= 2;
		}

		if (!(TrInput & IN_FORWARD) || Lara.ropeSegment <= 4)
			item->goalAnimState = LS_ROPE_IDLE;
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

// TODO: Move test functions to lara_tests.cpp when lara_state_cleaning_etc branch is merged.
bool TestLaraPoleUp(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestLaraPoleCollision(item, coll, true, STEP_SIZE))
		return false;

	// TODO: Accuracy.
	return (coll->Middle.Ceiling < -STEP_SIZE);
}

bool TestLaraPoleDown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestLaraPoleCollision(item, coll, false))
		return false;

	return (coll->Middle.Floor > 0);
}

// State:		LS_POLE_IDLE (99)
// Collision:	lara_col_pole_idle()
void lara_as_pole_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_FREEFALL; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_ACTION)
	{
		if (item->currentAnimState == LA_POLE_IDLE) // Hack.
		{
			if (TrInput & IN_LEFT)
			{
				info->turnRate += LARA_TURN_RATE;
				if (info->turnRate > LARA_SLOW_TURN)
					info->turnRate = LARA_SLOW_TURN;
			}
			else if (TrInput & IN_RIGHT)
			{
				info->turnRate -= LARA_TURN_RATE;
				if (info->turnRate < -LARA_SLOW_TURN)
					info->turnRate = -LARA_SLOW_TURN;
			}
		}

		// TODO: Add forward jump.
		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_BACK;
			info->gunStatus = LG_NO_ARMS;
			return;
		}

		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->goalAnimState = LS_POLE_UP;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->itemFlags[2] = 0; // Doesn't seem necessary?
			item->goalAnimState = LS_POLE_DOWN;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_POLE_TURN_CLOCKWISE;
			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_POLE_TURN_COUNTER_CLOCKWISE;
			return;
		}

		item->goalAnimState = LS_POLE_IDLE;
		return;
	}

	info->gunStatus = LG_NO_ARMS;

	if (coll->Middle.Floor <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	// TODO: This should not be required. Update anim's root displacement + set position distance.
	//item->pos.xPos -= phd_sin(item->pos.yRot) * 64;
	//item->pos.zPos -= phd_cos(item->pos.yRot) * 64;
	item->goalAnimState = LS_FREEFALL;
}

// State:		LS_POLE_IDLE (99)
// Control:		lara_as_pole_idle()
void lara_col_pole_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.SlopesAreWalls = true;
	GetCollisionInfo(coll, item);

	if (coll->Middle.Floor < 0)
		item->pos.yPos += coll->Middle.Floor;
}

// State:		LS_POLE_UP (100)
// Collision:	lara_col_pole_up()
void lara_as_pole_up(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
		{
			info->turnRate += LARA_TURN_RATE;
			if (info->turnRate > LARA_SLOW_TURN)
				info->turnRate = LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			info->turnRate -= LARA_TURN_RATE;
			if (info->turnRate < -LARA_SLOW_TURN)
				info->turnRate = -LARA_SLOW_TURN;
		}

		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->goalAnimState = LS_POLE_UP;
			return;
		}

		item->goalAnimState = LS_POLE_IDLE;
		return;
	}

	item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to freefall?

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
	LaraInfo*& info = item->data;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	// TODO: In WAD.
	SoundEffect(SFX_TR4_LARA_POLE_LOOP, &item->pos, 0);

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_LEFT)
		{
			info->turnRate += LARA_TURN_RATE;
			if (info->turnRate > LARA_SLOW_TURN)
				info->turnRate = LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			info->turnRate -= LARA_TURN_RATE;
			if (info->turnRate < -LARA_SLOW_TURN)
				info->turnRate = -LARA_SLOW_TURN;
		}

		if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->goalAnimState = LS_POLE_DOWN;
			return;
		}

		item->itemFlags[2] = 0; // ??
		item->goalAnimState = LS_POLE_IDLE;

		return;
	}

	item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to freefall?
}

// State:		LS_POLE_DOWN (101)
// Control:		lara_as_pole_down()
void lara_col_pole_down(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	GetCollisionInfo(coll, item);

	// Translate Lara down.
	if (item->animNumber == LA_POLE_DOWN_END)
		item->itemFlags[2] -= WALL_SIZE;
	else
		item->itemFlags[2] += STEP_SIZE;

	// Clamp speed.
	if (item->itemFlags[2] < 0)
		item->itemFlags[2] = 0;
	else if (item->itemFlags[2] > SHRT_MAX / 2)
		item->itemFlags[2] = SHRT_MAX / 2;

	if (TestLaraPoleCollision(item, coll, false))
		item->pos.yPos += item->itemFlags[2] >> 8;

	if (coll->Middle.Floor < 0)
		item->pos.yPos += coll->Middle.Floor;
}

// State:		LS_POLE_TURN_CLOCKWISE (102)
// Collision:	lara_col_pole_turn_clockwise()
void lara_as_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE; // TODO: Death state dispatch.
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to climp up.
			return;
		}
		else if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to climb down.
			return;
		}

		if (TrInput & IN_LEFT)
		{
			info->turnRate += LARA_TURN_RATE;
			if (info->turnRate > LARA_SLOW_TURN + ANGLE(0.5f))
				info->turnRate = LARA_SLOW_TURN + ANGLE(0.5f);

			item->goalAnimState = LS_POLE_TURN_CLOCKWISE;
			return;
		}

		item->goalAnimState = LS_POLE_IDLE;
		return;
	}

	item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to freefall.
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
	LaraInfo*& info = item->data;

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE; // TODO: Death state dispatch.

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_ACTION)
	{
		if (TrInput & IN_FORWARD && TestLaraPoleUp(item, coll))
		{
			item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to climb up.
			return;
		}
		else if (TrInput & IN_BACK && TestLaraPoleDown(item, coll))
		{
			item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to climb down.
			return;
		}

		if (TrInput & IN_RIGHT)
		{
			info->turnRate -= LARA_TURN_RATE;
			if (info->turnRate < -(LARA_SLOW_TURN + ANGLE(0.5f)))
				info->turnRate = -(LARA_SLOW_TURN + ANGLE(0.5f));

			item->goalAnimState = LS_POLE_TURN_COUNTER_CLOCKWISE;
			return;
		}

		item->goalAnimState = LS_POLE_IDLE;
		return;
	}

	item->goalAnimState = LS_POLE_IDLE; // TODO: Dispatch to freefall.
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
	LaraInfo*& info = item->data;

	/*state 70*/
	/*collision: lara_void_func*/
	Camera.targetAngle = ANGLE(70.0f);

	if (!(TrInput & IN_ACTION))
	{
		item->goalAnimState = LS_JUMP_FORWARD;

		AnimateLara(item);

		item->gravityStatus = true;
		item->speed = 100;
		item->fallspeed = 40;
		info->moveAngle = item->pos.yRot;
	}
}
