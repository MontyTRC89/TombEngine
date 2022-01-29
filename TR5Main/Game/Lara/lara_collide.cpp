#include "framework.h"
#include "Game/Lara/lara_collide.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Scripting/ScriptInterfaceFlow.h"
#include <ScriptInterfaceLevel.h>

// -----------------------------
// COLLISION TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->goalAnimState = LS_IDLE;
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

bool LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
{
	ShiftItem(item, coll);

	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		if (!Lara.climbStatus || item->speed != 2)
		{
			if (coll->Middle.Floor <= STEP_SIZE)
				SetAnimation(item, LA_LAND);
			else
				SetAnimation(item, LA_JUMP_WALL_SMASH_START, 1);

			item->speed /= 4;
			Lara.moveAngle += ANGLE(180.0f);

			if (item->fallspeed <= 0)
				item->fallspeed = 1;
		}

		return true;
	}

	if (coll->CollisionType == CT_TOP)
	{
		if (item->fallspeed <= 0)
			item->fallspeed = 1;
	}
	else if (coll->CollisionType == CT_LEFT)
		item->pos.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		item->pos.yRot -= ANGLE(5.0f);
	else if (coll->CollisionType == CT_CLAMP)
	{
		item->pos.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);
		item->pos.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);

		item->speed = 0;
		coll->Middle.Floor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;
	}

	return false;
}

bool LaraDeflectEdgeCrawl(ITEM_INFO* item, COLL_INFO* coll)
{
	// Useless in the best case; Lara does not have to embed in order to perform climbing actions in crawl states. Keeping for security. @Sezz 2021.11.26
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
		item->pos.yRot += ANGLE(coll->DiagonalStepAtLeft() ? DEFLECT_DIAGONAL_ANGLE_CRAWL : DEFLECT_STRAIGHT_ANGLE_CRAWL);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(coll->DiagonalStepAtRight() ? DEFLECT_DIAGONAL_ANGLE_CRAWL : DEFLECT_STRAIGHT_ANGLE_CRAWL);
	}

	return false;
}

// TODO: Move the following two functions to lara_tests.cpp and lara_helpers.cpp?
// @Sezz 2021.09.26
bool TestLaraHitCeiling(COLL_INFO* coll)
{
	if (coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_CLAMP)
	{
		return true;
	}

	return false;
}

void SetLaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll)
{
	item->pos.xPos = coll->Setup.OldPosition.x;
	item->pos.yPos = coll->Setup.OldPosition.y;
	item->pos.zPos = coll->Setup.OldPosition.z;

	item->speed = 0;
	item->fallspeed = 0;
	item->gravityStatus = false;
}

// LEGACY
// TODO: Gradually replace usage with TestLaraHitCeiling() and SetLaraHitCeiling(). @Sezz 2021.09.27
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
	case LS_IDLE:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_RIGHT_FAST:
	case LS_TURN_LEFT_FAST:
		item->currentAnimState = coll->Setup.OldAnimState;
		item->animNumber = coll->Setup.OldAnimNumber;
		item->frameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
		{
			// Prevent turn lock against walls.
			if (item->currentAnimState == LS_TURN_RIGHT_SLOW ||
				item->currentAnimState == LS_TURN_RIGHT_FAST)
			{
				item->goalAnimState = LS_IDLE;
			}
			else
				item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			if (item->currentAnimState == LS_TURN_LEFT_SLOW ||
				item->currentAnimState == LS_TURN_LEFT_FAST)
			{
				item->goalAnimState = LS_IDLE;
			}
			else
				item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
		else
			item->goalAnimState = LS_IDLE;

		AnimateLara(item);

		break;

	default:
		item->goalAnimState = LS_IDLE;

		if (item->animNumber != LA_STAND_SOLID)
			SetAnimation(item, LA_STAND_SOLID);

		break;
	}
}

void LaraCollideStopCrawl(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->Setup.OldAnimState)
	{
	case LS_CRAWL_IDLE:
	case LS_CRAWL_TURN_LEFT:
	case LS_CRAWL_TURN_RIGHT:
		item->currentAnimState = coll->Setup.OldAnimState;
		item->animNumber = coll->Setup.OldAnimNumber;
		item->frameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
			item->goalAnimState = LS_CRAWL_TURN_LEFT;
		else if (TrInput & IN_RIGHT)
			item->goalAnimState = LS_CRAWL_TURN_RIGHT;
		else
			item->goalAnimState = LS_CRAWL_IDLE;

		AnimateLara(item);
		break;

	default:
		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = GetFrameNumber(item, 0);
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

void LaraJumpCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*states 25, 26, 27*/
	/*state code: none, but is called in lara_col_backjump, lara_col_rightjump and lara_col_leftjump*/
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;

	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->Middle.Floor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
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
	if (height < ((level->GetLaraType() == LaraType::Divesuit) << 6) + 200)
		height = ((level->GetLaraType() == LaraType::Divesuit) << 6) + 200;

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

bool TestLaraObjectCollision(ITEM_INFO* item, short angle, int dist, int height)
{
	auto oldPos = item->pos;

	item->pos.xPos += dist * phd_sin(item->pos.yRot + angle);
	item->pos.yPos += height;
	item->pos.zPos += dist * phd_cos(item->pos.yRot + angle);

	auto result = GetCollidedObjects(item, LARA_RAD, 1, CollidedItems, CollidedMeshes, 0);

	item->pos = oldPos;

	return result;
}
