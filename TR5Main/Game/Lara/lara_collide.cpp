#include "framework.h"
#include "lara.h"
#include "input.h"
#include "draw.h"
#include "effect2.h"

/*this file has all the generic **collision** test functions called in lara's state code*/

int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (coll->collType == CT_FRONT || coll->collType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->goalAnimState = LS_STOP;
		item->speed = 0;
		item->gravityStatus = false;

		return 1;
	}

	if (coll->collType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(5.0f);
	}
	else if (coll->collType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(5.0f);
	}

	return 0;
}

void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll)//12904, 129B4 (F)
{
	ShiftItem(item, coll);

	switch (coll->collType)
	{
	case CT_FRONT:
	case CT_TOP_FRONT:
		if (!Lara.climbStatus || item->speed != 2)
		{
			if (coll->midFloor <= 512)
			{
				if (coll->midFloor <= 128)
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
		item->pos.xPos -= 400 * phd_sin(coll->facing);
		item->pos.zPos -= 400 * phd_cos(coll->facing);

		item->speed = 0;
		coll->midFloor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}

int LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (coll->collType == CT_FRONT || coll->collType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->gravityStatus = false;
		item->speed = 0;

		return 1;
	}

	if (coll->collType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(2.0f);
	}
	else if (coll->collType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(2.0f);
	}

	return 0;
}

int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (coll->collType == CT_TOP || coll->collType == CT_CLAMP)
	{
		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->pos.zPos = coll->old.z;

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

void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll)//126F0(<), 127A0(<) (F)
{
	switch (coll->oldAnimState)
	{
	case LS_STOP:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_FAST:
		item->currentAnimState = coll->oldAnimState;
		item->animNumber = coll->oldAnimNumber;
		item->frameNumber = coll->oldFrameNumber;
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
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[LA_STAND_SOLID].frameBase;
		break;
	}
}

void SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle) // (F) (D)
{
	if (item->currentAnimState == LS_SHIMMY_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x390;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0x70;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0x70;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x390;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0x70;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x390;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x390;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0x70;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_FEET_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x720;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0xE0;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0xE0;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x720;
			return;
		}
	}

	if (item->currentAnimState == LS_SHIMMY_FEET_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->pos.xPos = coll->old.x & 0xFFFFFC70 | 0xE0;
			return;
		case EAST:
			item->pos.zPos = coll->old.z & 0xFFFFFF90 | 0x720;
			return;
		case SOUTH:
			item->pos.xPos = coll->old.x & 0xFFFFFF90 | 0x720;
			return;
		case WEST:
		default:
			item->pos.zPos = coll->old.z & 0xFFFFFC70 | 0xE0;
			return;
		}
	}
}

short GetDirOctant(int rot)//160B4(<), 161E8(<) (F)
{
	return abs(rot) >= ANGLE(45) && abs(rot) <= ANGLE(135.0f);
}

void GetLaraDeadlyBounds() // (F) (D)
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
