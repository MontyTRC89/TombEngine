#include "framework.h"
#include "lara_surface.h"
#include "lara_tests.h"
#include "control/control.h"
#include "camera.h"
#include "collide.h"
#include "items.h"
#include "control/box.h"
#include "Lara.h"
#include "lara_swim.h"
#include "lara_fire.h"
#include "level.h"
#include "input.h"

bool EnableCrawlFlexWaterPullUp, EnableCrawlFlexSubmerged;

void lara_col_surftread(ITEM_INFO* item, COLL_INFO* coll) 
{
	if (item->goalAnimState == LS_UNDERWATER_FORWARD)
	{
		item->currentAnimState = LS_DIVE;
		item->animNumber = LA_ONWATER_DIVE;
		item->pos.xRot = -8190;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->fallspeed = 80;
		Lara.waterStatus = LW_UNDERWATER;
	}
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
	LaraTestWaterClimbOut(item, coll);
	LaraTestLadderClimbOut(item, coll);
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
			item->goalAnimState = LS_UNDERWATER_FORWARD;
	}
	else
	{
		Lara.diveCount = 0;
	}
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

void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle = Lara.moveAngle;
	
	GetCollisionInfo(coll, item, PHD_VECTOR(0, 700, 0));
	ShiftItem(item, coll);
	
	if (coll->CollisionType & (CT_FRONT | CT_TOP | CT_TOP_FRONT | CT_CLAMP) ||
		coll->Middle.Floor < 0 && coll->Middle.Slope)
	{
		item->fallspeed = 0;
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}
	else if (coll->CollisionType == CT_LEFT)
	{
		item->pos.yRot += ANGLE(5);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		item->pos.yRot -= ANGLE(5);
	}

	if (GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber) - item->pos.yPos > -100)
	{
		LaraTestWaterStepOut(item, coll);
	}
	else
	{
		item->goalAnimState = LS_UNDERWATER_FORWARD;
		item->currentAnimState = LS_DIVE;
		item->animNumber = LA_ONWATER_DIVE;
		item->pos.xRot = -8190;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->fallspeed = 80;
		Lara.waterStatus = LW_UNDERWATER;
	}
}

int LaraTestWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType != CT_FRONT || !(TrInput & IN_ACTION))
		return 0;

	// FOR DEBUG PURPOSES UNTIL SCRIPTING IS READY-
	EnableCrawlFlexWaterPullUp = false;
	EnableCrawlFlexSubmerged = false;


	if (Lara.gunStatus && (Lara.gunStatus != LG_READY || Lara.gunType != WEAPON_FLARE))
		return 0;

	if (coll->Front.Ceiling > 0)
		return 0;

	if (coll->Middle.Ceiling > -384)
		return 0;

	if (coll->ObjectHeadroom < LARA_HEIGHT)
		return 0;

	int frontFloor = coll->Front.Floor + 700;
	int frontCeiling = coll->Front.Ceiling + 700;
	if (frontFloor <= -512 || frontFloor > 316)
		return 0;

	short rot = item->pos.yRot;
	int slope = 0;

	if (abs(coll->FrontRight.Floor - coll->FrontLeft.Floor) >= 60)
		return 0;

	bool result = SnapToQuadrant(rot, 35);
	if (!result)
		return 0;

	item->pos.yPos += frontFloor - 5;

	UpdateItemRoom(item, -LARA_HEIGHT / 2);


	Vector2 v = GetOrthogonalIntersect(item->pos.xPos, item->pos.zPos, -LARA_RAD, item->pos.yRot);
	item->pos.xPos = v.x;
	item->pos.zPos = v.y;

	if (frontFloor <= -256)
	{
		if ((LaraCeilingFront(item, item->pos.yRot, 384, 512) >= -512) && EnableCrawlFlexWaterPullUp == true)
		{
			item->animNumber = LA_ONWATER_TO_CROUCH_1CLICK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LA_CROUCH_IDLE;
		}
		else
		{
			item->animNumber = LA_ONWATER_TO_STAND_1CLICK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_STOP;
		}
	}
	else if (frontFloor > 128)
	{
		if ((LaraCeilingFront(item, item->pos.yRot, 384, 512) >= -512) && EnableCrawlFlexSubmerged == true)
		{
			item->animNumber = LA_ONWATER_TO_CROUCH_M1CLICK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LA_CROUCH_IDLE;
		}
		else
			item->animNumber = LA_ONWATER_TO_STAND_M1CLICK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	}

	else
	{
		if ((LaraCeilingFront(item, item->pos.yRot, 384, 512) >= -512) && EnableCrawlFlexWaterPullUp == true)
		{
			item->animNumber = LA_ONWATER_TO_CROUCH_0CLICK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LA_CROUCH_IDLE;
		}
		else
		{
			item->animNumber = LA_ONWATER_TO_STAND_0CLICK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_STOP;
		}


	}
	
	item->currentAnimState = LS_ONWATER_EXIT;
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

int LaraTestWaterStepOut(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT 
		|| coll->Middle.Slope  
		|| coll->Middle.Floor >= 0)
	{
		return 0;
	}

	if (coll->Middle.Floor >= -128)
	{
		if (item->goalAnimState == LS_ONWATER_LEFT)
		{
			item->goalAnimState = LS_STEP_LEFT;
		}
		else if (item->goalAnimState == LS_ONWATER_RIGHT)
		{
			item->goalAnimState = LS_STEP_RIGHT;
		}
		else
		{
			item->animNumber = LA_WADE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_WADE_FORWARD;
			item->currentAnimState = LS_WADE_FORWARD;
		}
	}
	else
	{
		item->animNumber = LA_ONWATER_TO_WADE_1CLICK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_ONWATER_EXIT;
		item->goalAnimState = LS_STOP;
	}

	item->pos.yPos += coll->Front.Floor + 695;

	UpdateItemRoom(item, -381);

	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;

	Lara.waterStatus = LW_WADE;

	return 1;
}

int LaraTestLadderClimbOut(ITEM_INFO* item, COLL_INFO* coll) // NEW function for water to ladder move
{
	if (!Lara.climbStatus || coll->CollisionType != CT_FRONT || !(TrInput & IN_ACTION))
		return 0;

	if (Lara.gunStatus && (Lara.gunStatus != LG_READY || Lara.gunType != WEAPON_FLARE))
		return 0;

	if (!TestLaraClimbStance(item, coll))
		return 0;
	
	short rot = item->pos.yRot;

	if (rot >= -ANGLE(35.0f) && rot <= ANGLE(35.0f))
		rot = 0;
	else if (rot >= ANGLE(55.0f) && rot <= ANGLE(125.0f))
		rot = ANGLE(90.0f);
	else if (rot >= ANGLE(145.0f) || rot <= -ANGLE(145.0f))
		rot = ANGLE(180.0f);
	else if (rot >= -ANGLE(125.0f) && rot <= -ANGLE(55.0f))
		rot = -ANGLE(90.0f);

	if (rot & 0x3FFF)
		return 0;

	switch ((unsigned short)rot / ANGLE(90.0f))
	{
	case NORTH:
		item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - LARA_RAD - 1;
		break;

	case EAST:
		item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - LARA_RAD - 1;
		break;

	case SOUTH:
		item->pos.zPos = (item->pos.zPos & -WALL_SIZE) + LARA_RAD + 1;
		break;

	case WEST:
		item->pos.xPos = (item->pos.xPos & -WALL_SIZE) + LARA_RAD + 1;
		break;
	}

	item->animNumber = LA_ONWATER_IDLE;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = LS_ONWATER_STOP;
	item->goalAnimState = LS_LADDER_IDLE;
	AnimateLara(item);

	item->pos.yRot = rot;
	item->pos.yPos -= 10;//otherwise she falls back into the water
	Lara.gunStatus = LG_HANDS_BUSY;
	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;
	Lara.waterStatus = LW_ABOVE_WATER;
	
	return 1;
}

void lara_as_waterout(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_HIPS;	//forces the camera to follow Lara instead of snapping
}

