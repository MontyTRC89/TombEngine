#include "framework.h"
#include "lara_climb.h"
#include "Lara.h"
#include "control/control.h"
#include "animation.h"
#include "sphere.h"
#include "camera.h"
#include "level.h"
#include "input.h"
#include "items.h"

short GetClimbFlags(int x, int y, int z, short roomNumber)
{
	return GetClimbFlags(GetFloor(x, y, z, &roomNumber));
}

short GetClimbFlags(FLOOR_INFO* floor)
{
	short result = 0;

	if (floor->Flags.ClimbEast)
		result |= (short)CLIMB_DIRECTION::East;
	if (floor->Flags.ClimbWest)
		result |= (short)CLIMB_DIRECTION::West;
	if (floor->Flags.ClimbNorth)
		result |= (short)CLIMB_DIRECTION::North;
	if (floor->Flags.ClimbSouth)
		result |= (short)CLIMB_DIRECTION::South;

	return result;
}

void lara_col_climbend(ITEM_INFO* item, COLL_INFO* coll)
{
	return;
}

void lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = -ANGLE(45);
}

void lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (LaraCheckForLetGo(item, coll) 
		|| item->animNumber != LA_LADDER_DOWN)
		return;

	int frame = item->frameNumber - g_Level.Anims[LA_LADDER_DOWN].frameBase;
	int xShift;
	int yShift;

	switch (frame)
	{
	case 0:
		yShift = 0;
		break;

	case 28:
	case 29:
		yShift = CLICK(1);
		break;

	case 57:
		yShift = CLICK(2);
		break;

	default:
		return;
	}

	item->pos.yPos += yShift + CLICK(1);

	int shiftLeft = 0;
	int shiftRight = 0;
	int resultRight = LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -CLICK(2), CLICK(2), &shiftRight);
	int resultLeft = LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -CLICK(2), CLICK(2), &shiftLeft);

	item->pos.yPos -= CLICK(1);

	if (resultRight != 0 && resultLeft != 0 &&
		resultRight != -2 && resultLeft != -2 &&
		TrInput & IN_BACK)
	{
		if (shiftRight && shiftLeft)
		{
			if (shiftRight < 0 != shiftLeft < 0)
			{
				item->goalAnimState = LS_LADDER_IDLE;
				AnimateLara(item);
				return;
			}

			if (shiftRight < 0 && shiftRight < shiftLeft ||
				shiftRight > 0 && shiftRight > shiftLeft)
			{
				shiftLeft = shiftRight;
			}
		}

		if (resultRight == -1 || resultLeft == -1)
		{
			SetAnimation(item, LA_LADDER_IDLE);
			item->goalAnimState = LS_HANG;

			AnimateLara(item);
		}
		else
		{
			item->goalAnimState = LS_LADDER_DOWN;
			item->pos.yPos -= yShift;
		}
		return;
	}

	item->goalAnimState = LS_LADDER_IDLE;

	if (yShift != 0)
		AnimateLara(item);
}

void lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.targetElevation = -ANGLE(45);
}

void lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!LaraCheckForLetGo(item, coll) && item->animNumber == LA_LADDER_UP)
	{
		int frame = item->frameNumber - g_Level.Anims[LA_LADDER_UP].frameBase;
		int yShift;
		int resultRight, resultLeft;
		int shiftRight, shiftLeft;
		int ledgeRight, ledgeLeft;

		if (frame == 0)
		{
			yShift = 0;
		}
		else if (frame == 28 || frame == 29)
		{
			yShift = -CLICK(1);
		}
		else if (frame == 57)
		{
			yShift = -CLICK(2);
		}
		else
		{
			return;
		}

		item->pos.yPos += yShift - CLICK(1);

		resultRight = LaraTestClimbUpPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), &shiftRight, &ledgeRight);
		resultLeft = LaraTestClimbUpPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), &shiftLeft, &ledgeLeft);

		item->pos.yPos += CLICK(1);
		 
		if (resultRight && resultLeft && TrInput & IN_FORWARD)
		{
			if (resultRight < 0 || resultLeft < 0)
			{
				item->goalAnimState = LS_LADDER_IDLE;

				AnimateLara(item);

				if (abs(ledgeRight - ledgeLeft) <= CLICK(0.5f))
				{
					if (resultRight != -1 || resultLeft != -1)
					{
						item->goalAnimState = LS_LADDER_TO_CROUCH;
						item->requiredAnimState = LS_CROUCH_IDLE;
					}
					else
					{
						item->goalAnimState = LS_GRABBING;
						item->pos.yPos += (ledgeRight + ledgeLeft) / 2 - CLICK(1);
					}
				}
			}
			else
			{
				item->goalAnimState = LS_LADDER_UP;
				item->pos.yPos -= yShift;
			}
		}
		else
		{
			item->goalAnimState = LS_LADDER_IDLE;

			if (yShift != 0)
				AnimateLara(item);
		}
	}
}

void lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.targetElevation = ANGLE(30);
}

void lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!LaraCheckForLetGo(item, coll))
	{
		int shift = 0;
		Lara.moveAngle = item->pos.yRot + ANGLE(90);
		LaraDoClimbLeftRight(item, coll, LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -CLICK(2), CLICK(2), &shift), shift);
	}
}

void lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.targetAngle = ANGLE(30);
	Camera.targetElevation = -ANGLE(15);

	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = LS_LADDER_IDLE;
}

void lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!LaraCheckForLetGo(item, coll))
	{
		int shift = 0;
		Lara.moveAngle = item->pos.yRot - ANGLE(90);
		LaraDoClimbLeftRight(item, coll, LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -CLICK(2), CLICK(2), &shift), shift);
	}
}

void lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.targetAngle = -ANGLE(30);
	Camera.targetElevation = -ANGLE(15);

	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_LADDER_IDLE;
}

void lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll)
{
	int yShift;
	int resultRight, resultLeft;
	int shiftRight, shiftLeft;
	int ledgeRight, ledgeLeft;

	if (LaraCheckForLetGo(item, coll) || item->animNumber != LA_LADDER_IDLE)
		return;

	if (!(TrInput & IN_FORWARD))
	{
		if (!(TrInput & IN_BACK))
			return;

		if (item->goalAnimState == LS_HANG)
			return;

		item->goalAnimState = LS_LADDER_IDLE;
		item->pos.yPos += CLICK(1);
		
		resultRight = LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -CLICK(2), CLICK(2), &ledgeRight);
		resultLeft = LaraTestClimbPos(item, coll->Setup.Radius, -CLICK(0.5f) - coll->Setup.Radius, -CLICK(2), CLICK(2), &ledgeLeft);
		
		item->pos.yPos -= CLICK(1);
		
		if (!resultRight || !resultLeft || resultLeft == -2 || resultRight == -2)
			return;

		yShift = ledgeLeft;

		if (ledgeRight && ledgeLeft)
		{
			if (ledgeLeft < 0 != ledgeRight < 0)
				return;
			if (ledgeRight < 0 == ledgeRight < ledgeLeft)
				yShift = ledgeRight;
		}

		if (resultRight == 1 && resultLeft == 1)
		{
			item->goalAnimState = LS_LADDER_DOWN;
			item->pos.yPos += yShift;
		}
		else
		{
			item->goalAnimState = LS_HANG;
		}
	}
	else if (item->goalAnimState != LS_GRABBING)
	{
		item->goalAnimState = LS_LADDER_IDLE;
		resultRight = LaraTestClimbUpPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), &shiftRight, &ledgeRight);
		resultLeft = LaraTestClimbUpPos(item, coll->Setup.Radius, -CLICK(0.5f) - coll->Setup.Radius, &shiftLeft, &ledgeLeft);

		if (!resultRight || !resultLeft)
			return;

		if (resultRight >= 0 && resultLeft >= 0)
		{
			yShift = shiftLeft;

			if (shiftRight)
			{
				if (shiftLeft)
				{
					if (shiftLeft < 0 != shiftRight < 0)
						return;
					if (shiftRight < 0 == shiftRight < shiftLeft)
						yShift = shiftRight;
				}
				else
				{
					yShift = shiftRight;
				}
			}

			item->goalAnimState = LS_LADDER_UP;
			item->pos.yPos += yShift;
		}
		else if (abs(ledgeLeft - ledgeRight) <= CLICK(0.5f))
		{
			if (resultRight == -1 && resultLeft == -1)
			{
				item->goalAnimState = LS_GRABBING;
				item->pos.yPos += (ledgeRight + ledgeLeft) / 2 - CLICK(1);
			}
			else
			{
				item->goalAnimState = LS_LADDER_TO_CROUCH;
				item->requiredAnimState = LS_CROUCH_IDLE;
			}
		}
	}
}

void lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.isClimbing = true;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

	Camera.targetElevation = -ANGLE(20);

	if (item->animNumber == LA_LADDER_DISMOUNT_LEFT_START)
		Camera.targetAngle = -ANGLE(60.0f);
	if (item->animNumber == LA_LADDER_DISMOUNT_RIGHT_START)
		Camera.targetAngle = ANGLE(60.0f);

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_LADDER_LEFT;
		Lara.moveAngle = item->pos.yRot - ANGLE(90);
	}
	else if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_LADDER_RIGHT;
		Lara.moveAngle = item->pos.yRot + ANGLE(90);
	}
	else if (TrInput & IN_JUMP)
	{
		if (item->animNumber == LA_LADDER_IDLE)
		{
			item->goalAnimState = LS_JUMP_BACK;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.moveAngle = item->pos.yRot + ANGLE(180);
		}
	}
}

void lara_as_stepoff_left(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.targetAngle = -ANGLE(60.0f);
	Camera.targetElevation = -ANGLE(15.0f);

	item->pos.yRot -= ANGLE(90.0f);
}

void lara_as_stepoff_right(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	Camera.targetAngle = ANGLE(60.0f);
	Camera.targetElevation = -ANGLE(15.0f);

	item->pos.yRot += ANGLE(90.0f);
}

int LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift)
{
	int x;
	int z;
	int xFront = 0;
	int zFront = 0;

	switch (GetQuadrant(item->pos.yRot))
	{
	case NORTH:
		x = item->pos.xPos + right;
		z = item->pos.zPos + front;
		zFront = CLICK(1);
		break;

	case EAST:
		x = item->pos.xPos + front;
		z = item->pos.zPos - right;
		xFront = CLICK(1);
		break;

	case SOUTH:
		x = item->pos.xPos - right;
		z = item->pos.zPos - front;
		zFront = -CLICK(1);
		break;

	case WEST:
	default:
		x = item->pos.xPos - front;
		z = item->pos.zPos + right;
		xFront = -CLICK(1);
		break;
	}

	return LaraTestClimb(x, item->pos.yPos + origin, z, xFront, zFront, height, item->roomNumber, shift);
}

void LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int result, int shift)
{
	if (result == 1)
	{
		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_LADDER_LEFT;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_LADDER_RIGHT;
		}
		else
		{
			item->goalAnimState = LS_LADDER_IDLE;
		}

		item->pos.yPos += shift;

		return;
	}

	if (result != 0)
	{
		item->goalAnimState = LS_HANG;

		do
		{
			AnimateItem(item);
		} while (item->currentAnimState != LS_HANG);

		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.zPos = coll->Setup.OldPosition.z;

		return;
	}

	item->pos.xPos = coll->Setup.OldPosition.x;
	item->pos.zPos = coll->Setup.OldPosition.z;

	item->goalAnimState = LS_LADDER_IDLE;
	item->currentAnimState = LS_LADDER_IDLE;

	if (coll->Setup.OldAnimState != LS_LADDER_IDLE)
	{	
		SetAnimation(item, LA_LADDER_IDLE);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		short troomnumber = item->roomNumber;
		int dx = int(sin(TO_RAD(item->pos.yRot - ANGLE(90.0f))) * 10);
		int dz = int(cos(TO_RAD(item->pos.yRot - ANGLE(90.0f))) * 10);
		int height = GetFloorHeight(GetFloor(item->pos.xPos + dx, item->pos.yPos, item->pos.zPos + dz, &troomnumber),
			item->pos.xPos, item->pos.yPos, item->pos.zPos) - item->pos.yPos;
		if (height < 3 * STEP_SIZE / 2) // LADDER dismounts (left/right)
		{
			item->goalAnimState = LS_LADDER_DISMOUNT_LEFT;
			item->currentAnimState = LS_MISC_CONTROL;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		short troomnumber = item->roomNumber;
		int dx = int(sin(TO_RAD(item->pos.yRot + ANGLE(90.0f))) * 10);
		int dz = int(cos(TO_RAD(item->pos.yRot + ANGLE(90.0f))) * 10);
		int height = GetFloorHeight(GetFloor(item->pos.xPos + dx, item->pos.yPos, item->pos.zPos + dz, &troomnumber),
			item->pos.xPos, item->pos.yPos, item->pos.zPos) - item->pos.yPos;
		if (height < 3 * STEP_SIZE / 2) // LADDER dismounts (left/right)
		{
			item->goalAnimState = LS_LADDER_DISMOUNT_RIGHT;
			item->currentAnimState = LS_MISC_CONTROL;
		}
	}

	if (TrInput & IN_LEFT)
	{
		int flag = LaraClimbLeftCornerTest(item, coll);

		if (flag)
		{
			if (flag <= 0)
				SetAnimation(item, LA_LADDER_LEFT_CORNER_INNER_START);
			else
				SetAnimation(item, LA_LADDER_LEFT_CORNER_OUTER_START);

			return;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		int flag = LaraClimbRightCornerTest(item, coll);

		if (flag)
		{
			if (flag <= 0)
				SetAnimation(item, LA_LADDER_RIGHT_CORNER_INNER_START);
			else
				SetAnimation(item, LA_LADDER_RIGHT_CORNER_OUTER_START);

			return;
		}
	}

	item->animNumber = coll->Setup.OldAnimNumber;
	item->frameNumber = coll->Setup.OldFrameNumber;

	AnimateLara(item);
}

int LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll)
{
	int result = 0;

	if (item->animNumber != LA_LADDER_RIGHT)
		return 0;

	auto oldPos = item->pos;
	auto oldRot = Lara.moveAngle;

	short angle = GetQuadrant(item->pos.yRot);
	int x, z;

	if (angle && angle != SOUTH)
	{
		x = (item->pos.xPos & -WALL_SIZE) - (item->pos.zPos % WALL_SIZE) + WALL_SIZE;
		z = (item->pos.zPos & -WALL_SIZE) - (item->pos.xPos % WALL_SIZE) + WALL_SIZE;
	}
	else
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & (WALL_SIZE - 1);
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & (WALL_SIZE - 1);
	}

	int shift = 0;

	if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & (short)LeftExtRightIntTab[angle])
	{
		Lara.nextCornerPos.xPos = item->pos.xPos = x;
		Lara.nextCornerPos.yPos = item->pos.yPos;
		Lara.nextCornerPos.zPos = item->pos.zPos = z;
		Lara.nextCornerPos.yRot = item->pos.yRot = Lara.moveAngle = item->pos.yRot + ANGLE(90);

		result = LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -CLICK(2), CLICK(2), &shift);
		item->itemFlags[3] = result;
	}

	if (!result)
	{
		item->pos = oldPos;
		Lara.moveAngle = oldRot;

		switch (angle)
		{
		case NORTH:
			x = ((item->pos.xPos + WALL_SIZE) & -WALL_SIZE) - (item->pos.zPos % WALL_SIZE) + WALL_SIZE;
			z = ((item->pos.zPos + WALL_SIZE) & -WALL_SIZE) - (item->pos.xPos % WALL_SIZE) + WALL_SIZE;
			break;

		case SOUTH:
			x = ((item->pos.xPos - WALL_SIZE) & -WALL_SIZE) - (item->pos.zPos % WALL_SIZE) + WALL_SIZE;
			z = ((item->pos.zPos - WALL_SIZE) & -WALL_SIZE) - (item->pos.xPos % WALL_SIZE) + WALL_SIZE;
			break;

		case EAST:
			x = ((item->pos.zPos ^ item->pos.xPos) % WALL_SIZE) ^ (item->pos.xPos + WALL_SIZE);
			z = (item->pos.zPos ^ ((item->pos.zPos ^ item->pos.xPos) % WALL_SIZE)) - WALL_SIZE;
			break;

		case WEST:
		default:
			x = (item->pos.xPos ^ (item->pos.zPos ^ item->pos.xPos) % WALL_SIZE) - WALL_SIZE;
			z = ((item->pos.zPos ^ item->pos.xPos) % WALL_SIZE) ^ (item->pos.zPos + WALL_SIZE);
			break;

		}

		if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & (short)LeftIntRightExtTab[angle])
		{
			Lara.nextCornerPos.xPos = item->pos.xPos = x;
			Lara.nextCornerPos.yPos = item->pos.yPos;
			Lara.nextCornerPos.zPos = item->pos.zPos = z;
			Lara.nextCornerPos.yRot = item->pos.yRot = Lara.moveAngle = item->pos.yRot - ANGLE(90);

			result = LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -CLICK(2), CLICK(2), &shift);
			item->itemFlags[3] = result;
		}
	}
	else
	{
		result = -1;
	}

	item->pos = oldPos;
	Lara.moveAngle = oldRot;

	return result;
}

int LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll)
{
	int result = 0;

	if (item->animNumber != LA_LADDER_LEFT)
		return 0;

	auto oldPos = item->pos;
	auto oldRot = Lara.moveAngle;

	short angle = GetQuadrant(item->pos.yRot);
	int x, z;

	if (angle && angle != SOUTH)
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & (WALL_SIZE - 1);
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & (WALL_SIZE - 1);
	}
	else
	{
		x = (item->pos.xPos & -WALL_SIZE) - (item->pos.zPos & (WALL_SIZE - 1)) + WALL_SIZE;
		z = (item->pos.zPos & -WALL_SIZE) - (item->pos.xPos & (WALL_SIZE - 1)) + WALL_SIZE;
	}

	int shift = 0;

	if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & (short)LeftIntRightExtTab[angle])
	{
		Lara.nextCornerPos.xPos = item->pos.xPos = x;
		Lara.nextCornerPos.yPos = item->pos.yPos;
		Lara.nextCornerPos.zPos = item->pos.zPos = z;
		Lara.nextCornerPos.yRot = item->pos.yRot = Lara.moveAngle = item->pos.yRot - ANGLE(90);

		result = LaraTestClimbPos(item, coll->Setup.Radius, -coll->Setup.Radius - CLICK(0.5f), -CLICK(2), CLICK(2), &shift);
		item->itemFlags[3] = result;
	}

	if (!result)
	{
		item->pos = oldPos;
		Lara.moveAngle = oldRot;

		switch (angle)
		{
		case NORTH:
			x = (item->pos.xPos ^ ((item->pos.zPos ^ item->pos.xPos) & (WALL_SIZE - 1))) - WALL_SIZE;
			z = ((item->pos.zPos ^ item->pos.xPos) & (WALL_SIZE - 1)) ^ (item->pos.zPos + WALL_SIZE);
			break;

		case SOUTH:
			x = ((item->pos.zPos ^ item->pos.xPos) & (WALL_SIZE - 1)) ^ (item->pos.xPos + WALL_SIZE);
			z = ((item->pos.zPos ^ item->pos.xPos) & (WALL_SIZE - 1)) ^ (item->pos.zPos - WALL_SIZE);
			break;

		case EAST:
			x = ((item->pos.xPos + WALL_SIZE) & -WALL_SIZE) - (item->pos.zPos & (WALL_SIZE - 1)) + WALL_SIZE;
			z = ((item->pos.zPos + WALL_SIZE) & -WALL_SIZE) - (item->pos.xPos & (WALL_SIZE - 1)) + WALL_SIZE;
			break;

		case WEST:
		default:
			x = (item->pos.xPos & -WALL_SIZE) - (item->pos.zPos & (WALL_SIZE - 1));
			z = (item->pos.zPos & -WALL_SIZE) - (item->pos.xPos & (WALL_SIZE - 1));
			break;

		}

		if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & (short)LeftExtRightIntTab[angle])
		{
			Lara.nextCornerPos.xPos = item->pos.xPos = x;
			Lara.nextCornerPos.yPos = item->pos.yPos;
			Lara.nextCornerPos.zPos = item->pos.zPos = z;
			Lara.nextCornerPos.yRot = item->pos.yRot = Lara.moveAngle = item->pos.yRot + ANGLE(90);

			item->itemFlags[3] = LaraTestClimbPos(item, coll->Setup.Radius, -coll->Setup.Radius - CLICK(0.5f), -CLICK(2), CLICK(2), &shift);
			result = item->itemFlags[3] != 0;
		}
	}
	else
	{
		result = -1;
	}

	item->pos = oldPos;
	Lara.moveAngle = oldRot;

	return result;
}

int LaraTestClimb(int x, int y, int z, int xFront, int zFront, int itemHeight, int itemRoom, int* shift)
{
	*shift = 0;
	int hang = 1;
	if (!Lara.climbStatus)
		return 0;

	short roomNumber = itemRoom;
	FLOOR_INFO* floor = GetFloor(x, y - CLICK(0.5f), z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);
	if (height == NO_HEIGHT)
		return 0;

	height -= (CLICK(0.5f) + y + itemHeight);
	if (height < -70)
		return 0;
	if (height < 0)
		*shift = height;

	int ceiling = GetCeiling(floor, x, y, z) - y;
	if (ceiling > 70)
		return 0;
	if (ceiling > 0)
	{
		if (*shift)
			return 0;
		*shift = ceiling;
	}

	if (itemHeight + height < 900)
		hang = 0;

	int dz = zFront + z;
	int dx = xFront + x;
	
	floor = GetFloor(dx, y, dz, &roomNumber);
	height = GetFloorHeight(floor, dx, y, dz);
	
	if (height != NO_HEIGHT)
	{
		height -= y;
	}

	if (height <= 70)
	{
		if (height > 0)
		{
			if (*shift < 0)
				return 0;
			if (height > *shift)
				*shift = height;
		}
		
		roomNumber = itemRoom;
		GetFloor(x, y + itemHeight, z, &roomNumber);
		FLOOR_INFO* floor2 = GetFloor(dx, y + itemHeight, dz, &roomNumber);
		ceiling = GetCeiling(floor2, dx, y + itemHeight, dz);
		
		if (ceiling == NO_HEIGHT)
			return 1;
		if (ceiling - y <= height)
			return 1;
		if (ceiling - y >= CLICK(2))
			return 1;
		if (ceiling - y <= 442)
			return -(hang != 0);
		if (*shift > 0)
			return -(hang != 0);
		*shift = ceiling - y - CLICK(2);

		return 1;
	}
	
	ceiling = GetCeiling(floor, dx, y, dz) - y;
	if (ceiling >= CLICK(2))
		return 1;
	if (ceiling > 442)
	{
		if (*shift > 0)
			return -(hang != 0);
		*shift = ceiling - CLICK(2);

		return 1;
	}

	if (ceiling > 0)
		return -(hang != 0);
	if (ceiling <= -70 || !hang || *shift > 0)
		return 0;
	if (*shift > ceiling)
		*shift = ceiling;

	return -1;
}

int LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge)
{
	int y = item->pos.yPos - 768;

	int x, z;
	int xFront = 0;
	int zFront = 0;

	switch (GetQuadrant(item->pos.yRot))
	{
	case NORTH:
		x = item->pos.xPos + right;
		z = item->pos.zPos + front;
		zFront = 4;
		break;
	case EAST:
		x = item->pos.xPos + front;
		z = item->pos.zPos - right;
		xFront = 4;
		break;
	case SOUTH:
		x = item->pos.xPos - right;
		z = item->pos.zPos - front;
		zFront = -4;
		break;
	default:
		x = item->pos.xPos - front;
		z = item->pos.zPos + right;
		xFront = -4;
		break;
	}

	*shift = 0;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int ceiling = CLICK(1) - y + GetCeiling(floor, x, y, z);
	
	if (ceiling > 70)
		return 0;

	if (ceiling > 0)
		*shift = ceiling;

	floor = GetFloor(x + xFront, y, z + zFront, &roomNumber);
	int height = GetFloorHeight(floor, x + xFront, y, z + zFront);
	
	if (height == NO_HEIGHT)
	{
		*ledge = NO_HEIGHT;
		return 1;
	}
	else
	{
		height -= y;
		*ledge = height;
		
		if (height <= CLICK(0.5f))
		{
			if (height > 0 && height > *shift)
				*shift = height;

			roomNumber = item->roomNumber;
			GetFloor(x, y + CLICK(2), z, &roomNumber);
			floor = GetFloor(x + xFront, y + CLICK(2), z + zFront, &roomNumber);
			ceiling = GetCeiling(floor, x + xFront, y + CLICK(2), z + zFront) - y;
			if (ceiling <= height)
				return 1;						
			if (ceiling >= CLICK(2))
				return 1;					
			else
				return 0;
		}
		else
		{
			ceiling = GetCeiling(floor, x + xFront, y, z + zFront) - y;
			if (ceiling < CLICK(2))
			{
				if (height - ceiling <= LARA_HEIGHT)
				{
					if (height - ceiling < CLICK(2))
						return 0;
					*shift = height;
					return -2;
				}
				else
				{
					*shift = height;
					return -1;
				}
			}
			else
			{
				return 1;
			}
		}
	}

	return -2;
}

int LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll)
{
	short roomNumber = item->roomNumber;

	item->gravityStatus = false;
	item->fallspeed = 0;

	if (TrInput & IN_ACTION && item->hitPoints > 0 || item->animNumber == LA_ONWATER_TO_LADDER) // Can't let go on this anim
		return 0;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	Lara.headYrot = 0;
	Lara.headXrot = 0;

	SetAnimation(item, LA_FALL_START);

	item->speed = 2;
	item->gravityStatus = true;
	item->fallspeed = 1;

	Lara.gunStatus = LG_NO_ARMS;

	return 1;
}
