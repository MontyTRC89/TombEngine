#include "framework.h"
#include "lara.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "effects/effects.h"
#include "collide.h"
#include "control/control.h"
#include "lara_collide.h"
#include "lara_tests.h"
#include "items.h"
#include "setup.h"

// -----------------------------
// COLLISION TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->goalAnimState = LS_STOP;
		item->speed = 0;
		item->gravityStatus = false;

		return 1;
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

	return 0;
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
			if (coll->Middle.Floor <= STOP_SIZE)
			{
				if (coll->Middle.Floor <= STEP_SIZE / 2)
				{
					item->goalAnimState = LS_GRAB_TO_FALL;
					item->currentAnimState = LS_GRAB_TO_FALL;

					item->animNumber = LA_JUMP_UP_LAND;
					item->frameNumber = g_Level.Anims[LA_JUMP_UP_LAND].frameBase;
				}
			}
			else
			{
				item->goalAnimState = LS_FREEFALL;
				item->currentAnimState = LS_FREEFALL;

				item->animNumber = LA_JUMP_WALL_SMASH_START;
				item->frameNumber = g_Level.Anims[LA_JUMP_WALL_SMASH_START].frameBase + 1;
			}

			item->speed /= 4;
			Lara.moveAngle += ANGLE(180.0f);

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

int LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->gravityStatus = false;
		item->speed = 0;

		return 1;
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

	return 0;
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
int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_TOP || coll->CollisionType == CT_CLAMP)
	{
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;

		item->goalAnimState = LS_STOP;
		item->currentAnimState = LS_STOP;
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;

		return 1;
	}
	return 0;
}

void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->Setup.OldAnimState)
	{
	case LS_STOP:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_RIGHT_FAST:
	case LS_TURN_LEFT_FAST:
		item->currentAnimState = coll->Setup.OldAnimState;
		item->animNumber = coll->Setup.OldAnimNumber;
		item->frameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		else if (TrInput & IN_RIGHT)
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		else
			item->goalAnimState = LS_STOP;

		AnimateLara(item);
		break;

	default:
		item->goalAnimState = LS_STOP;

		if (item->animNumber != LA_STAND_SOLID)
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = GF(LA_STAND_SOLID, 0);
		}

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
			item->frameNumber = GF(LA_CRAWL_IDLE, 0);
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

	if (item->currentAnimState == LS_SHIMMY_FEET_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFF90 | 0x720;
			return;
		case EAST:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFC70 | 0xE0;
			return;
		case SOUTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFC70 | 0xE0;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFF90 | 0x720;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_FEET_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFC70 | 0xE0;
			return;
		case EAST:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFF90 | 0x720;
			return;
		case SOUTH:
			item->pos.xPos = coll->Setup.OldPosition.x & 0xFFFFFF90 | 0x720;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->Setup.OldPosition.z & 0xFFFFFC70 | 0xE0;
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
