#include "framework.h"
#include "tr5_rollingball.h"
#include "sphere.h"
#include "camera.h"
#include "control.h"
#include "lara.h"
#include "setup.h"

void RollingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, l, coll->radius))
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
	item->pos.xPos += item->itemFlags[0] >> 5;
	item->pos.yPos += item->fallspeed;
	item->pos.zPos += item->itemFlags[1] >> 5;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int dh = height - 512;

	if (item->pos.yPos > height - 512)
	{
		if (abs(item->fallspeed) > 16)
		{
			int distance = sqrt(
				SQUARE(Camera.pos.x - item->pos.xPos) +
				SQUARE(Camera.pos.y - item->pos.yPos) +
				SQUARE(Camera.pos.z - item->pos.zPos));

			if (distance < 16384)
				Camera.bounce = -((16384 - distance) * abs(item->fallspeed) >> 14);
		}

		if (item->pos.yPos - dh < 512)
			item->pos.yPos = dh;

		if (item->fallspeed <= 64)
		{
			if (abs(item->speed) <= 512 || GetRandomControl() & 0x1F)
				item->fallspeed = 0;
			else
				item->fallspeed = -(short)(GetRandomControl() % (item->speed >> 3));
		}
		else
		{
			item->fallspeed = -(short)(item->fallspeed >> 2);
		}
	}

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	floor = GetFloor(x, y, z + 128, &roomNumber);
	int y1a = GetFloorHeight(floor, x, y, z + 128) - 512;

	floor = GetFloor(x, y, z - 128, &roomNumber);
	int y2a = GetFloorHeight(floor, x, y, z - 128) - 512;

	floor = GetFloor(x + 128, y, z, &roomNumber);
	int y3a = GetFloorHeight(floor, x + 128, y, z) - 512;

	floor = GetFloor(x - 128, y, z, &roomNumber);
	int y4a = GetFloorHeight(floor, x - 128, y, z) - 512;

	floor = GetFloor(x, y, z + 512, &roomNumber);
	int y1b = GetFloorHeight(floor, x, y, z + 512) - 512;

	floor = GetFloor(x, y, z - 512, &roomNumber);
	int y2b = GetFloorHeight(floor, x, y, z - 512) - 512;

	floor = GetFloor(x + 512, y, z, &roomNumber);
	int y3b = GetFloorHeight(floor, x + 512, y, z) - 512;

	floor = GetFloor(x - 512, y, z, &roomNumber);
	int y4b = GetFloorHeight(floor, x - 512, y, z) - 512;

	if (item->pos.yPos - dh  > -256
	||  item->pos.yPos - y1b >= 512
	||  item->pos.yPos - y3b >= 512
	||  item->pos.yPos - y2b >= 512
	||  item->pos.yPos - y4b >= 512)
	{
		int counterZ = 0;

		if (y1a - dh <= 256)
		{
			if (y1b - dh < -1024 || y1a - dh < -256)
			{
				if (item->itemFlags[1] <= 0)
				{
					if (!item->itemFlags[1] && item->itemFlags[0])
					{
						item->pos.zPos = (item->pos.zPos & -512) + 512;
					}
				}
				else
				{
					item->itemFlags[1] = -item->itemFlags[1] >> 1;
					item->pos.zPos = (item->pos.zPos & -512) + 512;
				}
			}
			else if (y1a == dh)
			{
				counterZ = 1;
			}
			else
			{
				item->itemFlags[1] += (y1a - dh) >> 1;
			}
		}

		if (y2a - dh <= 256)
		{
			if (y2b - dh < -1024 || y2a - dh < -256)
			{
				if (item->itemFlags[1] >= 0)
				{
					if (!item->itemFlags[1] && item->itemFlags[0])
					{
						item->pos.zPos = (item->pos.zPos & -512) + 512;
					}
				}
				else
				{
					item->itemFlags[1] = -item->itemFlags[1] >> 1;
					item->pos.zPos = (item->pos.zPos & -512) + 512;
				}
			}
			else if (y2a == dh)
			{
				counterZ++;
			}
			else
			{
				item->itemFlags[1] -= (y2a - dh) >> 1;
			}
		}

		if (counterZ == 2)
		{
			if (abs(item->itemFlags[1]) <= 64)
				item->itemFlags[1] = 0;
			else
				item->itemFlags[1] = item->itemFlags[1] - (item->itemFlags[1] >> 6);
		}

		int counterX = 0;

		if (y4a - dh <= 256)
		{
			if (y4b - dh < -1024 || y4a - dh < -256)
			{
				if (item->itemFlags[0] >= 0)
				{
					if (!item->itemFlags[0] && item->itemFlags[1])
					{
						item->pos.xPos = (item->pos.xPos & -512) + 512;
					}
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] >> 1;
					item->pos.xPos = (item->pos.xPos & -512) + 512;
				}
			}
			else if (y4a == dh)
			{
				counterX = 1;
			}
			else
			{
				item->itemFlags[0] -= (y4a - dh) >> 1;
			}
		}

		if (y3a - dh <= 256)
		{
			if (y3b - dh < -1024 || y3a - dh < -256)
			{
				if (item->itemFlags[0] <= 0)
				{
					if (!item->itemFlags[0] && item->itemFlags[1])
					{
						item->pos.xPos = (item->pos.xPos & -512) + 512;
					}
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] >> 1;
					item->pos.xPos = (item->pos.xPos & -512) + 512;
				}
			}
			else if (y3a == dh)
			{
				counterX++;
			}
			else
			{
				item->itemFlags[0] += (y3a - dh) >> 1;
			}
		}

		if (counterX == 2)
		{
			if (abs(item->itemFlags[0]) <= 64)
				item->itemFlags[0] = 0;
			else
				item->itemFlags[0] = item->itemFlags[0] - (item->itemFlags[0] >> 6);
		}
	}

	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->itemFlags[0] <= 3072)
	{
		if (item->itemFlags[0] < -3072)
			item->itemFlags[0] = -3072;
	}
	else
	{
		item->itemFlags[0] = 3072;
	}

	if (item->itemFlags[1] <= 3072)
	{
		if (item->itemFlags[1] < -3072)
			item->itemFlags[1] = -3072;
	}
	else
	{
		item->itemFlags[1] = 3072;
	}

	short angle = 0;

	if (item->itemFlags[1] || item->itemFlags[0])
		angle = phd_atan(item->itemFlags[1], item->itemFlags[0]);
	else
		angle = item->pos.yRot;

	if (item->pos.yRot != angle)
	{
		if (((angle - item->pos.yRot) & 32767) >= 512)
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

	item->pos.xRot -= (abs(item->itemFlags[0]) + abs(item->itemFlags[1])) >> 1;

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	TestTriggers(TriggerIndex, TRUE, NULL);
}

void ClassicRollingBallCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll)
{
	int x, y, z;
	short d;
	ITEM_INFO* item;

	item = &g_Level.Items[itemNum];

	if (item->status == ITEM_ACTIVE)
	{
		if (TestBoundsCollide(item, lara, coll->radius))
		{
			if (TestCollision(item, lara))
			{
				if (lara->gravityStatus)
				{
					if (coll->enableBaddiePush)
						ItemPushLara(item, lara, coll, coll->enableSpaz, 1);
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
						/*
						Lara's animation stuff
						*/
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
		}
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(itemNum, lara, coll);

}

void ClassicRollingBallControl(short itemNum)
{
	short x, z, dist, oldx, oldz, roomNum;
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	GAME_VECTOR* old;
	ROOM_INFO* r;

	item = &g_Level.Items[itemNum];
	if (item->status == ITEM_ACTIVE)
	{
		if (LaraItem->goalAnimState == 2)
		{
			AnimateItem(item);
			return;
		}

		if (item->pos.yPos < item->floor)
		{
			if (item->gravityStatus == 0)
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

		TestTriggers(TriggerIndex, TRUE, NULL);

		if (item->pos.yPos >= (int)floor - 256)
		{
			item->gravityStatus = false;
			item->fallspeed = 0;
			item->pos.yPos = item->floor;
			/*stupid sound crap hardcoded to object # idk*/
			dist = sqrt((SQUARE(Camera.mikePos.x - item->pos.xPos)) + (SQUARE(Camera.mikePos.z - item->pos.zPos)));
			if (dist < 10240)
				Camera.bounce = -40 * (10240 - dist) / 10240;
		}

		dist = (item->objectNumber == ID_CLASSIC_ROLLING_BALL) ? 384 : 1024;//huh?
		x = item->pos.xPos + (dist * phd_sin(item->pos.yRot) >> W2V_SHIFT);
		z = item->pos.zPos + (dist * phd_cos(item->pos.yRot) >> W2V_SHIFT);
		floor = GetFloor(x, item->pos.yPos, z, &roomNum);
		if (GetFloorHeight(floor, x, item->pos.yPos, z) < item->pos.yPos)
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
	old = (GAME_VECTOR *)malloc(sizeof(GAME_VECTOR));
	item->data = (GAME_VECTOR *)malloc(sizeof(GAME_VECTOR));

	old->x = item->pos.xPos;
	old->y = item->pos.yPos;
	old->z = item->pos.zPos;
	old->roomNumber = item->roomNumber;

}
