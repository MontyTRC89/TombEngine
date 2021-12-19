#include "framework.h"
#include "lara_surface.h"
#include "lara_tests.h"
#include "control/control.h"
#include "camera.h"
#include "collision/collide_room.h"
#include "items.h"
#include "Lara.h"
#include "level.h"
#include "input.h"
#include "lara_collide.h"
#include "lara_swim.h"

void lara_col_surftread(ITEM_INFO* item, COLL_INFO* coll) 
{
	Lara.moveAngle = item->pos.yRot;
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfback(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(180);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfswim(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	Lara.moveAngle = item->pos.yRot;
	LaraSurfaceCollision(item, coll);
	TestLaraWaterClimbOut(item, coll);
	TestLaraLadderClimbOut(item, coll);
}

void lara_as_surftread(ITEM_INFO* item, COLL_INFO* coll)
{
	item->fallspeed -= 4;
	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot -= ANGLE(4);
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot += ANGLE(4);
	}

	if (TrInput & IN_FORWARD)
	{
		item->goalAnimState = LS_ONWATER_FORWARD;
	}
	else if (TrInput & IN_BACK)
	{
		item->goalAnimState = LS_ONWATER_BACK;
	}

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_ONWATER_LEFT;
	}
	else if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_ONWATER_RIGHT;
	}

	if (TrInput & IN_JUMP)
	{
		Lara.diveCount++;
		if (Lara.diveCount == 10)
			SwimDive(item);
	}
	else
		Lara.diveCount = 0;
}

void lara_as_surfright(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	Lara.diveCount = 0;

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot -= ANGLE(2);
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot += ANGLE(2);
	}

	if (!(TrInput & IN_RSTEP))
	{
		item->goalAnimState = LS_ONWATER_STOP;
	}

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_surfleft(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	Lara.diveCount = 0;

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot -= ANGLE(2);
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot += ANGLE(2);
	}

	if (!(TrInput & IN_LSTEP))
	{
		item->goalAnimState = LS_ONWATER_STOP;
	}

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_surfback(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	Lara.diveCount = 0;

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot -= ANGLE(2);
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot += ANGLE(2);
	}

	if (!(TrInput & IN_BACK))
	{
		item->goalAnimState = LS_ONWATER_STOP;
	}

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_surfswim(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	Lara.diveCount = 0;

	if (TrInput & IN_LEFT)
	{
		item->pos.yRot -= ANGLE(4);
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.yRot += ANGLE(4);
	}

	if (!(TrInput & IN_FORWARD))
		item->goalAnimState = LS_ONWATER_STOP;
	if (TrInput & IN_JUMP)
		item->goalAnimState = LS_ONWATER_STOP;

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_HIPS;	//forces the camera to follow Lara instead of snapping
}

