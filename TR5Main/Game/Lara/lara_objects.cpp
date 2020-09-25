#include "framework.h"
#include "lara.h"
#include "input.h"
#include "sound.h"
#include "draw.h"
#include "rope.h"
#include "lara_tests.h"

// OBJECT INTERACTION

// ------------------------------
// PICKUP
// Control & Collision Functions
// ------------------------------

// States:		39, 98
// Collision:	lara_default_col()
void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll)//1AB00(<), 1AC34(<) (F)
{
	Camera.targetAngle = ANGLE(-130.0f);
	Camera.targetElevation = ANGLE(-15.0f);
	Camera.targetDistance = SECTOR(1);
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// State:		67
// Collison:	lara_default_col()
void lara_as_pickup_flare(ITEM_INFO* item, COLL_INFO* coll)//1AB5C(<), 1AC90(<) (F)
{
	Camera.targetAngle = ANGLE(130.0f);
	Camera.targetElevation = ANGLE(-15.0f);
	Camera.targetDistance = SECTOR(1);
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
}

// ------------------------------
// SWITCH
// Control & Collision Functions
// ------------------------------

// States:		41, 42, 126
// Collision:	lara_default_col()
void lara_as_switch(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetAngle = ANGLE(80.0f);
	Camera.targetElevation = ANGLE(-25.0f);
	Camera.targetDistance = SECTOR(1);
	Camera.speed = 6;
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// State:		95
// State code:	lara_as_controlledl()
void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll)//1B1B4(<), 1B2E8(<) (F)
{
	if (coll->old.x != item->pos.xPos || coll->old.z != item->pos.zPos)
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

// ------------------------------
// PUZZLES & KEYS
// Control & Collision Functions
// ------------------------------

// State:		42
// Collision:	lara_default_col()
void lara_as_use_key(ITEM_INFO* item, COLL_INFO* coll)//1ACBC(<), 1ADF0(<) (F)
{
	Camera.targetAngle = ANGLE(-80.0f);
	Camera.targetElevation = ANGLE(-25.0f);
	Camera.targetDistance = SECTOR(1);
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// State:		43
// Collision:	lara_default_col()
void lara_as_use_puzzle(ITEM_INFO* item, COLL_INFO* coll)//1AD18(<), 1AE4C(<) (F)
{
	Camera.targetAngle = ANGLE(-80.0f);
	Camera.targetElevation = ANGLE(-25.0f);
	Camera.targetDistance = SECTOR(1);
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
	{
		if (item->itemFlags[0])
		{
			item->animNumber = item->itemFlags[0];
			item->currentAnimState = LS_MISC_CONTROL;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
}

// ------------------------------
// PUSHABLE
// Control & Collision Functions
// ------------------------------

// State:		36
// Collision:	lara_default_col()
void lara_as_pushable_push(ITEM_INFO* item, COLL_INFO* coll)//1AA04(<), 1AB38(<) (F)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(90.0f);
	Camera.targetElevation = ANGLE(-25.0f);
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// State:		37
// Collision:	lara_default_col()
void lara_as_pushable_pull(ITEM_INFO* item, COLL_INFO* coll)//1AA60(<), 1AB94(<) (F)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(35.0f);
	Camera.targetElevation = ANGLE(-25.0f);
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// State:		38
// Collision:	lara_default_col()
void lara_as_pushable_ready(ITEM_INFO* item, COLL_INFO* coll)//1AABC(<), 1ABF0(<) (F)
{
	Camera.targetAngle = ANGLE(75.0f);

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (!(TrInput & IN_ACTION))
	{
		item->goalAnimState = LS_STOP;
	}
}

// ------------------------------
// PULLEY
// Control & Collision Functions
// ------------------------------

// State:		104
// Collision:	lara_default_col()
void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll)//1B288, 1B3BC (F)
{
	ITEM_INFO* p = (ITEM_INFO*)Lara.generalPtr;

	Lara.look = false;

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (TrInput & IN_ACTION && p->triggerFlags)
	{
		item->goalAnimState = LS_PULLEY;
	}
	else
	{
		item->goalAnimState = LS_STOP;
	}

	if (item->animNumber == LA_PULLEY_PULL && item->frameNumber == g_Level.Anims[LA_PULLEY_PULL].frameBase + 44)
	{
		if (p->triggerFlags)
		{
			p->triggerFlags--;

			if (p->triggerFlags)
			{
				if (p->itemFlags[2])
				{
					p->itemFlags[2] = 0;
					p->status = ITEM_DEACTIVATED;
				}
			}
			else
			{
				if (!p->itemFlags[1])
					p->status = ITEM_DEACTIVATED;

				p->itemFlags[2] = 1;

				if (p->itemFlags[3] >= 0)
				{
					p->triggerFlags = abs(p->itemFlags[3]);
				}
				else
				{
					p->itemFlags[0] = 1;
				}
			}
		}
	}

	if (item->animNumber == LA_PULLEY_RELEASE && item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
}

// ------------------------------
// SWING BAR
// Control & Collision Functions
// ------------------------------

// State:		128
// Collision:	lara_default_col()
void lara_as_swing_bar(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_BARS_JUMP;
	}
}

// State:		129
// Collision:	lara_default_col()
void lara_as_swing_bar_leap(ITEM_INFO* item, COLL_INFO* coll)//1D244, 1D3D8 (F)
{
	ITEM_INFO* pitem = (ITEM_INFO*)Lara.generalPtr;

	item->gravityStatus = true;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
	{
		int dist;

		if (item->pos.yRot == item->pos.yRot)
		{
			dist = item->triggerFlags / 100 - 2;
		}
		else
		{
			dist = item->triggerFlags % 100 - 2;
		}

		item->fallspeed = -(20 * dist + 64);
		item->speed = 20 * dist + 58;
	}

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
	{
		item->pos.xPos += 700 * phd_sin(item->pos.yRot) >> W2V_SHIFT;
		item->pos.yPos -= 361;
		item->pos.zPos += 700 * phd_cos(item->pos.yRot) >> W2V_SHIFT;

		item->animNumber = LA_REACH;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_REACH;
		item->currentAnimState = LS_REACH;
	}
}

// ------------------------------
// TIGHTROPE
// Control & Collision Functions
// ------------------------------

// State:		119
// Collision:	lara_default_col()
void lara_as_tightrope_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	GetTighropeFallOff(127);

	if (LaraItem->currentAnimState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (Lara.tightRopeFall)
		{
			if (GetRandomControl() & 1)
			{
				item->goalAnimState = LS_TIGHTROPE_UNBALANCE_RIGHT;
			}
			else
			{
				item->goalAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
			}
		}
		else
		{
			if (TrInput & IN_FORWARD)
			{
				item->goalAnimState = LS_TIGHTROPE_FORWARD;
			}
			else if ((TrInput & IN_ROLL) || (TrInput & IN_BACK))
			{
				if (item->animNumber == LA_TIGHTROPE_IDLE)
				{
					item->currentAnimState = LS_TIGHTROPE_TURN_180;
					item->animNumber = LA_TIGHTROPE_TURN_180;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

					GetTighropeFallOff(1);
				}
			}
		}
	}
}

// State:		121
// Collision:	lara_default_col()
void lara_as_tightrope_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	if (Lara.tightRopeOnCount)
	{
		Lara.tightRopeOnCount--;
	}
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
	{
		GetTighropeFallOff(127);
	}

	if (LaraItem->currentAnimState != LS_TIGHTROPE_UNBALANCE_LEFT)
	{
		if (TrInput & IN_LOOK)
		{
			LookUpDown();
		}

		if (item->goalAnimState != LS_TIGHTROPE_EXIT &&
			(Lara.tightRopeFall || (TrInput & IN_BACK || TrInput & IN_ROLL || !(TrInput & IN_FORWARD)) && !Lara.tightRopeOnCount && !Lara.tightRopeOff))
		{
			item->goalAnimState = LS_TIGHTROPE_IDLE;
		}
	}
}

// States:		122, 123
// Collision:	lara_default_col()
void lara_as_tightrope_fall(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->animNumber == LA_TIGHTROPE_FALL_LEFT || item->animNumber == LA_TIGHTROPE_FALL_RIGHT)
	{
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		{
			item->fallspeed = 81;
			Camera.targetspeed = 16;
		}
	}
	else
	{
		int recoveryInput, wrongInput;
		int undoAnim, undoFrame;

		if (Lara.tightRopeOnCount > 0)
		{
			Lara.tightRopeOnCount--;
		}

		if (item->animNumber == LA_TIGHTROPE_UNBALANCE_LEFT)
		{
			recoveryInput = IN_RIGHT;
			wrongInput = IN_LEFT;
			undoAnim = LA_TIGHTROPE_RECOVER_LEFT;
		}
		else if (item->animNumber == LA_TIGHTROPE_UNBALANCE_RIGHT)
		{
			recoveryInput = IN_LEFT;
			wrongInput = IN_RIGHT;
			undoAnim = LA_TIGHTROPE_RECOVER_RIGHT;
		}
		else
		{
			return;
		}

		undoFrame = g_Level.Anims[item->animNumber].frameEnd + g_Level.Anims[undoAnim].frameBase - item->frameNumber;

		if (TrInput & recoveryInput && Lara.tightRopeOnCount == 0)
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
				{
					Lara.tightRopeOnCount += (GetRandomControl() & 3) + 2;
				}
			}
		}
	}
}

// ------------------------------
// ROPE
// Control & Collision Functions
// ------------------------------

// State:		90
// Collision:	lara_void_func()
void lara_as_rope_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if ((TrInput & IN_ACTION && !EnableActionToggle) ||
		(!(TrInput & IN_ACTION) && EnableActionToggle))
	{
		if (TrInput & IN_LEFT)
		{
			Lara.ropeY += 256;
		}
		else
		{
			item->goalAnimState = LS_ROPE_IDLE;
		}

		return;
	}
	else if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle))			// How can I check for a pull button press (down, let go)?
	{
		FallFromRope(item);
	}
}

// State:		91
// Collision:	lara_void_func()
void lara_as_rope_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if ((TrInput & IN_ACTION && !EnableActionToggle) ||
		(!(TrInput & IN_ACTION) && EnableActionToggle))
	{
		if (TrInput & IN_RIGHT)
		{
			Lara.ropeY -= 256;
		}
		else
		{
			item->goalAnimState = LS_ROPE_IDLE;
		}
	}
	else if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle))
	{
		FallFromRope(item);
	}
}

// States :		111, 114, 115													// What? Why 115??
// Collison:	lara_col_rope() (111), lara_col_RopeSwing() (114, 115)
void lara_as_rope(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle))
	{
		FallFromRope(item);
	}

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}
}

// State:		111
// State code:	lara_as_rope()
void lara_col_rope(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if ((TrInput & IN_ACTION && !EnableActionToggle) ||
		(!(TrInput & IN_ACTION) && EnableActionToggle))
	{
		UpdateRopeSwing(item);

		if (TrInput & IN_SPRINT)
		{
			Lara.ropeDFrame = (g_Level.Anims[LA_ROPE_SWING].frameBase + 32) << 8;
			Lara.ropeFrame = Lara.ropeDFrame;

			item->goalAnimState = LS_ROPE_SWING;
		}
		else if (TrInput & IN_FORWARD && Lara.ropeSegment > 4)
		{
			item->goalAnimState = LS_ROPE_UP;
		}
		else if (TrInput & IN_BACK && Lara.ropeSegment < 21)
		{
			item->goalAnimState = LS_ROPE_DOWN;

			Lara.ropeFlag = 0;
			Lara.ropeCount = 0;
		}
		else if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_ROPE_TURN_CLOCKWISE;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_ROPE_TURN_COUNTER_CLOCKWISE;
		}
	}
	else if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle))
	{
		FallFromRope(item);
	}
}

// States:		114, 115														// Again, why 155?
// State code:	lara_as_rope()
void lara_col_rope_swing(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
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
			{
				Lara.ropeFrame = Lara.ropeDFrame;
			}
		}
		else if (Lara.ropeFrame < Lara.ropeDFrame)
		{
			Lara.ropeFrame += (unsigned short)Lara.ropeFrameRate;
			if (Lara.ropeFrame > Lara.ropeDFrame)
			{
				Lara.ropeFrame = Lara.ropeDFrame;
			}
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
		{
			JumpOffRope(item);
		}
	}
	else if (item->frameNumber == g_Level.Anims[LA_ROPE_IDLE_TO_SWING].frameBase + 15)
	{
		ApplyVelocityToRope(Lara.ropeSegment, item->pos.yRot, 128);
	}
}

// State:		112
// Collision:	lara_void_func()
void lara_as_rope_up(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	//if (TrInput & IN_ROLL)	// Stupid.
	//{
	//	FallFromRope(item);
	//}
	//else
	//{
	Camera.targetAngle = ANGLE(30.0f);

	if (g_Level.Anims[item->animNumber].frameEnd == item->frameNumber)
	{
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		Lara.ropeSegment -= 2;
	}

	if (!(TrInput & IN_FORWARD) || Lara.ropeSegment <= 4)
	{
		item->goalAnimState = LS_ROPE_IDLE;
	}
	//}
}

// State:		113
// Collision:	lara_void_func()
void lara_as_rope_down(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	LaraClimbRope(item, coll);
}

// ------------------------------
// POLE
// Control & Collision Functions
// ------------------------------

void lara_as_pole(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	if ((TrInput & IN_ACTION && !EnableActionToggle) ||
		(!(TrInput & IN_ACTION) && EnableActionToggle))
	{
		item->goalAnimState = LS_POLE_IDLE;

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_POLE_TURN_CLOCKWISE;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_POLE_TURN_COUNTER_CLOCKWISE;
		}

		if (TrInput & IN_LOOK)
		{
			LookUpDown();
		}

		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_BACK;
		}
	}
	else if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle))
	{
		item->pos.xPos -= (phd_sin(item->pos.yRot)) << 6 >> W2V_SHIFT;
		item->pos.zPos -= (phd_cos(item->pos.yRot)) << 6 >> W2V_SHIFT;
		item->goalAnimState = LS_FREEFALL;
	}
}

// TODO: split pole up and pole down into dedicated states.
// State:		99
// State code:	lara_as_null()
void lara_col_pole_stop(ITEM_INFO* item, COLL_INFO* coll)//16DFC, 16F30 (F)
{
	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	/*if (item->animNumber == LA_POLE_IDLE)
	{*/
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	Lara.moveAngle = 0;

	coll->facing = Lara.moveAngle;
	coll->radius = 100;
	coll->slopesAreWalls = true;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if ((TrInput & IN_ACTION && !EnableActionToggle) ||
		(!(TrInput & IN_ACTION) && EnableActionToggle))
	{
		if (TrInput & IN_FORWARD)
		{
			short roomNum = item->roomNumber;

			if (item->pos.yPos - GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
				item->pos.xPos, item->pos.yPos, item->pos.zPos) > SECTOR(1))
			{
				item->goalAnimState = LS_POLE_UP;
			}
		}
		else if (TrInput & IN_BACK && coll->midFloor > 0)
		{
			item->goalAnimState = LS_POLE_DOWN;
			item->itemFlags[2] = 0;
		}
	}
	else if (coll->midFloor <= 0)
	{
		item->goalAnimState = LS_STOP;
	}
	else if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle))
	{
		item->pos.xPos -= (phd_sin(item->pos.yRot)) << 6 >> W2V_SHIFT;
		item->pos.zPos -= (phd_cos(item->pos.yRot)) << 6 >> W2V_SHIFT;
		item->goalAnimState = LS_FREEFALL;
	}
	//}
}

void lara_as_pole_up(ITEM_INFO* item, COLL_INFO* coll)
{

}

void lara_as_pole_down(ITEM_INFO* item, COLL_INFO* coll)
{

}

// State:		100
// State code:	lara_as_null()
void lara_col_pole_up(ITEM_INFO* item, COLL_INFO* coll)//170D8(<), 1720C(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot += 256;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot -= 256;
	}

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_FORWARD) || item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE;
	}

	short roomNumber = item->roomNumber;
	FLOOR_INFO* f = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int c = GetCeiling(f, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (item->pos.yPos - c < SECTOR(1))
	{
		item->goalAnimState = LS_POLE_IDLE;
	}
}

// State:		101
// State code:	lara_as_null()
void lara_col_pole_down(ITEM_INFO* item, COLL_INFO* coll)//171A0, 172D4 (F)
{
	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if ((TrInput & (IN_ACTION | IN_BACK)) != (IN_ACTION | IN_BACK) || item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE;
	}

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;
	coll->radius = 100;
	coll->facing = Lara.moveAngle;

	Lara.moveAngle = 0;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (coll->midFloor < 0)
	{
		short roomNumber = item->roomNumber;
		item->floor = GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
			item->pos.xPos, item->pos.yPos - 762, item->pos.zPos);

		item->goalAnimState = LS_POLE_IDLE;
		item->itemFlags[2] = 0;
	}

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot += 256;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot -= 256;
	}

	if (item->animNumber == LA_POLE_DOWN_END)
	{
		item->itemFlags[2] -= SECTOR(1);
	}
	else
	{
		item->itemFlags[2] += 256;
	}

	// CHECK
	SoundEffect(SFX_LARA_ROPEDOWN_LOOP, &item->pos, 0);

	item->itemFlags[2] = CLAMP(item->itemFlags[2], 0, ANGLE(90.0f));

	item->pos.yPos += item->itemFlags[2] >> 8;
}

// TODO: check for consistency with ropes.
// State:		102
// Collision:	lara_void_func()
void lara_as_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll)//17020(<), 17154(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_LEFT) || (TrInput & (IN_FORWARD | IN_BACK)) || item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE;
	}
	else
	{
		item->pos.yRot += 256;
	}
}

// State:		103
// Collision:	lara_void_func()
void lara_as_pole_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll)//1707C(<), 171B0(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_RIGHT) || (TrInput & (IN_FORWARD | IN_BACK)) || item->hitPoints <= 0)
	{
		item->goalAnimState = LS_POLE_IDLE;
	}
	else
	{
		item->pos.yRot -= 256;
	}
}

// ------------------------------
// ZIPLINE
// Control & Collision Functions
// ------------------------------

// State:	70
// Collision:	lara_void_func()
void lara_as_zipline(ITEM_INFO* item, COLL_INFO* coll)//1B038, 1B16C (F)
{
	Camera.targetAngle = ANGLE(70.0f);

	coll->trigger = TriggerIndex;

	short roomNum = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if ((!(TrInput & IN_ACTION) && !EnableActionToggle) ||
		(TrInput & IN_ACTION && EnableActionToggle) ||
		TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_FORWARD;

		AnimateLara(item);
		
		Lara.moveAngle = 0;

		item->gravityStatus = true;
		item->speed = 100;
		item->fallspeed = 40;
	}
}
