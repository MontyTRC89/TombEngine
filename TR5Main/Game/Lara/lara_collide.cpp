#include "framework.h"
#include "lara.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "effects/effects.h"
#include "collide.h"
#include "control/control.h"
#include "lara_collide.h"
#include "lara_swim.h"
#include "lara_tests.h"
#include "items.h"
#include "setup.h"
#include "GameFlowScript.h"
#include "GameScriptLevel.h"

/*this file has all the generic **collision** test functions called in lara's state code*/

bool LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->goalAnimState = LS_STOP;
		item->speed = 0;
		item->gravityStatus = false;

		return true;
	}

	if (coll->CollisionType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(coll->DiagonalStepAtLeft() ? DEFLECT_DIAGONAL_ANGLE : DEFLECT_STRAIGHT_ANGLE);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(coll->DiagonalStepAtRight() ? DEFLECT_DIAGONAL_ANGLE : DEFLECT_STRAIGHT_ANGLE);
	}

	return false;
}

void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
{
	ShiftItem(item, coll);

	switch (coll->CollisionType)
	{
	case CT_FRONT:
	case CT_TOP_FRONT:
		if (!Lara.climbStatus || item->speed != 2)
		{
			if (coll->Middle.Floor <= 512)
			{
				if (coll->Middle.Floor <= 128)
					SetAnimation(item, LA_JUMP_UP_LAND);
			}
			else
			{
				SetAnimation(item, LA_JUMP_WALL_SMASH_START, 1);
			}

			item->speed /= 4;
			Lara.moveAngle += ANGLE(180);

			if (item->fallspeed <= 0)
				item->fallspeed = 1;
		}

		break;
	case CT_TOP:
		if (item->fallspeed <= 0)
			item->fallspeed = 1;

		break;
	case CT_LEFT:
		item->pos.yRot += ANGLE(5.0f);
		break;
	case CT_RIGHT:
		item->pos.yRot -= ANGLE(5.0f);
		break;
	case CT_CLAMP:
		item->pos.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);
		item->pos.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);

		item->speed = 0;
		coll->Middle.Floor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}

bool LaraDeflectEdgeCrawl(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->gravityStatus = false;
		item->speed = 0;

		return true;
	}

	if (coll->CollisionType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(2.0f);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(2.0f);
	}

	return false;
}

bool LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_TOP || coll->CollisionType == CT_CLAMP)
	{
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;

		SetAnimation(item, LA_STAND_SOLID);

		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;

		return true;
	}

	return false;
}

void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->Setup.OldAnimState)
	{
	case LS_STOP:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_FAST:
		item->currentAnimState = coll->Setup.OldAnimState;
		item->animNumber = coll->Setup.OldAnimNumber;
		item->frameNumber = coll->Setup.OldFrameNumber;
		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
		AnimateLara(item);
		break;
	default:
		item->goalAnimState = LS_STOP;
		if (item->animNumber != LA_STAND_SOLID)
		{
			SetAnimation(item, LA_STAND_SOLID);
		}
		break;
	}
}

void LaraSnapToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	if (item->currentAnimState == LS_SHIMMY_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFF90 | 0x390;
			return;
		case EAST:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFC70 | 0x70;
			return;
		case SOUTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFC70 | 0x70;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFF90 | 0x390;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFC70 | 0x70;
			return;
		case EAST:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFF90 | 0x390;
			return;
		case SOUTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFF90 | 0x390;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFC70 | 0x70;
			return;
		}
	}
}

void LaraResetGravityStatus(ITEM_INFO* item, COLL_INFO* coll)
{
	// This routine cleans gravity status flag and fallspeed, making it
	// impossible to perform bugs such as QWOP and flare jump. Found by Troye -- Lwmte, 25.09.2021

	if (coll->Middle.Floor <= STEPUP_HEIGHT)
	{
		item->gravityStatus = false;
		item->fallspeed = 0;
	}
}

void LaraSnapToHeight(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraSwamp(item) && coll->Middle.Floor > 0)
		item->pos.yPos += SWAMP_GRAVITY;
	else if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;
}

short GetDirOctant(int rot)
{
	return abs(rot) >= ANGLE(45) && abs(rot) <= ANGLE(135.0f);
}

void GetLaraDeadlyBounds()
{
	BOUNDING_BOX* bounds;
	BOUNDING_BOX tbounds;

	bounds = GetBoundsAccurate(LaraItem);
	phd_RotBoundingBoxNoPersp(&LaraItem->pos, bounds, &tbounds);

	DeadlyBounds[0] = LaraItem->pos.xPos + tbounds.X1;
	DeadlyBounds[1] = LaraItem->pos.xPos + tbounds.X2;
	DeadlyBounds[2] = LaraItem->pos.yPos + tbounds.Y1;
	DeadlyBounds[3] = LaraItem->pos.yPos + tbounds.Y2;
	DeadlyBounds[4] = LaraItem->pos.zPos + tbounds.Z1;
	DeadlyBounds[5] = LaraItem->pos.zPos + tbounds.Z2;
}

void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item, PHD_VECTOR(0, LARA_HEIGHT_SURFSWIM, 0));
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
		TestLaraWaterStepOut(item, coll);
	}
	else
		SwimDive(item);
}

void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;
	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	if (item->pos.xRot < -ANGLE(90.0f) ||
		item->pos.xRot > ANGLE(90.0f))
	{
		Lara.moveAngle = item->pos.yRot + ANGLE(180.0f);
		coll->Setup.ForwardAngle = item->pos.yRot - ANGLE(180.0f);
	}
	else
	{
		Lara.moveAngle = item->pos.yRot;
		coll->Setup.ForwardAngle = item->pos.yRot;
	}

	int height = LARA_HEIGHT * phd_sin(item->pos.xRot);
	height = abs(height);

	auto level = g_GameFlow->GetLevel(CurrentLevel);
	if (height < ((level->LaraType == LaraType::Divesuit) << 6) + 200)
		height = ((level->LaraType == LaraType::Divesuit) << 6) + 200;

	coll->Setup.BadHeightUp = -(STEP_SIZE / 4);
	coll->Setup.Height = height;

	GetCollisionInfo(coll, item, PHD_VECTOR(0, height / 2, 0));

	auto c1 = *coll;
	c1.Setup.ForwardAngle += ANGLE(45.0f);
	GetCollisionInfo(&c1, item, PHD_VECTOR(0, height / 2, 0));

	auto c2 = *coll;
	c2.Setup.ForwardAngle -= ANGLE(45.0f);
	GetCollisionInfo(&c2, item, PHD_VECTOR(0, height / 2, 0));

	ShiftItem(item, coll);

	int flag = 0;

	switch (coll->CollisionType)
	{
	case CT_FRONT:
		if (item->pos.xRot <= ANGLE(25.0f))
		{
			if (item->pos.xRot >= -ANGLE(25.0f))
			{
				if (item->pos.xRot > ANGLE(5.0f))
					item->pos.xRot += ANGLE(0.5f);
				else if (item->pos.xRot < -ANGLE(5.0f))
					item->pos.xRot -= ANGLE(0.5f);
				else if (item->pos.xRot > 0)
					item->pos.xRot += 45;
				else if (item->pos.xRot < 0)
					item->pos.xRot -= 45;
				else
				{
					item->fallspeed = 0;
					flag = 1;
				}
			}
			else
			{
				item->pos.xRot -= ANGLE(1.0f);
				flag = 1;
			}
		}
		else
		{
			item->pos.xRot += ANGLE(1.0f);
			flag = 1;
		}

		if (c1.CollisionType == CT_LEFT)
			item->pos.yRot += ANGLE(2.0f);
		else if (c1.CollisionType == CT_RIGHT)
			item->pos.yRot -= ANGLE(2.0f);
		else if (c2.CollisionType == CT_LEFT)
			item->pos.yRot += ANGLE(2.0f);
		else if (c2.CollisionType == CT_RIGHT)
			item->pos.yRot -= ANGLE(2.0f);

		break;

	case CT_TOP:
		if (item->pos.xRot >= -8190)
		{
			flag = 1;
			item->pos.xRot -= ANGLE(1.0f);
		}

		break;

	case CT_TOP_FRONT:
		item->fallspeed = 0;
		flag = 1;

		break;

	case CT_LEFT:
		item->pos.yRot += ANGLE(2.0f);
		flag = 1;

		break;

	case CT_RIGHT:
		item->pos.yRot -= ANGLE(2.0f);
		flag = 1;

		break;

	case CT_CLAMP:
		flag = 2;
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
		item->fallspeed = 0;

		break;
	}

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
	{
		flag = 1;
		item->pos.xRot += ANGLE(1.0f);
		item->pos.yPos += coll->Middle.Floor;
	}

	if (oldX == item->pos.xPos &&
		oldY == item->pos.yPos &&
		oldZ == item->pos.zPos &&
		oldXrot == item->pos.xRot &&
		oldYrot == item->pos.yRot ||
		flag != 1)
	{
		if (flag == 2)
			return;
	}

	if (Lara.waterStatus != LW_FLYCHEAT && Lara.ExtraAnim == NO_ITEM)
		TestLaraWaterDepth(item, coll);
}
