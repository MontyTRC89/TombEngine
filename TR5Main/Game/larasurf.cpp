#include "larasurf.h"

#include "..\Global\global.h"
#include "control.h"
#include "camera.h"
#include "collide.h"
#include "items.h"
#include "box.h"
#include "Lara.h"
#include "laraswim.h"
#include "larafire.h"
#include "laramisc.h"

extern void(*lara_control_routines[NUM_LARA_STATES + 1])(ITEM_INFO* item, COLL_INFO* coll);
extern void(*lara_collision_routines[NUM_LARA_STATES + 1])(ITEM_INFO* item, COLL_INFO* coll);

void lara_col_surftread(ITEM_INFO* item, COLL_INFO* coll) 
{
	if (item->goalAnimState == STATE_LARA_UNDERWATER_FORWARD)
	{
		item->currentAnimState = STATE_LARA_UNDERWATER_DIVING;
		item->animNumber = ANIMATION_LARA_FREE_FALL_TO_UNDERWATER_ALTERNATE;
		item->pos.xRot = -8190;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->fallspeed = 80;
		Lara.waterStatus = LW_UNDERWATER;
	}
	Lara.moveAngle = item->pos.yRot;
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO* item, COLL_INFO* coll)//4DD90(<), 4E1F4(<) (F)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO* item, COLL_INFO* coll)//4DD64(<), 4E1C8(<) (F)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfback(ITEM_INFO* item, COLL_INFO* coll)//4DD38(<), 4E19C(<) (F)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(180);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfswim(ITEM_INFO* item, COLL_INFO* coll)//4DCE8(<), 4E14C(<) (F)
{
	coll->badNeg = -384;
	Lara.moveAngle = item->pos.yRot;
	LaraSurfaceCollision(item, coll);
	LaraTestWaterClimbOut(item, coll);
}

void lara_as_surftread(ITEM_INFO* item, COLL_INFO* coll)//4DBA0, 4E004 (F)
{
	item->fallspeed -= 4;
	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;
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
		item->goalAnimState = STATE_LARA_ONWATER_FORWARD;
	}
	else if (TrInput & IN_BACK)
	{
		item->goalAnimState = STATE_LARA_ONWATER_BACK;
	}

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = STATE_LARA_ONWATER_LEFT;
	}
	else if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = STATE_LARA_ONWATER_RIGHT;
	}

	if (TrInput & IN_JUMP)
	{
		Lara.diveCount++;
		if (Lara.diveCount == 10)
			item->goalAnimState = STATE_LARA_UNDERWATER_FORWARD;
	}
	else
	{
		Lara.diveCount = 0;
	}
}

void lara_as_surfright(ITEM_INFO* item, COLL_INFO* coll)//4DAF8, 4DF5C (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;
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
		item->goalAnimState = STATE_LARA_ONWATER_STOP;
	}

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_surfleft(ITEM_INFO* item, COLL_INFO* coll)//4DA50(<), 4DEB4(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;
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
		item->goalAnimState = STATE_LARA_ONWATER_STOP;
	}

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_surfback(ITEM_INFO* item, COLL_INFO* coll)//4D9A8(<), 4DE0C(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;
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
		item->goalAnimState = STATE_LARA_ONWATER_STOP;
	}

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void lara_as_surfswim(ITEM_INFO* item, COLL_INFO* coll)//4D8E4(<), 4DD48(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;
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
		item->goalAnimState = STATE_LARA_ONWATER_STOP;
	if (TrInput & IN_JUMP)
		item->goalAnimState = STATE_LARA_ONWATER_STOP;

	item->fallspeed += 8;
	if (item->fallspeed > 60)
		item->fallspeed = 60;
}

void LaraSurface(ITEM_INFO* item, COLL_INFO* coll)//4D684, 4DAE8 (F)
{
	Camera.targetElevation = -ANGLE(22);

	coll->badPos = 32512;
	coll->badNeg = -128;
	coll->badCeiling = 100;

	coll->old.x = item->pos.xPos;
	coll->old.y = item->pos.yPos;
	coll->old.z = item->pos.zPos;

	coll->slopesAreWalls = 0;
	coll->slopesArePits = 0;
	coll->lavaIsPit = 0;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	coll->radius = 100;
	coll->trigger = NULL;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	lara_control_routines[item->currentAnimState](item, coll);

	if (item->pos.zRot >= -ANGLE(2) && item->pos.zRot <= ANGLE(2))
		item->pos.zRot = 0;
	else if (item->pos.zRot < 0)
		item->pos.zRot += ANGLE(2);
	else
		item->pos.zRot -= ANGLE(2);

	if (Lara.currentActive && Lara.waterStatus != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.xPos += item->fallspeed * SIN(Lara.moveAngle) >> (W2V_SHIFT + 2);
	item->pos.zPos += item->fallspeed * COS(Lara.moveAngle) >> (W2V_SHIFT + 2);

	LaraBaddieCollision(item, coll);

	lara_collision_routines[item->currentAnimState](item, coll);

	UpdateLaraRoom(item, 100);

	LaraGun();

	TestTriggers(coll->trigger, 0, 0);
}

void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll)//4D4F0(<), 4D954(<) (F)
{
	coll->facing = Lara.moveAngle;
	
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos + 700, item->pos.zPos, item->roomNumber, 800);
	ShiftItem(item, coll);
	
	if (coll->collType & (CT_FRONT | CT_TOP | CT_TOP_FRONT | CT_CLAMP) ||
		coll->midFloor < 0 && (coll->midType == BIG_SLOPE || coll->midType == DIAGONAL))
	{
		item->fallspeed = 0;
		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->pos.zPos = coll->old.z;
	}
	else if (coll->collType == CT_LEFT)
	{
		item->pos.yRot += ANGLE(5);
	}
	else if (coll->collType == CT_RIGHT)
	{
		item->pos.yRot -= ANGLE(5);
	}

	if (GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber) - item->pos.yPos > -100)
	{
		LaraTestWaterStepOut(item, coll);
	}
	else
	{
		item->goalAnimState = STATE_LARA_UNDERWATER_FORWARD;
		item->currentAnimState = STATE_LARA_UNDERWATER_DIVING;
		item->animNumber = ANIMATION_LARA_FREE_FALL_TO_UNDERWATER_ALTERNATE;
		item->pos.xRot = -8190;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->fallspeed = 80;
		Lara.waterStatus = LW_UNDERWATER;
	}
}

int LaraTestWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll)//4D22C, 4D690
{
	if (coll->collType != CT_FRONT || !(TrInput & IN_ACTION))
		return 0;

	if (abs(coll->rightFloor2 - coll->leftFloor2) >= 60 
		|| Lara.gunStatus 
		&& (Lara.gunStatus != LG_READY 
			|| Lara.gunType != WEAPON_FLARE))
		return 0;

	if (coll->frontCeiling > 0)
		return 0;

	if (coll->midCeiling > -384)
		return 0;

	int frontFloor = coll->frontFloor + 700;
	if (frontFloor <= -512 || frontFloor > 316)
		return 0;

	short rot = item->pos.yRot;
	if (item->pos.yRot < -ANGLE(35) || item->pos.yRot > ANGLE(35))
	{
		if (item->pos.yRot < 10014 || item->pos.yRot > 22754)
		{
			if (item->pos.yRot >= 26397 || item->pos.yRot <= -26397)
			{
				rot = ANGLE(180);
			}
			else if (item->pos.yRot >= -22754 && item->pos.yRot <= -10014)
			{
				rot = -ANGLE(90);
			}
		}
		else
		{
			rot = ANGLE(90);
		}
	}
	else
	{
		rot = 0;
	}
	
	if (rot & 0x3FFF)
		return 0;

	item->pos.yPos += coll->frontFloor + 695;
	
	UpdateLaraRoom(item, -381);

	if (rot > 0)
	{
		if (rot == ANGLE(90))
		{
			item->pos.xPos = (item->pos.xPos & 0xFFFFFC00) + 1124;
		}
	}
	else if (rot)
	{
		if (rot == ANGLE(180))
		{
			item->pos.zPos = (item->pos.zPos & 0xFFFFFC00) - 100;
		}
		else if (rot == -ANGLE(90))
		{
			item->pos.xPos = (item->pos.xPos & 0xFFFFFC00) - 100;
		}
	}
	else
	{
		item->pos.zPos = (item->pos.zPos & 0xFFFFFC00) + 1124;
	}

	if (frontFloor >= -128)
	{
		if (frontFloor >= 128)
		{
			item->animNumber = ANIMATION_LARA_ONWATER_TO_WADE;
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
		else
		{
			item->animNumber = ANIMATION_LARA_ONWATER_TO_LAND_LOW;
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		item->animNumber = ANIMATION_LARA_CLIMB_OUT_OF_WATER;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
	
	item->currentAnimState = STATE_LARA_ONWATER_EXIT;
	item->goalAnimState = STATE_LARA_STOP;
	item->pos.yRot = rot;
	Lara.gunStatus = LG_HANDS_BUSY;
	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;
	Lara.waterStatus = LW_ABOVE_WATER;

	return 1;
}

int LaraTestWaterStepOut(ITEM_INFO* item, COLL_INFO* coll)//4D100, 4D564 (F)
{
	if (coll->collType == CT_FRONT 
		|| coll->midType == BIG_SLOPE 
		|| coll->midType == DIAGONAL 
		|| coll->midFloor >= 0)
	{
		return 0;
	}

	if (coll->midFloor >= -128)
	{
		if (item->goalAnimState == STATE_LARA_ONWATER_LEFT)
		{
			item->goalAnimState = STATE_LARA_WALK_LEFT;
		}
		else if (item->goalAnimState == STATE_LARA_ONWATER_RIGHT)
		{
			item->goalAnimState = STATE_LARA_WALK_RIGHT;
		}
		else
		{
			item->animNumber = ANIMATION_LARA_WADE;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->goalAnimState = STATE_LARA_WADE_FORWARD;
			item->currentAnimState = STATE_LARA_WADE_FORWARD;
		}
	}
	else
	{
		item->animNumber = ANIMATION_LARA_ONWATER_TO_WADE_DEEP;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->currentAnimState = STATE_LARA_ONWATER_EXIT;
		item->goalAnimState = STATE_LARA_STOP;
	}

	item->pos.yPos += coll->frontFloor + 695;

	UpdateLaraRoom(item, -381);

	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;

	Lara.waterStatus = LW_WADE;

	return true;
}
