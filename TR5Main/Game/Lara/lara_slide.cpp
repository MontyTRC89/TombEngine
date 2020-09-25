#include "framework.h"
#include "lara.h"
#include "lara_collide.h"
#include "lara_tests.h"
#include "input.h"
#include "sound.h"
//#include "lara_slide.h"

int OldAngle = 1;	// Don't care. To be deleted.

// For later.
bool EnableAugmentedSlide;			// Adds steep sliding, slope hugging, 180 turn.
bool EnableNonCardinalSlide;		// Slides Lara in true slope direction.
bool EnableSlideInterpolation;		// Adapts Lara angles to slope.
bool EnableSlideSteering;			// Allows player influence on slide trajectory.

// SLOPE SLIDING

// ------------------------------
// Auxiliary functions
// ------------------------------

bool TestLaraSlide(COLL_INFO* coll)
{
	if (abs(coll->tiltX) <= 2 && abs(coll->tiltZ) <= 2)
	{
		return false;
	}

	return true;
}

short GetLaraSlideDirection(COLL_INFO* coll)
{
	short laraAngle = ANGLE(0.0f);

	//if (EnableNonCardinalSlide)
	//{
	//	// TODO: Get true slope direction.
	//}
	//else
	//{
		if (coll->tiltX > 2)
		{
			laraAngle = -ANGLE(90.0f);
		}
		else if (coll->tiltX < -2)
		{
			laraAngle = ANGLE(90.0f);
		}

		if (coll->tiltZ > 2 && coll->tiltZ > abs(coll->tiltX))
		{
			laraAngle = ANGLE(180.0f);
		}
		else if (coll->tiltZ < -2 && -coll->tiltZ > abs(coll->tiltX))
		{
			laraAngle = ANGLE(0.0f);
		}
	//}

	return laraAngle;
}

int GetSlopeAngle(COLL_INFO* coll)
{
	return 1;
}

void SetLaraSlide(ITEM_INFO* item, COLL_INFO* coll)
{
	short dir = GetLaraSlideDirection(coll);
	short polarity = dir - item->pos.yRot;

	ShiftItem(item, coll);

	// Slide back.
	if (polarity < ANGLE(-90.0f) || polarity > ANGLE(90.0f))
	{
		if (GetSlopeAngle(coll) < ANGLE(60.0f))//SLOPE_ANGLE_STEEP)
		{
			Lara.moveAngle = ANGLE(180);
			item->goalAnimState = LS_SLIDE_BACK;
			item->pos.yRot = dir + ANGLE(180.0f);
		}
		else
		{
			item->goalAnimState = LS_SLIDE_STEEP_BACK;
			item->pos.yRot = dir + ANGLE(180.0f);
		}
	}
	// Slide forward.
	else
	{
		if (GetSlopeAngle(coll) < ANGLE(60.0f))//SLOPE_ANGLE_STEEP)
		{
			Lara.moveAngle = 0;
			item->goalAnimState = LS_SLIDE_FORWARD;
			item->pos.yRot = dir;
		}
		else
		{
			item->goalAnimState = LS_SLIDE_STEEP_FORWARD;
			item->pos.yRot = dir;
		}
	}
}

// LEGACY. Staying until everything is done.
int Old_TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (abs(coll->tiltX) <= 2 && abs(coll->tiltZ) <= 2)
		return 0;

	short angle = ANGLE(0.0f);
	if (coll->tiltX > 2)
		angle = -ANGLE(90.0f);
	else if (coll->tiltX < -2)
		angle = ANGLE(90.0f);

	if (coll->tiltZ > 2 && coll->tiltZ > abs(coll->tiltX))
		angle = ANGLE(180.0f);
	else if (coll->tiltZ < -2 && -coll->tiltZ > abs(coll->tiltX))
		angle = ANGLE(0.0f);

	short delta = angle - item->pos.yRot;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->currentAnimState == LS_SLIDE_BACK && OldAngle == angle)
			return 1;

		item->animNumber = LA_SLIDE_BACK_START;
		item->goalAnimState = LS_SLIDE_BACK;
		item->currentAnimState = LS_SLIDE_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->pos.yRot = angle + ANGLE(180.0f);
		Lara.moveAngle = ANGLE(180);
	}
	else
	{
		if (item->currentAnimState == LS_SLIDE_FORWARD && OldAngle == angle)
			return 1;

		item->animNumber = LA_SLIDE_FORWARD;
		item->goalAnimState = LS_SLIDE_FORWARD;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_SLIDE_FORWARD;
		item->pos.yRot = angle;
		Lara.moveAngle = 0;
	}

	OldAngle = angle;

	return 1;
}

void PerformLaraSlide(ITEM_INFO* item, COLL_INFO* coll)//127BC, 1286C (F)
{

}

void PerformLaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll)//12B18, 12BC8 (F)
{
	ShiftItem(item, coll);

	switch (coll->collType)
	{
	case CT_LEFT:
		item->pos.yRot += ANGLE(5.0f);
		break;

	case CT_RIGHT:
		item->pos.yRot -= ANGLE(5.0f);
		break;

	case CT_TOP:
	case CT_TOP_FRONT:
		if (item->fallspeed <= 0)
			item->fallspeed = 1;
		break;

	case CT_CLAMP:
		item->pos.zPos -= (400 * phd_cos(coll->facing)) >> W2V_SHIFT;
		item->pos.xPos -= (400 * phd_sin(coll->facing)) >> W2V_SHIFT;

		item->speed = 0;

		coll->midFloor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}

// ------------------------------
// SLOPE SLIDING
// Control & Collision Functions
// ------------------------------

// State:		24
// Collision:	lara_col_Slide()
void lara_as_slide(ITEM_INFO* player, COLL_INFO* coll)//1A824(<), 1A958(<) (F)
{
	Camera.targetElevation = player->pos.xRot - ANGLE(45.0f);

	/*if (TrInput & IN_LEFT)
	{
		player->pos.yRot -= ANGLE(2.0f);

		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
		{
			Lara.turnRate = -LARA_FAST_TURN;
		}

		if (TestLaraLean(coll) & EnableSlideSteering)
		{
			player->pos.zRot -= LARA_LEAN_RATE;
			if (player->pos.zRot < -LARA_LEAN_MAX)
			{
				player->pos.zRot = -LARA_LEAN_MAX;
			}
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		player->pos.yRot += ANGLE(2.0f);

		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
		{
			Lara.turnRate = LARA_FAST_TURN;
		}

		if (TestLaraLean(coll) && EnableSlideSteering)
		{
			player->pos.zRot += LARA_LEAN_RATE;
			if (player->pos.zRot > LARA_LEAN_MAX)
			{
				player->pos.zRot = LARA_LEAN_MAX;
			}
		}
	}*/

	if ((TrInput & IN_BACK) || (TrInput & IN_DUCK))
	{
		player->goalAnimState = LS_SLIDE_STEEP_FORWARD;
	}

	if (TrInput & IN_JUMP)
	{
		player->goalAnimState = LS_JUMP_FORWARD;
	}

	if (TrInput & IN_ROLL)
	{
		player->goalAnimState = LS_SLIDE_TURN_180;
		//SetLaraSlide(item, coll);	// Temp: Lara slides up the slope otherwise.
	}
}

void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll)//1C108(<), 1C23C(<) (F)
{
	Lara.moveAngle = ANGLE(0.0f);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -512;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	LaraDeflectEdge(item, coll);

	if (coll->midFloor <= 200)
	{
		item->pos.yPos += coll->midFloor;

		if (!TestLaraSlide(coll))
		{
			item->goalAnimState = (TrInput & IN_FORWARD) ? LS_RUN_FORWARD : LS_STOP;

			StopSoundEffect(SFX_LARA_SLIPPING);
		}
	}
	else
	{
		if (TrInput & IN_ACTION && coll->midFloor >= 1024 && EnableSafetyDrop)
		{
			item->goalAnimState = LS_SAFE_DROP;
		}
		else
		{
			item->goalAnimState = LS_FALL;
		}

		item->gravityStatus = true;
		item->fallspeed = 0;

		StopSoundEffect(SFX_LARA_SLIPPING);
	}
}

// State:		32
// Collision:	lara_col_SlideBack()
void lara_as_slide_back(ITEM_INFO* item, COLL_INFO* coll)//1A9E0(<), 1AB14(<) (F)
{
	Camera.targetElevation = item->pos.xRot;

	//if (TrInput & IN_FORWARD)
	//	item->goalAnimState = LS_SLIDE_STEEP_BACK;

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_BACK;
	}

	/*if (TrInput & IN_ROLL)
	{
		item->pos.yRot += ANGLE(180.0f);
		item->goalAnimState = LS_SLIDE_FORWARD;
		SetLaraSlide(item, coll);
	}*/
}

void lara_col_slide_back(ITEM_INFO* item, COLL_INFO* coll)//1C284(<), 1C3B8(<) (F)
{
	Lara.moveAngle = ANGLE(180.0f);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -512;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	LaraDeflectEdge(item, coll);

	if (coll->midFloor <= 200)
	{
		item->pos.yPos += coll->midFloor;

		if (!TestLaraSlide(coll))
		{
			item->goalAnimState = LS_STOP;
			StopSoundEffect(SFX_LARA_SLIPPING);
		}
	}
	else
	{
		if (TrInput & IN_ACTION && coll->midFloor >= 1024 && EnableSafetyDrop)
		{
			item->goalAnimState = LS_SAFE_DROP;
		}
		else
		{
			item->goalAnimState = LS_FALL_BACK;
		}

		item->gravityStatus = true;
		item->fallspeed = 0;

		StopSoundEffect(SFX_LARA_SLIPPING);
	}
}

// TODO: Unique handling for steep slopes. Anims on the way.
void lara_as_steep_slide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->pos.xRot >= ANGLE(60))
	{
		Camera.targetElevation = ANGLE(-60.0f);
	}
	else
	{
		Camera.targetElevation = ANGLE(-45.0f);
	}

	if (TrInput & IN_FORWARD)
	{
		item->goalAnimState = LS_SLIDE_FORWARD;
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
	}
}

void lara_col_steep_slide(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = ANGLE(0.0f);
	PerformLaraSlide(item, coll);
}

void lara_as_steep_slide_back(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetElevation = ANGLE(-60.0f);

	if (TrInput & IN_BACK)
	{
		item->goalAnimState = LS_SLIDE_BACK;
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_BACK;
	}
}

void lara_col_steep_slide_back(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = ANGLE(180.0f);
	PerformLaraSlide(item, coll);
}

void lara_as_slide_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = ANGLE(0.0f);
}

// TODO: Straddling illegal slopes.
// TODO: Grabbing edge of plateau leading to steep slope.
