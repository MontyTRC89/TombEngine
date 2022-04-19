#include "framework.h"
#include "traps.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/Lara/lara.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/camera.h"
#include "tr5_light.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Game/room.h"
#include "Sound/sound.h"

using namespace TEN::Effects::Environment;

static short WreckingBallData[2] = {0, 0};

void InitialiseWreckingBall(short itemNumber)
{
	ITEM_INFO* item;
	short room;

	item = &g_Level.Items[itemNumber];
	item->itemFlags[3] = FindAllItems(ID_ANIMATING16)[0];
	room = item->roomNumber;
	item->pos.yPos = GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room), item->pos.xPos, item->pos.yPos, item->pos.zPos) + 1644;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room);
	if (room != item->roomNumber)
		ItemNewRoom(itemNumber, room);
}

void WreckingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;
	int x, y, z, test;
	short damage;

	item = &g_Level.Items[itemNumber];
	if (TestBoundsCollide(item, l, coll->Setup.Radius))
	{
		x = l->pos.xPos;
		y = l->pos.yPos;
		z = l->pos.zPos;
		test = (x & 1023) > 256 && (x & 1023) < 768 && (z & 1023) > 256 && (z & 1023) < 768;
		damage = item->fallspeed > 0 ? 96 : 0;
		if (ItemPushItem(item, l, coll, coll->Setup.EnableSpaz, 1))
		{
			if (test)
				l->hitPoints = 0;
			else
				l->hitPoints -= damage;
			x -= l->pos.xPos;
			y -= l->pos.yPos;
			z -= l->pos.zPos;
			if (damage)
			{
				for (int i = 14 + (GetRandomControl() & 3); i > 0; --i)
				{
					TriggerBlood(l->pos.xPos + (GetRandomControl() & 63) - 32, l->pos.yPos - (GetRandomControl() & 511) - 256,
						l->pos.zPos + (GetRandomControl() & 63) - 32, -1, 1);
				}
			}
			if (!coll->Setup.EnableObjectPush || test)
			{
				l->pos.xPos += x;
				l->pos.yPos += y;
				l->pos.zPos += z;
			}
		}
	}
}

void WreckingBallControl(short itemNumber)
{
	ITEM_INFO* item, *item2;
	int test, x, z, oldX, oldZ, wx, wz, flagX, flagZ, height, dx, dz, ceilingX, ceilingZ, adx, adz;
	short room;

	item = &g_Level.Items[itemNumber];
	test = 1;
	item2 = &g_Level.Items[item->itemFlags[3]];
	if (LaraItem->pos.xPos >= 45056 && LaraItem->pos.xPos <= 57344 && LaraItem->pos.zPos >= 26624 && LaraItem->pos.zPos <= 43008
		|| item->itemFlags[2] < 900)
	{
		if (item->itemFlags[2] < 900)
		{
			if (!item->itemFlags[2] || !(GlobalCounter & 0x3F))
			{
				WreckingBallData[0] = GetRandomControl() % 7 - 3;
				WreckingBallData[1] = GetRandomControl() % 7 - 3;
			}
			x = (WreckingBallData[0] << 10) + 51712;
			z = (WreckingBallData[1] << 10) + 34304;
			test = 0;
		}
		else
		{
			x = LaraItem->pos.xPos;
			z = LaraItem->pos.zPos;
		}
	}
	else
	{
		x = 51200;
		z = 33792;
		test = 0;
	}
	if (item->itemFlags[2] < 900)
		++item->itemFlags[2];

	if (item->itemFlags[1] <= 0)
	{
		oldX = item->pos.xPos;
		oldZ = item->pos.zPos;
		x = x & 0xFFFFFE00 | 0x200;
		z = z & 0xFFFFFE00 | 0x200;
		dx = x - item->pos.xPos;
		dz = z - item->pos.zPos;
		wx = 0;
		if (dx < 0)
			wx = -1024;
		else if (dx > 0)
			wx = 1024;
		wz = 0;
		if (dz < 0)
			wz = -1024;
		else if (dz > 0)
			wz = 1024;
		room = item->roomNumber;
		ceilingX = GetCeiling(GetFloor(item->pos.xPos + wx, item2->pos.yPos, item->pos.zPos, &room), item->pos.xPos + wx, item2->pos.yPos, item->pos.zPos);
		room = item->roomNumber;
		ceilingZ = GetCeiling(GetFloor(item->pos.xPos, item2->pos.yPos, item->pos.zPos + wz, &room), item->pos.xPos, item2->pos.yPos, item->pos.zPos + wz);
		if (ceilingX <= item2->pos.yPos && ceilingX != NO_HEIGHT)
			flagX = 1;
		else
			flagX = 0;
		if (ceilingZ <= item2->pos.yPos && ceilingZ != NO_HEIGHT)
			flagZ = 1;
		else
			flagZ = 0;
		if (!item->itemFlags[0])
		{
			if (flagX && dx && (abs(dx) > abs(dz) || !flagZ || GetRandomControl() & 1))
			{
				item->itemFlags[0] = 1;
			}
			else if (flagZ && dz)
			{
				item->itemFlags[0] = 2;
			}
		}
		if (item->itemFlags[0] == 1)
		{
			SoundEffect(1045, &item->pos, 0);
			adx = abs(dx);
			if (adx >= 32)
				adx = 32;
			if (dx > 0)
			{
				item->pos.xPos += adx;
			}
			else if (dx < 0)
			{
				item->pos.xPos -= adx;
			}
			else
			{
				item->itemFlags[0] = 0;
			}
		}
		if (item->itemFlags[0] == 2)
		{
			SoundEffect(1045, &item->pos, 0);
			adz = abs(dz);
			if (adz >= 32)
				adz = 32;
			if (dz > 0)
			{
				item->pos.zPos += adz;
			}
			else if (dz < 0)
			{
				item->pos.zPos -= adz;
			}
			else
			{
				item->itemFlags[0] = 0;
			}
		}
		if (item->itemFlags[1] == -1 && (oldX != item->pos.xPos || oldZ != item->pos.zPos))
		{
			item->itemFlags[1] = 0;
			SoundEffect(1044, &item->pos, 0);
		}
		if ((item->pos.xPos & 0x3FF) == 512 && (item->pos.zPos & 0x3FF) == 512)
			item->itemFlags[0] = 0;
		if (x == item->pos.xPos && z == item->pos.zPos && test)
		{
			if (item->itemFlags[1] != -1)
			{
				StopSoundEffect(1045);
				SoundEffect(1046, &item->pos, 0);
			}
			item->itemFlags[1] = 1;
			item->triggerFlags = 30;
		}
	}
	else if (item->itemFlags[1] == 1)
	{
		if (!item->triggerFlags)
		{
			--item->triggerFlags;
		}
		else if (!item->currentAnimState)
		{
			item->goalAnimState = 1;
		}
		else if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		{
			SoundEffect(1042, &item->pos, 0);
			++item->itemFlags[1];
			item->fallspeed = 6;
			item->pos.yPos += item->fallspeed;
		}
	}
	else if (item->itemFlags[1] == 2)
	{
		item->fallspeed += 24;
		item->pos.yPos += item->fallspeed;
		room = item->roomNumber;
		height = GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room), item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (height < item->pos.yPos)
		{
			item->pos.yPos = height;
			if (item->fallspeed > 48)
			{
				BounceCamera(item, 64, 8192);
				item->fallspeed = -item->fallspeed >> 3;
			}
			else
			{
				++item->itemFlags[1];
				item->fallspeed = 0;
			}
		}
		else if (height - item->pos.yPos < 1536 && item->currentAnimState)
		{
			item->goalAnimState = 0;
		}
	}
	else if (item->itemFlags[1] == 3)
	{
		item->fallspeed -= 3;
		item->pos.yPos += item->fallspeed;
		if (item->pos.yPos < item2->pos.yPos + 1644)
		{
			StopSoundEffect(1048);
			item->itemFlags[0] = 1;
			item->pos.yPos = item2->pos.yPos + 1644;
			if (item->fallspeed < -32)
			{
				SoundEffect(1047, &item->pos, 4104);
				item->fallspeed = -item->fallspeed >> 3;
				BounceCamera(item, 16, 8192);
			}
			else
			{
				item->itemFlags[1] = -1;
				item->fallspeed = 0;
				item->itemFlags[0] = 0;
			}
		}
		else if (!item->itemFlags[0])
		{
			SoundEffect(1048, &item->pos, 0);
		}
	}
	item2->pos.xPos = item->pos.xPos;
	item2->pos.zPos = item->pos.zPos;
	room = item->roomNumber;
	item2->pos.yPos = GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room), item->pos.xPos, item->pos.yPos, item->pos.zPos);
	GetFloor(item2->pos.xPos, item2->pos.yPos, item2->pos.zPos, &room);
	if (room != item2->roomNumber)
		ItemNewRoom(item->itemFlags[3], room);
	TriggerAlertLight(item2->pos.xPos, item2->pos.yPos + 64, item2->pos.zPos, 255, 64, 0, 64 * (GlobalCounter & 0x3F), item2->roomNumber, 24);
	TriggerAlertLight(item2->pos.xPos, item2->pos.yPos + 64, item2->pos.zPos, 255, 64, 0, 64 * (GlobalCounter - 32) & 0xFFF, item2->roomNumber, 24);
	room = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room);
	if (room != item->roomNumber)
		ItemNewRoom(itemNumber, room);
	AnimateItem(item);
}

