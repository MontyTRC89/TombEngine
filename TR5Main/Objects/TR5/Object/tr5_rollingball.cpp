#include "framework.h"
#include "tr5_rollingball.h"
#include "Game/collision/sphere.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Game/effects/effects.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"

constexpr auto MAX_ROLLINGBALL_SPEED = WALL_SIZE * 3;

void RollingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, l, coll->Setup.Radius))
	{
		if (TestCollision(item, l))
		{
			if (TriggerActive(item) && (item->itemFlags[0] || item->fallspeed))
			{
				LaraItem->animNumber = LA_BOULDER_DEATH;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = LS_DEATH;
				LaraItem->currentAnimState = LS_DEATH;
				LaraItem->gravityStatus = false;
			}
			else
			{
				ObjectCollision(itemNumber, l, coll);
			}
		}
	}
}

void RollingBallControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->fallspeed += GRAVITY;
	item->pos.xPos += item->itemFlags[0] / 32;
	item->pos.yPos += item->fallspeed;
	item->pos.zPos += item->itemFlags[1] / 32;

	int dh = GetCollisionResult(item).Position.Floor - CLICK(2);

	if (item->pos.yPos > dh)
	{
		if (abs(item->fallspeed) > 16)
		{
			int distance = sqrt(
				SQUARE(Camera.pos.x - item->pos.xPos) +
				SQUARE(Camera.pos.y - item->pos.yPos) +
				SQUARE(Camera.pos.z - item->pos.zPos));

			if (distance < 16384)
				Camera.bounce = -(((16384 - distance) * abs(item->fallspeed)) / 16384);
		}

		if (item->pos.yPos - dh < 512)
			item->pos.yPos = dh;

		if (item->fallspeed <= 64)
		{
			if (abs(item->speed) <= 512 || (GetRandomControl() & 0x1F))
				item->fallspeed = 0;
			else
				item->fallspeed = -(short)(GetRandomControl() % (item->speed / 8));
		}
		else
		{
			item->fallspeed = -(short)(item->fallspeed / 4);
		}
	}

	int frontX = item->pos.xPos;
	int frontZ = item->pos.zPos + CLICK(0.5f);
	int backX  = item->pos.xPos;
	int backZ  = item->pos.zPos - CLICK(0.5f);
	int rightX = item->pos.xPos + CLICK(0.5f);
	int rightZ = item->pos.zPos;
	int leftX  = item->pos.xPos - CLICK(0.5f);
	int leftZ  = item->pos.zPos;

	auto frontFloor = GetCollisionResult(frontX, item->pos.yPos, frontZ, item->roomNumber);
	auto backFloor  = GetCollisionResult(backX,  item->pos.yPos, backZ,  item->roomNumber);
	auto rightFloor = GetCollisionResult(rightX, item->pos.yPos, rightZ, item->roomNumber);
	auto leftFloor  = GetCollisionResult(leftX,  item->pos.yPos, leftZ,  item->roomNumber);

	int frontHeight = frontFloor.Position.Floor - CLICK(2);
	int backHeight  = backFloor.Position.Floor  - CLICK(2);
	int rightHeight = rightFloor.Position.Floor - CLICK(2);
	int leftHeight  = leftFloor.Position.Floor  - CLICK(2);

	int frontCeiling = frontFloor.Position.Ceiling + CLICK(2);
	int backCeiling  = backFloor.Position.Ceiling  + CLICK(2);
	int rightCeiling = rightFloor.Position.Ceiling + CLICK(2);
	int leftCeiling  = leftFloor.Position.Ceiling  + CLICK(2);

	frontX = item->pos.xPos;
	frontZ = item->pos.zPos + CLICK(2);
	backX  = item->pos.xPos;
	backZ  = item->pos.zPos - CLICK(2);
	rightX = item->pos.xPos + CLICK(2);
	rightZ = item->pos.zPos;
	leftX  = item->pos.xPos - CLICK(2);
	leftZ  = item->pos.zPos;

	auto fronFarFloor  = GetCollisionResult(frontX, item->pos.yPos, frontZ, item->roomNumber);
	auto backFarFloor  = GetCollisionResult(backX,  item->pos.yPos, backZ,  item->roomNumber);
	auto rightFarFloor = GetCollisionResult(rightX, item->pos.yPos, rightZ, item->roomNumber);
	auto leftFarFloor  = GetCollisionResult(leftX,  item->pos.yPos, leftZ,  item->roomNumber);

	int frontFarHeight = fronFarFloor.Position.Floor  - CLICK(2);
	int backFarHeight  = backFarFloor.Position.Floor  - CLICK(2);
	int rightFarHeight = rightFarFloor.Position.Floor - CLICK(2);
	int leftFarHeight  = leftFarFloor.Position.Floor  - CLICK(2);

	int frontFarCeiling = fronFarFloor.Position.Ceiling  + CLICK(2);
	int backFarCeiling  = backFarFloor.Position.Ceiling  + CLICK(2);
	int rightFarCeiling = rightFarFloor.Position.Ceiling + CLICK(2);
	int leftFarCeiling  = leftFarFloor.Position.Ceiling  + CLICK(2);

	if (item->pos.yPos - dh > -CLICK(1) ||
		item->pos.yPos - frontFarHeight >= CLICK(2) ||
		item->pos.yPos - rightFarHeight >= CLICK(2) ||
		item->pos.yPos - backFarHeight  >= CLICK(2) ||
		item->pos.yPos - leftFarHeight  >= CLICK(2))
	{
		int counterZ = 0;

		if (frontFarHeight - dh <= CLICK(1))
		{
			if (frontFarHeight - dh < -CLICK(4) || frontHeight - dh < -CLICK(1))
			{
				if (item->itemFlags[1] <= 0)
				{
					if (!item->itemFlags[1] && item->itemFlags[0])
					{
						item->pos.zPos = (item->pos.zPos & ~(CLICK(4) - 1)) | CLICK(2);
					}
				}
				else
				{
					item->itemFlags[1] = -item->itemFlags[1] / 2;
					item->pos.zPos = (item->pos.zPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (frontHeight == dh)
			{
				counterZ++;
			}
			else
			{
				item->itemFlags[1] += (frontHeight - dh) / 2;
			}
		}

		if (backHeight - dh <= CLICK(1))
		{
			if (backFarHeight - dh < -CLICK(4) || backHeight - dh < -CLICK(1))
			{
				if (item->itemFlags[1] >= 0)
				{
					if (!item->itemFlags[1] && item->itemFlags[0])
					{
						item->pos.zPos = (item->pos.zPos & ~(CLICK(4) - 1)) | CLICK(2);
					}
				}
				else
				{
					item->itemFlags[1] = -item->itemFlags[1] / 2;
					item->pos.zPos = (item->pos.zPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (backHeight == dh)
			{
				counterZ++;
			}
			else
			{
				item->itemFlags[1] -= (backHeight - dh) / 2;
			}
		}

		if (counterZ == 2)
		{
			if (abs(item->itemFlags[1]) <= 64)
				item->itemFlags[1] = 0;
			else
				item->itemFlags[1] -= (item->itemFlags[1] / 64);
		}

		int counterX = 0;

		if (leftHeight - dh <= CLICK(1))
		{
			if (leftFarHeight - dh < -CLICK(4) || leftHeight - dh < -CLICK(1))
			{
				if (item->itemFlags[0] >= 0)
				{
					if (!item->itemFlags[0] && item->itemFlags[1])
					{
						item->pos.xPos = (item->pos.xPos & ~(CLICK(4) - 1)) | CLICK(2);
					}
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] / 2;
					item->pos.xPos = (item->pos.xPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (leftHeight == dh)
			{
				counterX++;
			}
			else
			{
				item->itemFlags[0] -= (leftHeight - dh) / 2;
			}
		}

		if (rightHeight - dh <= CLICK(1))
		{
			if (rightFarHeight - dh < -CLICK(4) || rightHeight - dh < -CLICK(1))
			{
				if (item->itemFlags[0] <= 0)
				{
					if (!item->itemFlags[0] && item->itemFlags[1])
					{
						item->pos.xPos = (item->pos.xPos & ~(CLICK(4) - 1)) | CLICK(2);
					}
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] / 2;
					item->pos.xPos = (item->pos.xPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (rightHeight == dh)
			{
				counterX++;
			}
			else
			{
				item->itemFlags[0] += (rightHeight - dh) / 2;
			}
		}

		if (counterX == 2)
		{
			if (abs(item->itemFlags[0]) <= 64)
				item->itemFlags[0] = 0;
			else
				item->itemFlags[0] -= (item->itemFlags[0] / 64);
		}
	}

	auto roomNumber = GetCollisionResult(item).RoomNumber;

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->itemFlags[0] > MAX_ROLLINGBALL_SPEED)
		item->itemFlags[0] = MAX_ROLLINGBALL_SPEED;
	else if (item->itemFlags[0] < -MAX_ROLLINGBALL_SPEED)
		item->itemFlags[0] = -MAX_ROLLINGBALL_SPEED;

	if (item->itemFlags[1] > MAX_ROLLINGBALL_SPEED)
		item->itemFlags[1] = MAX_ROLLINGBALL_SPEED;
	else if (item->itemFlags[1] < -MAX_ROLLINGBALL_SPEED)
		item->itemFlags[1] = -MAX_ROLLINGBALL_SPEED;

	short angle = 0;

	if (item->itemFlags[1] || item->itemFlags[0])
		angle = phd_atan(item->itemFlags[1], item->itemFlags[0]);
	else
		angle = item->pos.yRot;

	if (item->pos.yRot != angle)
	{
		if (((angle - item->pos.yRot) & 0x7fff) >= 512)
		{
			if (angle <= item->pos.yRot || angle - item->pos.yRot >= 0x8000)
				item->pos.yRot -= 512;
			else
				item->pos.yRot += 512;
		}
		else
		{
			item->pos.yRot = angle;
		}
	}

	item->pos.xRot -= (abs(item->itemFlags[0]) + abs(item->itemFlags[1])) / 2;

	TestTriggers(item, true);
}

void ClassicRollingBallCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll)
{
	int x, y, z;
	short d;
	ITEM_INFO* item;

	item = &g_Level.Items[itemNum];

	if (item->status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, lara, coll->Setup.Radius))
			return;
		if (!TestCollision(item, lara))
			return;
		if (lara->gravityStatus)
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(item, lara, coll, coll->Setup.EnableSpaz, 1);
			lara->hitPoints -= 100;
			x = lara->pos.xPos - item->pos.xPos;
			y = (lara->pos.yPos - 350) - (item->pos.yPos - 512);
			z = lara->pos.zPos - item->pos.zPos;				
			d = (short)sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));
			if (d < 512)
				d = 512;
			x = item->pos.xPos + ((x * 512) / d);
			y = item->pos.yPos - 512 + ((y * 512) / d);
			z = item->pos.zPos + ((z * 512) / d);
			DoBloodSplat(x, y, z, item->speed, item->pos.yRot, item->roomNumber);
		}
		else
		{
			lara->hitStatus = 1;
			if (lara->hitPoints > 0)
			{
				lara->hitPoints = -1;//?
				lara->pos.yRot = item->pos.yRot;
				lara->pos.zPos = 0;
				lara->pos.zRot = 0;	

				lara->animNumber = LA_BOULDER_DEATH;
				lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
				lara->currentAnimState = LS_BOULDER_DEATH;
				lara->goalAnimState = LS_BOULDER_DEATH;
						
				Camera.flags = CF_FOLLOW_CENTER;
				Camera.targetAngle = ANGLE(170);
				Camera.targetElevation = -ANGLE(25);
				for (int i = 0; i < 15; i++)
				{
					x = lara->pos.xPos + (GetRandomControl() - ANGLE(180) / 256);
					y = lara->pos.yPos - (GetRandomControl() / 64);
					z = lara->pos.zPos + (GetRandomControl() - ANGLE(180) / 256);
					d = ((GetRandomControl() - ANGLE(180) / 8) + item->pos.yRot);
					DoBloodSplat(x, y, z, (short)(item->speed * 2), d, item->roomNumber);
				}
			}
		}
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(itemNum, lara, coll);

}

void ClassicRollingBallControl(short itemNum)
{
	short x, z, dist, oldx, oldz, roomNum;
	short y1, y2, ydist;
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	GAME_VECTOR* old;
	ROOM_INFO* r;

	item = &g_Level.Items[itemNum];
	if (item->status == ITEM_ACTIVE)
	{
		if (item->goalAnimState == 2)
		{
			AnimateItem(item);
			return;
		}

		if (item->pos.yPos < item->floor)
		{
			if (!item->gravityStatus)
			{
				item->gravityStatus = 1;
				item->fallspeed = -10;
			}
		}
		else if (item->currentAnimState == 0)
			item->goalAnimState = 1;

		oldx = item->pos.xPos;
		oldz = item->pos.zPos;
		AnimateItem(item);
		roomNum = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum);
		if (item->roomNumber != roomNum)
			ItemNewRoom(itemNum, item->roomNumber);

		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		TestTriggers(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNum, true);

		if (item->pos.yPos >= (int)floor - 256)
		{
			item->gravityStatus = false;
			item->fallspeed = 0;
			item->pos.yPos = item->floor;
			SoundEffect(SFX_TR3_ROLLING_BALL, &item->pos, 0);
			dist = sqrt((SQUARE(Camera.mikePos.x - item->pos.xPos)) + (SQUARE(Camera.mikePos.z - item->pos.zPos)));
			if (dist < 10240)
				Camera.bounce = -40 * (10240 - dist) / 10240;
		}

//		dist = (item->objectNumber == ID_CLASSIC_ROLLING_BALL) ? 384 : 1024;//huh?
		if (item->objectNumber == ID_CLASSIC_ROLLING_BALL)
		{
			dist = 320;
			ydist = 832;
		}
		else if (item->objectNumber == ID_BIG_ROLLING_BALL)
		{
			dist = 1088;
			ydist = 2112;
		}
		else
		{
			dist = 1024;
			ydist = 1024;
		}

		x = item->pos.xPos + dist * phd_sin(item->pos.yRot);
		z = item->pos.zPos + dist * phd_cos(item->pos.yRot);

		floor = GetFloor(x, item->pos.yPos, z, &roomNum);
		y1 = GetFloorHeight(floor, x, item->pos.yPos, z);

		roomNum = item->roomNumber;
		floor = GetFloor(x, item->pos.yPos - ydist, z, &roomNum);
		y2 = GetCeiling(floor, x, item->pos.yPos - ydist, z);

		if (y1 < item->pos.yPos || y2 > (item->pos.yPos-ydist)) //there's something wrong here, this if statement returns true, executing this block, deactivating the boulders.
		{
			/*stupid sound crap hardcoded to object # idk*/
			item->status = ITEM_DEACTIVATED;
			item->pos.yPos = item->floor;
			item->pos.xPos = oldx;
			item->pos.zPos = oldz;
			item->speed = 0;
			item->fallspeed = 0;
			item->touchBits = 0;
		}
	}
	else if (item->status == ITEM_DEACTIVATED)
	{
		if (!TriggerActive(item))
		{
			item->status = ITEM_NOT_ACTIVE;
			old = (GAME_VECTOR*)item->data;
			item->pos.xPos = old->x;
			item->pos.yPos = old->y;
			item->pos.zPos = old->z;
			if (item->roomNumber != old->roomNumber)
			{
				RemoveDrawnItem(itemNum);
				r = &g_Level.Rooms[old->roomNumber];
				item->nextItem = r->itemNumber;
				r->itemNumber = itemNum;
				item->roomNumber = old->roomNumber;
			}
			item->currentAnimState = 0;
			item->goalAnimState = 0;
			item->animNumber = Objects[item->objectNumber].animIndex;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = g_Level.Anims[item->animNumber].currentAnimState; 
			item->goalAnimState = g_Level.Anims[item->animNumber].currentAnimState;
			item->requiredAnimState = 0;
			RemoveActiveItem(itemNum);
		}
	}
}

void InitialiseClassicRollingBall(short itemNum)
{
	ITEM_INFO *item;
	GAME_VECTOR* old;

	item = &g_Level.Items[itemNum];
	item->data = GAME_VECTOR{ };
	old = item->data;
	old->x = item->pos.xPos;
	old->y = item->pos.yPos;
	old->z = item->pos.zPos;
	old->roomNumber = item->roomNumber;

}
