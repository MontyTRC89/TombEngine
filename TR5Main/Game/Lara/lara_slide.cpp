#include "framework.h"
#include "lara.h"
#include "lara_collide.h"
#include "input.h"
#include "Sound\sound.h"

short OldAngle = 1;

/*this file has all the related functions to sliding*/

/*tests and others*/
int TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (abs(coll->TiltX) <= 2 && abs(coll->TiltZ) <= 2)
		return 0;

	short angle = ANGLE(0.0f);
	if (coll->TiltX > 2)
		angle = -ANGLE(90.0f);
	else if (coll->TiltX < -2)
		angle = ANGLE(90.0f);

	if (coll->TiltZ > 2 && coll->TiltZ > abs(coll->TiltX))
		angle = ANGLE(180.0f);
	else if (coll->TiltZ < -2 && -coll->TiltZ > abs(coll->TiltX))
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
	}

	Lara.moveAngle = angle;
	OldAngle = angle;

	return 1;
}

void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Settings.BadHeightUp = NO_BAD_POS;
	coll->Settings.BadHeightDown = -512;
	coll->Settings.BadCeilingHeight = 0;

	coll->Settings.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (!LaraHitCeiling(item, coll))
	{
		LaraDeflectEdge(item, coll);

		if (coll->Middle.Floor <= 200)
		{
			TestLaraSlide(item, coll);

			item->pos.yPos += coll->Middle.Floor;

			if (abs(coll->TiltX) <= 2 && abs(coll->TiltZ) <= 2)
			{
				if (TrInput & IN_FORWARD && item->currentAnimState != LS_SLIDE_BACK)
				{
					item->goalAnimState = LS_RUN_FORWARD;
				}
				else
					item->goalAnimState = LS_STOP;
				StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			}
		}
		else
		{
			if (item->currentAnimState == LS_SLIDE_FORWARD)
			{
				item->animNumber = LA_FALL_START;
				item->frameNumber = g_Level.Anims[LA_FALL_START].frameBase;

				item->currentAnimState = LS_JUMP_FORWARD;
				item->goalAnimState = LS_JUMP_FORWARD;
			}
			else
			{
				item->animNumber = LA_FALL_BACK;
				item->frameNumber = g_Level.Anims[LA_FALL_BACK].frameBase;

				item->currentAnimState = LS_FALL_BACK;
				item->goalAnimState = LS_FALL_BACK;
			}

			StopSoundEffect(SFX_TR4_LARA_SLIPPING);

			item->gravityStatus = true;
			item->fallspeed = 0;
		}
	}
}

void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
{
	ShiftItem(item, coll);

	switch (coll->CollisionType)
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
		item->pos.zPos -= 400 * phd_cos(coll->Settings.ForwardAngle);
		item->pos.xPos -= 400 * phd_sin(coll->Settings.ForwardAngle);

		item->speed = 0;

		coll->Middle.Floor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}
/*end tests and others*/
/*-*/
/*Lara state code*/
void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 24*/
	/*collision: lara_col_slide*/
	Camera.targetElevation = -ANGLE(45.0f); // FIXED
	if ((TrInput & IN_JUMP) && !(TrInput & IN_BACK))
		item->goalAnimState = LS_JUMP_FORWARD;
}

void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 24*/
	/*state code: lara_as_slide*/
	Lara.moveAngle = item->pos.yRot;
	lara_slide_slope(item, coll);
}

void lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 32*/
	/*collision: lara_col_slideback*/
	if ((TrInput & IN_JUMP) && !(TrInput & IN_FORWARD))
	{
		item->goalAnimState = LS_JUMP_BACK;
	}
}

void lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 32*/
	/*state code: lara_as_slideback*/
	Lara.moveAngle = item->pos.yRot + ANGLE(180);
	lara_slide_slope(item, coll);
}
/*end Lara state code*/
