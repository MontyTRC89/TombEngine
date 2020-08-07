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