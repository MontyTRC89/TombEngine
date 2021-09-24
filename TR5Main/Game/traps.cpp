#include "framework.h"
#include "traps.h"
#include "items.h"
#include "effects/effects.h"
#include "effects/tomb4fx.h"
#include "effects/weather.h"
#include "lara.h"
#include "collide.h"
#include "sphere.h"
#include "camera.h"
#include "tr5_light.h"
#include "animation.h"
#include "level.h"
#include "input.h"
#include "room.h"
#include "Sound/sound.h"
#include "item.h"

using namespace TEN::Effects::Environment;

static short WreckingBallData[2] = {0, 0};
ITEM_INFO* WBItem;
short WBRoom;

void InitialiseFallingBlock(short itemNumber)
{
	g_Level.Items[itemNumber].meshBits = 1;
	TEN::Floordata::AddBridge(itemNumber);
}

void FallingBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (!item->itemFlags[0] && !item->triggerFlags && item->pos.yPos == l->pos.yPos)
	{
		if (!((item->pos.xPos ^ l->pos.xPos) & 0xFFFFFC00) && !((l->pos.zPos ^ item->pos.zPos) & 0xFFFFFC00))
		{
			SoundEffect(SFX_TR4_ROCK_FALL_CRUMBLE, &item->pos, 0);
			AddActiveItem(itemNum);

			item->itemFlags[0] = 0;
			item->status = ITEM_ACTIVE;
			item->flags |= 0x3E00;
		}
	}
}

void FallingBlockControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->triggerFlags)
	{
		item->triggerFlags--;
	}
	else
	{
		if (item->itemFlags[0])
		{
			if (item->itemFlags[0] < 60)
			{
				if (item->itemFlags[0] < 52)
				{
					if (!(GetRandomControl() % (62 - item->itemFlags[0])))
						item->pos.yPos += (GetRandomControl() & 3) + 1;
					item->itemFlags[0]++;
				}
				else
				{
					item->itemFlags[1] += 2;
					item->itemFlags[0]++;
					item->pos.yPos += item->itemFlags[1];
				}
			}
			else
			{
				KillItem(itemNumber);
			}
		}
		else
		{
			item->meshBits = -2;
			ExplodingDeath(itemNumber, -1, 15265);
			item->itemFlags[0]++;
		}
	}
}

//void FallingBlockFloor(ITEM_INFO* item, int x, int y, int z, int* height)
//{
//	if (!((x ^ item->pos.xPos) & 0xFFFFFC00) && !((z ^ item->pos.zPos) & 0xFFFFFC00))
//	{
//		if (y <= item->pos.yPos)
//		{
//			*height = item->pos.yPos;
//			HeightType = WALL;
//		}
//	}
//}
//
//void FallingBlockCeiling(ITEM_INFO* item, int x, int y, int z, int* height)
//{
//	if (!((x ^ item->pos.xPos) & 0xFFFFFC00) && !((z ^ item->pos.zPos) & 0xFFFFFC00))
//	{
//		if (y > item->pos.yPos)
//		{
//			*height = item->pos.yPos + 256;
//		}
//	}
//}

std::optional<int> FallingBlockFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (!item->meshBits || item->itemFlags[0] >= 52)
		return std::nullopt;

	int height = item->pos.yPos;
	return std::optional{ height };
}

std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits || item->itemFlags[0] >= 52)
		return std::nullopt;

	int height = item->pos.yPos + 256;
	return std::optional{ height };
}

int FallingBlockFloorBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return item->pos.yPos;
}

int FallingBlockCeilingBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return (item->pos.yPos + 256);
}

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
			SoundEffect(SFX_TR5_J_GRAB_MOTOR_B_LP, &item->pos, 0);
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
			SoundEffect(SFX_TR5_J_GRAB_MOTOR_B_LP, &item->pos, 0);
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
			SoundEffect(SFX_TR5_J_GRAB_MOTOR_A, &item->pos, 0);
		}
		if ((item->pos.xPos & 0x3FF) == 512 && (item->pos.zPos & 0x3FF) == 512)
			item->itemFlags[0] = 0;
		if (x == item->pos.xPos && z == item->pos.zPos && test)
		{
			if (item->itemFlags[1] != -1)
			{
				StopSoundEffect(SFX_TR5_J_GRAB_MOTOR_B_LP);
				SoundEffect(SFX_TR5_J_GRAB_MOTOR_C, &item->pos, 0);
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
			SoundEffect(SFX_TR5_J_GRAB_DROP, &item->pos, 0);
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
			StopSoundEffect(SFX_TR5_J_GRAB_WINCH_UP_LP);
			item->itemFlags[0] = 1;
			item->pos.yPos = item2->pos.yPos + 1644;
			if (item->fallspeed < -32)
			{
				SoundEffect(SFX_TR5_J_GRAB_IMPACT, &item->pos, 4104);
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
			SoundEffect(SFX_TR5_J_GRAB_WINCH_UP_LP, &item->pos, 0);
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
	WBItem = item;
	WBRoom = room;
}

