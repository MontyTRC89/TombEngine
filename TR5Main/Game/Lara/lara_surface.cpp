#include "framework.h"
#include "Game/Lara/lara_surface.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/level.h"
#include "Specific/input.h"

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
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	Lara.moveAngle = item->pos.yRot;
	LaraSurfaceCollision(item, coll);
	TestLaraWaterClimbOut(item, coll);
	TestLaraLadderClimbOut(item, coll);
}

void lara_as_surftread(ITEM_INFO* item, COLL_INFO* coll)
{
	item->VerticalVelocity -= 4;
	if (item->VerticalVelocity < 0)
		item->VerticalVelocity = 0;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_WATER_DEATH;
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
		item->targetState = LS_ONWATER_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK)
	{
		item->targetState = LS_ONWATER_BACK;
		return;
	}
	else if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}
	else if (TrInput & IN_LSTEP)
	{
		item->targetState = LS_ONWATER_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP)
	{
		item->targetState = LS_ONWATER_RIGHT;
		return;
	}
	else if (TrInput & IN_JUMP)
	{
		Lara.diveCount++;
		if (Lara.diveCount == 10)
			SwimDive(item);
		return;
	}

	Lara.diveCount = 0;
	item->targetState = LS_ONWATER_STOP;
}

void lara_as_surfright(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->targetState = LS_WATER_DEATH;
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
		item->targetState = LS_ONWATER_STOP;
	}

	item->VerticalVelocity += 8;
	if (item->VerticalVelocity > 60)
		item->VerticalVelocity = 60;
}

void lara_as_surfleft(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->targetState = LS_WATER_DEATH;
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
		item->targetState = LS_ONWATER_STOP;
	}

	item->VerticalVelocity += 8;
	if (item->VerticalVelocity > 60)
		item->VerticalVelocity = 60;
}

void lara_as_surfback(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->targetState = LS_WATER_DEATH;
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
		item->targetState = LS_ONWATER_STOP;
	}

	item->VerticalVelocity += 8;
	if (item->VerticalVelocity > 60)
		item->VerticalVelocity = 60;
}

void lara_as_surfswim(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->targetState = LS_WATER_DEATH;
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
		item->targetState = LS_ONWATER_STOP;
	if (TrInput & IN_JUMP)
		item->targetState = LS_ONWATER_STOP;

	item->VerticalVelocity += 8;
	if (item->VerticalVelocity > 60)
		item->VerticalVelocity = 60;
}

void lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_HIPS;	//forces the camera to follow Lara instead of snapping
}

