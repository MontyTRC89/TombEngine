#include "framework.h"
#include "lara.h"
#include "lara_collide.h"
#include "lara_tests.h"
#include "input.h"
#include "Sound\sound.h"
#include "collide.h"
#include "camera.h"
#include "level.h"
#include "items.h"

/*this file has all the related functions to sliding*/

void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -512;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

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
				SetAnimation(item, LA_FALL_START);
			else
				SetAnimation(item, LA_FALL_BACK);

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
		item->pos.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);
		item->pos.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);

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
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	}
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
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
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
