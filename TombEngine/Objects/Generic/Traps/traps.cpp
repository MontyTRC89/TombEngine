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
	ItemInfo* item;
	short room;

	item = &g_Level.Items[itemNumber];
	item->ItemFlags[3] = FindAllItems(ID_ANIMATING16)[0];
	room = item->RoomNumber;
	item->Pose.Position.y = GetCeiling(GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room), item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) + 1644;
	GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room);
	if (room != item->RoomNumber)
		ItemNewRoom(itemNumber, room);
}

void WreckingBallCollision(short itemNumber, ItemInfo* l, CollisionInfo* coll)
{
	ItemInfo* item;
	int x, y, z, test;
	short damage;

	item = &g_Level.Items[itemNumber];
	if (TestBoundsCollide(item, l, coll->Setup.Radius))
	{
		x = l->Pose.Position.x;
		y = l->Pose.Position.y;
		z = l->Pose.Position.z;
		test = (x & 1023) > 256 && (x & 1023) < 768 && (z & 1023) > 256 && (z & 1023) < 768;
		damage = item->Animation.VerticalVelocity > 0 ? 96 : 0;
		if (ItemPushItem(item, l, coll, coll->Setup.EnableSpasm, 1))
		{
			if (test)
				DoDamage(l, INT_MAX);
			else
				DoDamage(l, damage);

			x -= l->Pose.Position.x;
			y -= l->Pose.Position.y;
			z -= l->Pose.Position.z;
			if (damage)
			{
				for (int i = 14 + (GetRandomControl() & 3); i > 0; --i)
				{
					TriggerBlood(l->Pose.Position.x + (GetRandomControl() & 63) - 32, l->Pose.Position.y - (GetRandomControl() & 511) - 256,
						l->Pose.Position.z + (GetRandomControl() & 63) - 32, -1, 1);
				}
			}
			if (!coll->Setup.EnableObjectPush || test)
			{
				l->Pose.Position.x += x;
				l->Pose.Position.y += y;
				l->Pose.Position.z += z;
			}
		}
	}
}

void WreckingBallControl(short itemNumber)
{
	ItemInfo* item, *item2;
	int test, x, z, oldX, oldZ, wx, wz, flagX, flagZ, height, dx, dz, ceilingX, ceilingZ, adx, adz;
	short room;

	item = &g_Level.Items[itemNumber];
	test = 1;
	item2 = &g_Level.Items[item->ItemFlags[3]];
	if (LaraItem->Pose.Position.x >= 45056 && LaraItem->Pose.Position.x <= 57344 && LaraItem->Pose.Position.z >= 26624 && LaraItem->Pose.Position.z <= 43008
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
			x = LaraItem->Pose.Position.x;
			z = LaraItem->Pose.Position.z;
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
		oldX = item->Pose.Position.x;
		oldZ = item->Pose.Position.z;
		x = x & 0xFFFFFE00 | 0x200;
		z = z & 0xFFFFFE00 | 0x200;
		dx = x - item->Pose.Position.x;
		dz = z - item->Pose.Position.z;
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
		ceilingX = GetCeiling(GetFloor(item->Pose.Position.x + wx, item2->Pose.Position.y, item->Pose.Position.z, &room), item->Pose.Position.x + wx, item2->Pose.Position.y, item->Pose.Position.z);
		room = item->RoomNumber;
		ceilingZ = GetCeiling(GetFloor(item->Pose.Position.x, item2->Pose.Position.y, item->Pose.Position.z + wz, &room), item->Pose.Position.x, item2->Pose.Position.y, item->Pose.Position.z + wz);
		if (ceilingX <= item2->Pose.Position.y && ceilingX != NO_HEIGHT)
			flagX = 1;
		else
			flagX = 0;
		if (ceilingZ <= item2->Pose.Position.y && ceilingZ != NO_HEIGHT)
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
			SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_B_LOOP, &item->Pose);
			adx = abs(dx);
			if (adx >= 32)
				adx = 32;
			if (dx > 0)
			{
				item->Pose.Position.x += adx;
			}
			else if (dx < 0)
			{
				item->Pose.Position.x -= adx;
			}
			else
			{
				item->ItemFlags[0] = 0;
			}
		}
		if (item->ItemFlags[0] == 2)
		{
			SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_B_LOOP, &item->Pose);
			adz = abs(dz);
			if (adz >= 32)
				adz = 32;
			if (dz > 0)
			{
				item->Pose.Position.z += adz;
			}
			else if (dz < 0)
			{
				item->Pose.Position.z -= adz;
			}
			else
			{
				item->ItemFlags[0] = 0;
			}
		}
		if (item->ItemFlags[1] == -1 && (oldX != item->Pose.Position.x || oldZ != item->Pose.Position.z))
		{
			item->ItemFlags[1] = 0;
			SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_A, &item->Pose);
		}
		if ((item->Pose.Position.x & 0x3FF) == 512 && (item->Pose.Position.z & 0x3FF) == 512)
			item->ItemFlags[0] = 0;
		if (x == item->Pose.Position.x && z == item->Pose.Position.z && test)
		{
			if (item->ItemFlags[1] != -1)
			{
				StopSoundEffect(SFX_TR5_BASE_CLAW_MOTOR_B_LOOP);
				SoundEffect(SFX_TR5_BASE_CLAW_MOTOR_C, &item->Pose);
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
			SoundEffect(SFX_TR5_BASE_CLAW_DROP, &item->Pose);
			++item->ItemFlags[1];
			item->Animation.VerticalVelocity = 6;
			item->Pose.Position.y += item->Animation.VerticalVelocity;
		}
	}
	else if (item->ItemFlags[1] == 2)
	{
		item->Animation.VerticalVelocity += 24;
		item->Pose.Position.y += item->Animation.VerticalVelocity;
		room = item->RoomNumber;
		height = GetFloorHeight(GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room), item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		if (height < item->Pose.Position.y)
		{
			item->Pose.Position.y = height;
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
		else if (height - item->Pose.Position.y < 1536 && item->Animation.ActiveState)
		{
			item->Animation.TargetState = 0;
		}
	}
	else if (item->ItemFlags[1] == 3)
	{
		item->Animation.VerticalVelocity -= 3;
		item->Pose.Position.y += item->Animation.VerticalVelocity;
		if (item->Pose.Position.y < item2->Pose.Position.y + 1644)
		{
			StopSoundEffect(SFX_TR5_BASE_CLAW_WINCH_UP_LOOP);
			item->ItemFlags[0] = 1;
			item->Pose.Position.y = item2->Pose.Position.y + 1644;
			if (item->Animation.VerticalVelocity < -32)
			{
				SoundEffect(SFX_TR5_BASE_CLAW_TOP_IMPACT, &item->Pose, SoundEnvironment::Land, 1.0f, 0.5f);
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
			SoundEffect(SFX_TR5_BASE_CLAW_WINCH_UP_LOOP, &item->Pose);
		}
	}
	item2->Pose.Position.x = item->Pose.Position.x;
	item2->Pose.Position.z = item->Pose.Position.z;
	room = item->RoomNumber;
	item2->Pose.Position.y = GetCeiling(GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room), item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
	GetFloor(item2->Pose.Position.x, item2->Pose.Position.y, item2->Pose.Position.z, &room);
	if (room != item2->RoomNumber)
		ItemNewRoom(item->ItemFlags[3], room);
	TriggerAlertLight(item2->Pose.Position.x, item2->Pose.Position.y + 64, item2->Pose.Position.z, 255, 64, 0, 64 * (GlobalCounter & 0x3F), item2->RoomNumber, 24);
	TriggerAlertLight(item2->Pose.Position.x, item2->Pose.Position.y + 64, item2->Pose.Position.z, 255, 64, 0, 64 * (GlobalCounter - 32) & 0xFFF, item2->RoomNumber, 24);
	room = item->RoomNumber;
	GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room);
	if (room != item->RoomNumber)
		ItemNewRoom(itemNumber, room);
	AnimateItem(item);
}

