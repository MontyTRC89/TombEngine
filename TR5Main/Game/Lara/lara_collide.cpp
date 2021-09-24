#include "framework.h"
#include "lara.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "effects/effects.h"
#include "collide.h"
#include "control/control.h"
#include "lara_collide.h"
#include "item.h"
/*this file has all the generic **collision** test functions called in lara's state code*/

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
			if (coll->Middle.Floor <= 512)
			{
				if (coll->Middle.Floor <= 128)
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
		item->pos.yRot += ANGLE(2.0f);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(2.0f);
	}

	return 0;
}

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
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[LA_STAND_SOLID].frameBase;
		}
		break;
	}
}

void SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle)
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
