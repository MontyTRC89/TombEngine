#include "framework.h"
#include "laraclmb.h"
#include "Lara.h"
#include "control.h"
#include "draw.h"
#include "sphere.h"
#include "laramisc.h"
#include "camera.h"
#include "level.h"
#include "input.h"

short LeftIntRightExtTab[4] = // offset 0xA0B7C
{
	0x0800, 0x0100, 0x0200, 0x0400
};
short LeftExtRightIntTab[4] = // offset 0xA0B84
{
	0x0200, 0x0400, 0x0800, 0x0100
};

short GetClimbTrigger(int x, int y, int z, short roomNumber) // (F) (D)
{
	GetFloorHeight(GetFloor(x, y, z, &roomNumber), x, y, z);

	short* data = TriggerIndex;

	if (data == NULL)
		return 0;

	if ((*data & DATA_TYPE) == LAVA_TYPE)
	{
		if (*data & END_BIT)
			return 0;

		data++;
	}

	return (*data & DATA_TYPE) == CLIMB_TYPE ? *data : 0;
}

void lara_col_climbend(ITEM_INFO* item, COLL_INFO* coll)//46E30(<), 47294(<) (F)
{
	return;
}

void lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll)//46DF8(<), 4725C(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = -ANGLE(45);
}

void lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll)//46BD0, 47034 (F)
{
	if (LaraCheckForLetGo(item, coll) 
		|| item->animNumber != ANIMATION_LARA_LADDER_DOWN)
		return;

	int frame = item->frameNumber - g_Level.Anims[ANIMATION_LARA_LADDER_DOWN].frameBase;
	int xShift;
	int yShift;

	switch (frame)
	{
	case 0:
		yShift = 0;
		break;

	case 28:
	case 29:
		yShift = 256;
		break;

	case 57:
		yShift = 512;
		break;

	default:
		return;
	}

	item->pos.yPos += yShift + 256;

	int shiftLeft = 0;
	int shiftRight = 0;
	int resultRight = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shiftRight);
	int resultLeft = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -512, 512, &shiftLeft);

	item->pos.yPos -= 256;

	if (resultRight != 0 && resultLeft != 0 &&
		resultRight != -2 && resultLeft != -2 &&
		TrInput & IN_BACK)
	{
		if (shiftRight && shiftLeft)
		{
			if (shiftRight < 0 != shiftLeft < 0)
			{
				item->goalAnimState = STATE_LARA_LADDER_IDLE;
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
			item->animNumber = ANIMATION_LARA_LADDER_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = STATE_LARA_LADDER_IDLE;
			item->goalAnimState = STATE_LARA_HANG;

			AnimateLara(item);
		}
		else
		{
			item->goalAnimState = STATE_LARA_LADDER_DOWN;
			item->pos.yPos -= yShift;
		}
		return;
	}

	item->goalAnimState = STATE_LARA_LADDER_IDLE;

	if (yShift != 0)
		AnimateLara(item);
}

void lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll)//46BA4(<), 47008(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetElevation = -ANGLE(45);
}

void lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll)//469B0, 46E14 (F)
{
	if (!LaraCheckForLetGo(item, coll)
		&& item->animNumber == ANIMATION_LARA_LADDER_UP)
	{
		int frame = item->frameNumber - g_Level.Anims[ANIMATION_LARA_LADDER_UP].frameBase;
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
			yShift = -256;
		}
		else if (frame == 57)
		{
			yShift = -512;
		}
		else
		{
			return;
		}

		item->pos.yPos += yShift - 256;

		resultRight = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &shiftRight, &ledgeRight);
		resultLeft = LaraTestClimbUpPos(item, coll->radius, -(coll->radius + 120), &shiftLeft, &ledgeLeft);

		item->pos.yPos += 256;
		 
		if (resultRight && resultLeft && TrInput & IN_FORWARD)
		{
			if (resultRight < 0 || resultLeft < 0)
			{
				item->goalAnimState = STATE_LARA_LADDER_IDLE;

				AnimateLara(item);

				if (abs(ledgeRight - ledgeLeft) <= 120)
				{
					if (resultRight != -1 || resultLeft != -1)
					{
						item->goalAnimState = STATE_LARA_UNKNOWN_138;
						item->requiredAnimState = STATE_LARA_CROUCH_IDLE;
					}
					else
					{
						item->goalAnimState = STATE_LARA_GRABBING;
						item->pos.yPos += (ledgeRight + ledgeLeft) / 2 - 256;
					}
				}
			}
			else
			{
				item->goalAnimState = STATE_LARA_LADDER_UP;
				item->pos.yPos -= yShift;
			}
		}
		else
		{
			item->goalAnimState = STATE_LARA_LADDER_IDLE;

			if (yShift != 0)
				AnimateLara(item);
		}
	}
}

void lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll)//46984(<), 46DE8(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetElevation = ANGLE(30);
}

void lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll)//46908(<), 46D6C(<) (F)
{
	if (!LaraCheckForLetGo(item, coll))
	{
		int shift = 0;
		Lara.moveAngle = item->pos.yRot + ANGLE(90);
		LaraDoClimbLeftRight(item, coll, LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shift), shift);
	}
}

void lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll)//468B8(<), 46D1C(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetAngle = ANGLE(30);
	Camera.targetElevation = -ANGLE(15);

	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = STATE_LARA_LADDER_IDLE;
}

void lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll)//46834(<), 46C98(<) (F)
{
	if (!LaraCheckForLetGo(item, coll))
	{
		int shift = 0;
		Lara.moveAngle = item->pos.yRot - ANGLE(90);
		LaraDoClimbLeftRight(item, coll, LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -512, 512, &shift), shift);
	}
}

void lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll)//467E4(<), 46C48(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetAngle = -ANGLE(30);
	Camera.targetElevation = -ANGLE(15);

	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = STATE_LARA_LADDER_IDLE;
}

void lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	int yShift;
	int resultRight, resultLeft;
	int shiftRight, shiftLeft;
	int ledgeRight, ledgeLeft;

	if (LaraCheckForLetGo(item, coll) 
		|| item->animNumber != ANIMATION_LARA_LADDER_IDLE)
		return;

	if (!(TrInput & IN_FORWARD))
	{
		if (!(TrInput & IN_BACK))
			return;

		if (item->goalAnimState == STATE_LARA_HANG)
			return;

		item->goalAnimState = STATE_LARA_LADDER_IDLE;
		item->pos.yPos += 256;
		
		resultRight = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &ledgeRight);
		resultLeft = LaraTestClimbPos(item, coll->radius, -120 - coll->radius, -512, 512, &ledgeLeft);
		
		item->pos.yPos -= 256;
		
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
			item->goalAnimState = STATE_LARA_LADDER_DOWN;
			item->pos.yPos += yShift;
		}
		else
		{
			item->goalAnimState = STATE_LARA_HANG;
		}
	}
	else if (item->goalAnimState != STATE_LARA_GRABBING)
	{
		item->goalAnimState = STATE_LARA_LADDER_IDLE;
		resultRight = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &shiftRight, &ledgeRight);
		resultLeft = LaraTestClimbUpPos(item, coll->radius, -120 - coll->radius, &shiftLeft, &ledgeLeft);

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

			item->goalAnimState = STATE_LARA_LADDER_UP;
			item->pos.yPos += yShift;
		}
		else if (abs(ledgeLeft - ledgeRight) <= 120)
		{
			if (resultRight == -1 && resultLeft == -1)
			{
				item->goalAnimState = STATE_LARA_GRABBING;
				item->pos.yPos += (ledgeRight + ledgeLeft) / 2 - 256;
			}
			else
			{
				item->goalAnimState = STATE_LARA_UNKNOWN_138;
				item->requiredAnimState = STATE_LARA_CROUCH_IDLE;
			}
		}
	}
}

void lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll)//463F0, 46854 (F)
{
	Lara.isClimbing = true;

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	Camera.targetElevation = -ANGLE(20);

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
	{
		item->goalAnimState = STATE_LARA_LADDER_LEFT;
		Lara.moveAngle = item->pos.yRot - ANGLE(90);
	}
	else if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
	{
		item->goalAnimState = STATE_LARA_LADDER_RIGHT;
		Lara.moveAngle = item->pos.yRot + ANGLE(90);
	}
	else if (TrInput & IN_JUMP)
	{
		if (item->animNumber == ANIMATION_LARA_LADDER_IDLE)
		{
			item->goalAnimState = STATE_LARA_JUMP_BACK;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.moveAngle = item->pos.yRot - ANGLE(180);
		}
	}
}

int LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift) // (F) (D)
{
	short angle = (unsigned short) (item->pos.yRot + ANGLE(45)) >> W2V_SHIFT;
	int x;
	int z;
	int xfront = 0;
	int zfront = 0;

	switch (angle)
	{
	case NORTH:
		x = item->pos.xPos + right;
		z = item->pos.zPos + front;
		zfront = 256;
		break;

	case EAST:
		x = item->pos.xPos + front;
		z = item->pos.zPos - right;
		xfront = 256;
		break;

	case SOUTH:
		x = item->pos.xPos - right;
		z = item->pos.zPos - front;
		zfront = -256;
		break;

	case WEST:
	default:
		x = item->pos.xPos - front;
		z = item->pos.zPos + right;
		xfront = -256;
		break;
	}

	return LaraTestClimb(x, item->pos.yPos + origin, z, xfront, zfront, height, item->roomNumber, shift);
}

void LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int result, int shift)//46100, 46564 (F)
{
	if (result == 1)
	{
		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = STATE_LARA_LADDER_LEFT;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = STATE_LARA_LADDER_RIGHT;
		}
		else
		{
			item->goalAnimState = STATE_LARA_LADDER_IDLE;
		}

		item->pos.yPos += shift;

		return;
	}

	if (result != 0)
	{
		item->goalAnimState = STATE_LARA_HANG;

		do
		{
			AnimateItem(item);
		} while (item->currentAnimState != STATE_LARA_HANG);

		item->pos.xPos = coll->old.x;
		item->pos.zPos = coll->old.z;

		return;
	}

	item->pos.xPos = coll->old.x;
	item->pos.zPos = coll->old.z;

	item->goalAnimState = STATE_LARA_LADDER_IDLE;
	item->currentAnimState = STATE_LARA_LADDER_IDLE;

	if (coll->oldAnimState != STATE_LARA_LADDER_IDLE)
	{
		item->animNumber = ANIMATION_LARA_LADDER_IDLE;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		int flag = LaraClimbLeftCornerTest(item, coll);

		if (flag)
		{
			if (flag <= 0)
			{
				item->animNumber = ANIMATION_LARA_LADDER_AROUND_LEFT_INNER_BEGIN;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = STATE_LARA_CLIMB_CORNER_LEFT_INNER;
				item->currentAnimState = STATE_LARA_CLIMB_CORNER_LEFT_INNER;
			}
			else
			{
				item->animNumber = ANIMATION_LARA_LADDER_AROUND_LEFT_OUTER_BEGIN;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = STATE_LARA_CLIMB_CORNER_LEFT_OUTER;
				item->currentAnimState = STATE_LARA_CLIMB_CORNER_LEFT_OUTER;
			}

			return;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		int flag = LaraClimbRightCornerTest(item, coll);

		if (flag)
		{
			if (flag <= 0)
			{
				item->animNumber = ANIMATION_LARA_LADDER_AROUND_RIGHT_INNER_BEGIN;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = STATE_LARA_CLIMB_CORNER_RIGHT_INNER;
				item->currentAnimState = STATE_LARA_CLIMB_CORNER_RIGHT_INNER;
			}
			else
			{
				item->animNumber = ANIMATION_LARA_LADDER_AROUND_RIGHT_OUTER_BEGIN;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = STATE_LARA_CLIMB_CORNER_RIGHT_OUTER;
				item->currentAnimState = STATE_LARA_CLIMB_CORNER_RIGHT_OUTER;
			}

			return;
		}
	}

	item->animNumber = coll->oldAnimNumber;
	item->frameNumber = coll->oldFrameNumber;

	AnimateLara(item);
}

int LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll)//45DE4, 46248
{
	int result = 0;

	if (item->animNumber != ANIMATION_LARA_LADDER_RIGHT)
		return 0;

	int oldYrot = item->pos.yRot;
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;

	short angle = (unsigned short) (item->pos.yRot + ANGLE(45)) / ANGLE(90);
	int x, z;

	if (angle && angle != SOUTH)
	{
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos % 1024) + 1024;
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos % 1024) + 1024;
	}
	else
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
	}

	int shift = 0;

	if (GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber) & LeftExtRightIntTab[angle])
	{
		item->pos.xPos = x;
		Lara.cornerX = x;
		item->pos.zPos = z;
		Lara.cornerZ = z;
		item->pos.yRot += ANGLE(90);
		Lara.moveAngle = item->pos.yRot;

		result = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shift);
	}

	if (!result)
	{
		item->pos.xPos = oldX;
		Lara.moveAngle = oldYrot;
		item->pos.yRot = oldYrot;
		item->pos.zPos = oldZ;

		int newX, newZ;

		switch (angle)
		{
		case NORTH:
			newX = ((item->pos.xPos + 1024) & 0xFFFFFC00) - (item->pos.zPos % 1024) + 1024;
			newZ = ((item->pos.zPos + 1024) & 0xFFFFFC00) - (item->pos.xPos % 1024) + 1024;
			break;

		case SOUTH:
			newX = ((item->pos.xPos - 1024) & 0xFFFFFC00) - (item->pos.zPos % 1024) + 1024;
			newZ = ((item->pos.zPos - 1024) & 0xFFFFFC00) - (item->pos.xPos % 1024) + 1024;
			break;

		case EAST:
			newX = ((item->pos.zPos ^ item->pos.xPos) % 1024) ^ (item->pos.xPos + 1024);
			newZ = (item->pos.zPos ^ ((item->pos.zPos ^ item->pos.xPos) % 1024)) - 1024;
			break;

		case WEST:
		default:
			newX = (item->pos.xPos ^ (item->pos.zPos ^ item->pos.xPos) % 1024) - 1024;
			newZ = ((item->pos.zPos ^ item->pos.xPos) % 1024) ^ (item->pos.zPos + 1024);
			break;

		}

		if (GetClimbTrigger(newX, item->pos.yPos, newZ, item->roomNumber) & LeftIntRightExtTab[angle])
		{
			item->pos.xPos = newX;
			Lara.cornerX = newX;
			item->pos.zPos = newZ;
			Lara.cornerZ = newZ;
			item->pos.yRot -= ANGLE(90);
			Lara.moveAngle = item->pos.yRot;
			result = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shift) != 0;
		}
	}
	else
	{
		result = -1;
	}

	item->pos.xPos = oldX;
	item->pos.yRot = oldYrot;
	item->pos.zPos = oldZ;
	Lara.moveAngle = oldYrot;

	return result;
}

int LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll)//45ABC, 45F20
{
	int result = 0;

	if (item->animNumber != ANIMATION_LARA_LADDER_LEFT)
		return 0;

	int oldYrot = item->pos.yRot;
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;

	short angle = (unsigned short) (item->pos.yRot + ANGLE(45)) >> W2V_SHIFT;
	int x, z;

	if (angle && angle != SOUTH)
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
	}
	else
	{
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + 1024;
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + 1024;
	}

	int shift = 0;

	if (GetClimbTrigger(x, item->pos.yPos, z, item->roomNumber) & LeftIntRightExtTab[angle])
	{
		item->pos.xPos = x;
		Lara.cornerX = x;
		item->pos.zPos = z;
		Lara.cornerZ = z;
		item->pos.yRot -= ANGLE(90);
		Lara.moveAngle = item->pos.yRot;

		result = LaraTestClimbPos(item, coll->radius, -coll->radius - 120, -512, 512, &shift);
		item->itemFlags[3] = result;
	}

	if (!result)
	{
		item->pos.xPos = oldX;
		Lara.moveAngle = oldYrot;
		item->pos.yRot = oldYrot;
		item->pos.zPos = oldZ;

		int newX, newZ;

		switch (angle)
		{
		case NORTH:
			newX = (item->pos.xPos ^ ((item->pos.zPos ^ item->pos.xPos) & 0x3FF)) - 1024;
			newZ = ((item->pos.zPos ^ item->pos.xPos) & 0x3FF) ^ (item->pos.zPos + 1024);
			break;

		case SOUTH:
			newX = ((item->pos.zPos ^ item->pos.xPos) & 0x3FF) ^ (item->pos.xPos + 1024);
			newZ = ((item->pos.zPos ^ item->pos.xPos) & 0x3FF) ^ (item->pos.zPos - 1024);
			break;

		case EAST:
			newX = ((item->pos.xPos + 1024) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + 1024;
			newZ = ((item->pos.zPos + 1024) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + 1024;
			break;

		case WEST:
		default:
			newX = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF);
			newZ = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF);
			break;

		}

		if (GetClimbTrigger(newX, item->pos.yPos, newZ, item->roomNumber) & LeftExtRightIntTab[angle])
		{
			item->pos.xPos = newX;
			Lara.cornerX = newX;
			item->pos.zPos = newZ;
			Lara.cornerZ = newZ;
			item->pos.yRot += ANGLE(90);
			Lara.moveAngle = item->pos.yRot;
			item->itemFlags[3] = LaraTestClimbPos(item, coll->radius, -coll->radius - 120, -512, 512, &shift);
			result = item->itemFlags[3] != 0;
		}
	}
	else
	{
		result = -1;
	}

	item->pos.xPos = oldX;
	item->pos.yRot = oldYrot;
	item->pos.zPos = oldZ;
	Lara.moveAngle = oldYrot;

	return result;
}

int LaraTestClimb(int x, int y, int z, int xFront, int zFront, int itemHeight, int itemRoom, int* shift)//457F0, 45C54
{

	*shift = 0;
	int hang = 1;
	if (!Lara.climbStatus)
		return 0;

	short roomNumber = itemRoom;
	FLOOR_INFO* floor = GetFloor(x, y - 128, z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);
	if (height == NO_HEIGHT)
		return 0;

	height -= (128 + y + itemHeight);
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
		if (ceiling - y >= 512)
			return 1;
		if (ceiling - y <= 442)
			return -(hang != 0);
		if (*shift > 0)
			return -(hang != 0);
		*shift = ceiling - y - 512;

		return 1;
	}
	
	ceiling = GetCeiling(floor, dx, y, dz) - y;
	if (ceiling >= 512)
		return 1;
	if (ceiling > 442)
	{
		if (*shift > 0)
			return -(hang != 0);
		*shift = ceiling - 512;

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

int LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge)//45530, 45994
{
	int y = item->pos.yPos - 768;
	short angle = (unsigned short) (item->pos.yRot + ANGLE(45)) / ANGLE(90);

	int x, z;
	int xFront = 0;
	int zFront = 0;

	switch(angle)
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
	int ceiling = 256 - y + GetCeiling(floor, x, y, z);
	
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
		
		if (height <= 128)
		{
			if (height > 0 && height > *shift)
				*shift = height;

			roomNumber = item->roomNumber;
			GetFloor(x, y + 512, z, &roomNumber);
			floor = GetFloor(x + xFront, y + 512, z + zFront, &roomNumber);
			ceiling = GetCeiling(floor, x + xFront, y + 512, z + zFront) - y;
			if (ceiling <= height)
				return 1;						
			if (ceiling >= 512)
				return 1;					
			else
				return 0;
		}
		else
		{
			ceiling = GetCeiling(floor, x + xFront, y, z + zFront) - y;
			if (ceiling < 512)
			{
				if (height - ceiling <= 762)
				{
					if (height - ceiling < 512)
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

int LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll)//45434, 45898 (F)
{
	short roomNumber = item->roomNumber;

	item->gravityStatus = false;
	item->fallspeed = 0;

	GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber),
		item->pos.xPos, item->pos.yPos, item->pos.zPos);

	coll->trigger = TriggerIndex;

	if (TrInput & IN_ACTION && item->hitPoints > 0)
		return 0;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	Lara.headYrot = 0;
	Lara.headXrot = 0;

	item->goalAnimState = STATE_LARA_JUMP_FORWARD;
	item->currentAnimState = STATE_LARA_JUMP_FORWARD;
	item->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

	item->speed = 2;
	item->gravityStatus = true;
	item->fallspeed = 1;

	Lara.gunStatus = LG_NO_ARMS;

	return 1;
}
