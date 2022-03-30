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
	item->ItemFlags[3] = FindAllItems(ID_ANIMATING16)[0];
	room = item->RoomNumber;
	item->Position.yPos = GetCeiling(GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &room), item->Position.xPos, item->Position.yPos, item->Position.zPos) + 1644;
	GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &room);
	if (room != item->RoomNumber)
		ItemNewRoom(itemNumber, room);
}

void WreckingBallCollision(short itemNumber, ITEM_INFO* l, CollisionInfo* coll)
{
	ITEM_INFO* item;
	int x, y, z, test;
	short damage;

	item = &g_Level.Items[itemNumber];
	if (TestBoundsCollide(item, l, coll->Setup.Radius))
	{
		x = l->Position.xPos;
		y = l->Position.yPos;
		z = l->Position.zPos;
		test = (x & 1023) > 256 && (x & 1023) < 768 && (z & 1023) > 256 && (z & 1023) < 768;
		damage = item->Animation.VerticalVelocity > 0 ? 96 : 0;
		if (ItemPushItem(item, l, coll, coll->Setup.EnableSpasm, 1))
		{
			if (test)
				l->HitPoints = 0;
			else
				l->HitPoints -= damage;
			x -= l->Position.xPos;
			y -= l->Position.yPos;
			z -= l->Position.zPos;
			if (damage)
			{
				for (int i = 14 + (GetRandomControl() & 3); i > 0; --i)
				{
					TriggerBlood(l->Position.xPos + (GetRandomControl() & 63) - 32, l->Position.yPos - (GetRandomControl() & 511) - 256,
						l->Position.zPos + (GetRandomControl() & 63) - 32, -1, 1);
				}
			}
			if (!coll->Setup.EnableObjectPush || test)
			{
				l->Position.xPos += x;
				l->Position.yPos += y;
				l->Position.zPos += z;
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
	item2 = &g_Level.Items[item->ItemFlags[3]];
	if (LaraItem->Position.xPos >= 45056 && LaraItem->Position.xPos <= 57344 && LaraItem->Position.zPos >= 26624 && LaraItem->Position.zPos <= 43008
		|| item->ItemFlags[2] < 900)
	{
		if (item->ItemFlags[2] < 900)
		{
			if (!item->ItemFlags[2] || !(GlobalCounter & 0x3F))
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
			x = LaraItem->Position.xPos;
			z = LaraItem->Position.zPos;
		}
	}
	else
	{
		x = 51200;
		z = 33792;
		test = 0;
	}
	if (item->ItemFlags[2] < 900)
		++item->ItemFlags[2];

	if (item->ItemFlags[1] <= 0)
	{
		oldX = item->Position.xPos;
		oldZ = item->Position.zPos;
		x = x & 0xFFFFFE00 | 0x200;
		z = z & 0xFFFFFE00 | 0x200;
		dx = x - item->Position.xPos;
		dz = z - item->Position.zPos;
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
		room = item->RoomNumber;
		ceilingX = GetCeiling(GetFloor(item->Position.xPos + wx, item2->Position.yPos, item->Position.zPos, &room), item->Position.xPos + wx, item2->Position.yPos, item->Position.zPos);
		room = item->RoomNumber;
		ceilingZ = GetCeiling(GetFloor(item->Position.xPos, item2->Position.yPos, item->Position.zPos + wz, &room), item->Position.xPos, item2->Position.yPos, item->Position.zPos + wz);
		if (ceilingX <= item2->Position.yPos && ceilingX != NO_HEIGHT)
			flagX = 1;
		else
			flagX = 0;
		if (ceilingZ <= item2->Position.yPos && ceilingZ != NO_HEIGHT)
			flagZ = 1;
		else
			flagZ = 0;
		if (!item->ItemFlags[0])
		{
			if (flagX && dx && (abs(dx) > abs(dz) || !flagZ || GetRandomControl() & 1))
			{
				item->ItemFlags[0] = 1;
			}
			else if (flagZ && dz)
			{
				item->ItemFlags[0] = 2;
			}
		}
		if (item->ItemFlags[0] == 1)
		{
			SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_B_LOOP, &item->Position, 0);
			adx = abs(dx);
			if (adx >= 32)
				adx = 32;
			if (dx > 0)
			{
				item->Position.xPos += adx;
			}
			else if (dx < 0)
			{
				item->Position.xPos -= adx;
			}
			else
			{
				item->ItemFlags[0] = 0;
			}
		}
		if (item->ItemFlags[0] == 2)
		{
			SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_B_LOOP, &item->Position, 0);
			adz = abs(dz);
			if (adz >= 32)
				adz = 32;
			if (dz > 0)
			{
				item->Position.zPos += adz;
			}
			else if (dz < 0)
			{
				item->Position.zPos -= adz;
			}
			else
			{
				item->ItemFlags[0] = 0;
			}
		}
		if (item->ItemFlags[1] == -1 && (oldX != item->Position.xPos || oldZ != item->Position.zPos))
		{
			item->ItemFlags[1] = 0;
			SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_A, &item->Position, 0);
		}
		if ((item->Position.xPos & 0x3FF) == 512 && (item->Position.zPos & 0x3FF) == 512)
			item->ItemFlags[0] = 0;
		if (x == item->Position.xPos && z == item->Position.zPos && test)
		{
			if (item->ItemFlags[1] != -1)
			{
				StopSoundEffect(SFX_TR5_BASE_CLAW_MOTOR_B_LOOP);
				SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_C, &item->Position, 0);
			}
			item->ItemFlags[1] = 1;
			item->TriggerFlags = 30;
		}
	}
	else if (item->ItemFlags[1] == 1)
	{
		if (!item->TriggerFlags)
		{
			--item->TriggerFlags;
		}
		else if (!item->Animation.ActiveState)
		{
			item->Animation.TargetState = 1;
		}
		else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
		{
			SoundEffect(SFX_TR5_BASE_CLAW_DROP, &item->Position, 0);
			++item->ItemFlags[1];
			item->Animation.VerticalVelocity = 6;
			item->Position.yPos += item->Animation.VerticalVelocity;
		}
	}
	else if (item->ItemFlags[1] == 2)
	{
		item->Animation.VerticalVelocity += 24;
		item->Position.yPos += item->Animation.VerticalVelocity;
		room = item->RoomNumber;
		height = GetFloorHeight(GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &room), item->Position.xPos, item->Position.yPos, item->Position.zPos);
		if (height < item->Position.yPos)
		{
			item->Position.yPos = height;
			if (item->Animation.VerticalVelocity > 48)
			{
				BounceCamera(item, 64, 8192);
				item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity >> 3;
			}
			else
			{
				++item->ItemFlags[1];
				item->Animation.VerticalVelocity = 0;
			}
		}
		else if (height - item->Position.yPos < 1536 && item->Animation.ActiveState)
		{
			item->Animation.TargetState = 0;
		}
	}
	else if (item->ItemFlags[1] == 3)
	{
		item->Animation.VerticalVelocity -= 3;
		item->Position.yPos += item->Animation.VerticalVelocity;
		if (item->Position.yPos < item2->Position.yPos + 1644)
		{
			StopSoundEffect(SFX_TR5_BASE_CLAW_WINCH_LOOP);
			item->ItemFlags[0] = 1;
			item->Position.yPos = item2->Position.yPos + 1644;
			if (item->Animation.VerticalVelocity < -32)
			{
				SoundEffect(SFX_TR5_BASE_CLAW_TOP_IMPACT, &item->Position, 4104);
				item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity >> 3;
				BounceCamera(item, 16, 8192);
			}
			else
			{
				item->ItemFlags[1] = -1;
				item->Animation.VerticalVelocity = 0;
				item->ItemFlags[0] = 0;
			}
		}
		else if (!item->ItemFlags[0])
		{
			SoundEffect(SFX_TR5_BASE_CLAW_WINCH_LOOP, &item->Position, 0);
		}
	}
	item2->Position.xPos = item->Position.xPos;
	item2->Position.zPos = item->Position.zPos;
	room = item->RoomNumber;
	item2->Position.yPos = GetCeiling(GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &room), item->Position.xPos, item->Position.yPos, item->Position.zPos);
	GetFloor(item2->Position.xPos, item2->Position.yPos, item2->Position.zPos, &room);
	if (room != item2->RoomNumber)
		ItemNewRoom(item->ItemFlags[3], room);
	TriggerAlertLight(item2->Position.xPos, item2->Position.yPos + 64, item2->Position.zPos, 255, 64, 0, 64 * (GlobalCounter & 0x3F), item2->RoomNumber, 24);
	TriggerAlertLight(item2->Position.xPos, item2->Position.yPos + 64, item2->Position.zPos, 255, 64, 0, 64 * (GlobalCounter - 32) & 0xFFF, item2->RoomNumber, 24);
	room = item->RoomNumber;
	GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &room);
	if (room != item->RoomNumber)
		ItemNewRoom(itemNumber, room);
	AnimateItem(item);
}

