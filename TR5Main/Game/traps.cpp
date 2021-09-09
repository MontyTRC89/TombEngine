#include "framework.h"
#include "traps.h"

#include "items.h"
#include "effects\effects.h"
#include "effects\tomb4fx.h"
#include "lara.h"
#include "collide.h"
#include "sphere.h"
#include "camera.h"
#include "tr5_light.h"
#include "draw.h"
#include "level.h"
#include "input.h"
#include "Sound\sound.h"
#include "kayak.h"

static short WreckingBallData[2] = {0, 0};
ITEM_INFO* WBItem;
short WBRoom;

byte Flame3xzoffs[16][2] =
{
	{ 0x09, 0x09 },
	{ 0x18, 0x09 },
	{ 0x28, 0x09 },
	{ 0x37, 0x09 },
	{ 0x09, 0x18 },
	{ 0x18, 0x18 },
	{ 0x28, 0x18 },
	{ 0x37, 0x18 },
	{ 0x09, 0x28 },
	{ 0x18, 0x28 },
	{ 0x28, 0x28 },
	{ 0x37, 0x28 },
	{ 0x09, 0x37 },
	{ 0x18, 0x37 },
	{ 0x28, 0x37 },
	{ 0x37, 0x37 }
};

OBJECT_COLLISION_BOUNDS FireBounds = {0, 0, 0, 0, 0, 0, -1820, 1820, -5460, 5460, -1820, 1820};

bool FlameEmitterFlags[8];

void LaraBurn()
{
	if (!Lara.burn && !Lara.burnSmoke)
	{
		short fxNum = CreateNewEffect(LaraItem->roomNumber);
		if (fxNum != NO_ITEM)
		{
			EffectList[fxNum].objectNumber = ID_FLAME;
			Lara.burn = true;
		}
	}
}

void FlameEmitterControl(short itemNumber)
{
	byte r, g, b;
	int falloff;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (TriggerActive(item))
	{
		if (item->triggerFlags < 0)
		{
			short flags = -item->triggerFlags;
			if ((flags & 7) == 2 || (flags & 7) == 7)
			{
				PHD_3DPOS* pos = &item->pos;
				SoundEffect(SFX_TR4_FLAME_EMITTER, &item->pos, 0);
				TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
				TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
					(GetRandomControl() & 3) + 20,
					(GetRandomControl() & 0x3F) + 192,
					(GetRandomControl() & 0x1F) + 96, 0);
			}
			else
			{
				if (item->itemFlags[0])
				{
					if (item->itemFlags[1])
						item->itemFlags[1] = item->itemFlags[1] - (item->itemFlags[1] >> 2);

					if (item->itemFlags[2] < 256)
						item->itemFlags[2] += 8;

					item->itemFlags[0]--;
					if (!item->itemFlags[0])
						item->itemFlags[3] = (GetRandomControl() & 0x3F) + 150;
				}
				else
				{
					if (!--item->itemFlags[3])
					{
						if (flags >> 3)
							item->itemFlags[0] = (GetRandomControl() & 0x1F) + 30 * (flags >> 3);
						else
							item->itemFlags[0] = (GetRandomControl() & 0x3F) + 60;
					}

					if (item->itemFlags[2])
						item->itemFlags[2] -= 8;

					if (item->itemFlags[1] > -8192)
						item->itemFlags[1] -= 512;
				}

				if (item->itemFlags[2])
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, SP_NORMALFIRE, item->roomNumber, item->itemFlags[2]);

				if (item->itemFlags[1])
				{
					SoundEffect(SFX_TR4_FLAME_EMITTER, &item->pos, 0);

					if (item->itemFlags[1] <= -8192)
						TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
					else
						TriggerSuperJetFlame(item, item->itemFlags[1], GlobalCounter & 1);

					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
						(-item->itemFlags[1] >> 10) - (GetRandomControl() & 1) + 16,
						(GetRandomControl() & 0x3F) + 192,
						(GetRandomControl() & 0x1F) + 96, 0);
				}
				else
				{
					r = (GetRandomControl() & 0x3F) + 192;
					g = (GetRandomControl() & 0x1F) + 96;
					falloff = 10 - (GetRandomControl() & 1);
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
						10 - (GetRandomControl() & 1),
						(GetRandomControl() & 0x3F) + 192,
						(GetRandomControl() & 0x1F) + 96, 0);
				}
			}

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		}
		else
		{
			if (item->triggerFlags < 8)
				FlameEmitterFlags[item->triggerFlags] = true;

			AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, item->roomNumber, 0);

			TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
				16 - (GetRandomControl() & 1),
				(GetRandomControl() & 0x3F) + 192,
				(GetRandomControl() & 0x1F) + 96, 0);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);

			if (!Lara.burn
				/*&& item->triggerFlags != 33*/
				&& ItemNearLara(&item->pos, 600)
				&& (SQUARE(LaraItem->pos.xPos - item->pos.xPos) +
					SQUARE(LaraItem->pos.zPos - item->pos.zPos) < SQUARE(512))
				&& Lara.waterStatus != LW_FLYCHEAT)
			{
				LaraBurn();
			}
		}
	}
	else
	{
		if (item->triggerFlags > 0 && item->triggerFlags < 8)
			FlameEmitterFlags[item->triggerFlags] = false;
	}
}

void FlameEmitter2Control(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags >= 0)
		{
			if (item->triggerFlags != 2)
			{
				if (item->triggerFlags == 123)
				{
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1, item->roomNumber, item->itemFlags[3]);
				}
				else
				{
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1 - item->triggerFlags, item->roomNumber, item->itemFlags[3]);
				}
			}
			
			if (!item->triggerFlags || item->triggerFlags == 2)
			{
				if (item->itemFlags[3])
				{
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
						10,
						((GetRandomControl() & 0x3F) + 192) * item->itemFlags[3] >> 8,
						(GetRandomControl() & 0x1F) + 96 * item->itemFlags[3] >> 8,
						0);
				}
				else
				{
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos,
						10,
						(GetRandomControl() & 0x3F) + 192,
						(GetRandomControl() & 0x1F) + 96,
						0);
				}
			}

			if (item->triggerFlags == 2)
			{
				item->pos.xPos += phd_sin(item->pos.yRot - ANGLE(180));
				item->pos.zPos += phd_cos(item->pos.yRot - ANGLE(180));

				short roomNumber = item->roomNumber;
				FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				
				if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
				{
					FlashFadeR = 255;
					FlashFadeG = 128;
					FlashFadeB = 0;
					FlashFader = 32;
					KillItem(itemNumber);
					return;
				}

				if (item->roomNumber != roomNumber)
				{
					ItemNewRoom(itemNumber, roomNumber);
				}

				item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);;
				
				if (Wibble & 7)
				{
					TriggerFireFlame(item->pos.xPos, item->pos.yPos - 32, item->pos.zPos, -1, 1);
				}
			}
			
			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		}
		else if (!item->itemFlags[0])
		{
			DoFlipMap(-item->triggerFlags);
			FlipMap[-item->triggerFlags] ^= 0x3E00u;
			item->itemFlags[0] = 1;
		}
	}
}

void FlameControl(short fxNumber)
{
	FX_INFO* fx = &EffectList[fxNumber];

	for (int i = 0; i < 14; i++)
	{
		if (!(Wibble & 0x2))
		{
			fx->pos.xPos = 0;
			fx->pos.yPos = 0;
			fx->pos.zPos = 0;

			GetLaraJointPosition((PHD_VECTOR*)&fx->pos, i);

			if (Lara.burnCount)
			{
				Lara.burnCount--;
				if (!Lara.burnCount)
					Lara.burnSmoke = true;
			}

			TriggerFireFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, i, 255 - Lara.burnSmoke);
		}
	}

	byte r = (GetRandomControl() & 0x3F) + 192;
	byte g = (GetRandomControl() & 0x1F) + 96;
	byte b;

	if (!Lara.burnSmoke)
	{
		PHD_VECTOR pos{ 0,0,0 };
		GetLaraJointPosition(&pos, LM_HIPS);
		if (!Lara.burnBlue)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, 13, r, g, 0);
		}
		else
		{
			if (Lara.burnBlue == 128)
			{
				b = r;
				
				TriggerDynamicLight(pos.x, pos.y, pos.z, 13, 0, g, b);
			}
			else if (Lara.burnBlue == 256)
			{
				b = g;
				g = r;
				TriggerDynamicLight(pos.x, pos.y, pos.z, 13, 0, g, b);
			}
		}
	}

	if (LaraItem->roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, LaraItem->roomNumber);
	
	int wh = GetWaterHeight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);
	if (wh == NO_HEIGHT || fx->pos.yPos <= wh || Lara.burnBlue)
	{
		SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &fx->pos, 0);

		LaraItem->hitPoints -= 7;
		LaraItem->hitStatus = true;
	}
	else
	{
		KillEffect(fxNumber);
		Lara.burn = false;
	}

	if (Lara.waterStatus == LW_FLYCHEAT)
	{
		KillEffect(fxNumber);
		Lara.burn = false;
	}
}

void LavaBurn(ITEM_INFO* item)
{
	if (item->hitPoints >= 0 && Lara.waterStatus != LW_FLYCHEAT)
	{
		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, 32000, item->pos.zPos, &roomNumber);
		if (item->floor == GetFloorHeight(floor, item->pos.xPos, 32000, item->pos.zPos))
		{
//			if (Objects[ID_KAYAK].loaded && Objects[ID_KAYAK_LARA_ANIMS].loaded)		//TEMPORARILY ADDING THIS HACK FOR TESTING-// KayakLaraRapidsDrown works fine.
//				KayakLaraRapidsDrown();
//			else
//			{
				item->hitPoints = -1;
				item->hitStatus = true;
				LaraBurn();
//			}
		}
	}
}

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
//			OnFloor = 1;
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
	item->itemFlags[3] = FindItem(ID_ANIMATING16)[0];
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
	if (TestBoundsCollide(item, l, coll->Settings.Radius))
	{
		x = l->pos.xPos;
		y = l->pos.yPos;
		z = l->pos.zPos;
		test = (x & 1023) > 256 && (x & 1023) < 768 && (z & 1023) > 256 && (z & 1023) < 768;
		damage = item->fallspeed > 0 ? 96 : 0;
		if (ItemPushItem(item, l, coll, coll->Settings.EnableSpaz, 1))
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
			if (!coll->Settings.EnableObjectPush || test)
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

void FlameEmitterCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	if (Lara.gunType != WEAPON_TORCH
		|| Lara.gunStatus != LG_READY
		|| Lara.leftArm.lock 
		|| Lara.litTorch == (item->status & 1)
		|| item->timer == -1
		|| !(TrInput & IN_ACTION)
		|| l->currentAnimState != LS_STOP
		|| l->animNumber != LA_STAND_IDLE
		|| l->gravityStatus)
	{
		if (item->objectNumber == ID_BURNING_ROOTS)
			ObjectCollision(itemNumber, l, coll);
	}
	else
	{
		switch (item->objectNumber)
		{
		case ID_FLAME_EMITTER:
			FireBounds.boundingBox.X1 = -256;
			FireBounds.boundingBox.X2 = 256;
			FireBounds.boundingBox.Y1 = 0;
			FireBounds.boundingBox.Y2 = 1024;
			FireBounds.boundingBox.Z1 = -800;
			FireBounds.boundingBox.Z2 = 800;
			break;

		case ID_FLAME_EMITTER2:
			FireBounds.boundingBox.X1 = -256;
			FireBounds.boundingBox.X2 = 256;
			FireBounds.boundingBox.Y1 = 0;
			FireBounds.boundingBox.Y2 = 1024;
			FireBounds.boundingBox.Z1 = -600;
			FireBounds.boundingBox.Z2 = 600;
			break;

		case ID_BURNING_ROOTS:
			FireBounds.boundingBox.X1 = -384;
			FireBounds.boundingBox.X2 = 384;
			FireBounds.boundingBox.Y1 = 0;
			FireBounds.boundingBox.Y2 = 2048;
			FireBounds.boundingBox.Z1 = -384;
			FireBounds.boundingBox.Z2 = 384;
			break;

		}

		short oldYrot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;

		if (TestLaraPosition(&FireBounds, item, l))
		{
			if (item->objectNumber == ID_BURNING_ROOTS)
			{
				l->animNumber = LA_TORCH_LIGHT_5;
			}
			else
			{
				int dy = abs(l->pos.yPos - item->pos.yPos);
				l->itemFlags[3] = 1;
				l->animNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
			}
			
			l->currentAnimState = LS_MISC_CONTROL;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			Lara.flareControlLeft = false;
			Lara.leftArm.lock = 3;
			Lara.interactedItem = itemNumber;
		}
		
		item->pos.yRot = oldYrot;
	}

	if (Lara.interactedItem == itemNumber 
		&& item->status != ITEM_ACTIVE 
		&& l->currentAnimState == LS_MISC_CONTROL)
	{
		if (l->animNumber >= LA_TORCH_LIGHT_1 && l->animNumber <= LA_TORCH_LIGHT_5)
		{
			if (l->frameNumber - g_Level.Anims[l->animNumber].frameBase == 40)
			{
				TestTriggers(item, true, item->flags & IFLAG_ACTIVATION_MASK);
				
				item->flags |= 0x3E00;
				item->itemFlags[3] = 0;
				item->status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);
			}
		}
	}
}

void InitialiseFlameEmitter2(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	item->pos.yPos -= 64;
	
	if (item->triggerFlags != 123)
	{
		if (item->pos.yRot > 0)
		{
			if (item->pos.yRot == ANGLE(90))
			{
				if (item->triggerFlags == 2)
					item->pos.xPos += 80;
				else
					item->pos.xPos += 256;
			}
		}
		else if (item->pos.yRot < 0)
		{
			if (item->pos.yRot == -ANGLE(180))
			{
				if (item->triggerFlags == 2)
					item->pos.zPos -= 80;
				else
					item->pos.zPos -= 256;
			}
			else if (item->pos.yRot == -ANGLE(90))
			{
				if (item->triggerFlags == 2)
					item->pos.xPos -= 80;
				else
					item->pos.xPos -= 256;
			}
		}
		else
		{
			if (item->triggerFlags == 2)
				item->pos.zPos += 80;
			else
				item->pos.zPos += 256;
		}
	}
}

void InitialiseFlameEmitter(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	/*if (item->triggerFlags > 0)
	{
		if (item->triggerFlags == 33)
		{
			if (item->pos.yRot > 0)
			{
				if (item->pos.yRot == ANGLE(90))
					item->pos.xPos += 144;
			}
			else
			{
				if (item->pos.yRot == 0)
				{
					item->pos.zPos += 144;
					item->pos.yPos += 32;
					return;
				}
				else if (item->pos.yRot == -ANGLE(180))
				{
					item->pos.zPos -= 144;
					item->pos.yPos += 32;
					return;
				}
				else if (item->pos.yRot == -ANGLE(90))
				{
					item->pos.xPos -= 144;
					item->pos.yPos += 32;
					return;
				}
			}

			item->pos.yPos += 32;
			return;
		}
	}
	else
	*/
	if (item->triggerFlags < 0)
	{
		item->itemFlags[0] = (GetRandomControl() & 0x3F) + 90;
		item->itemFlags[2] = 256;
		
		if (((-item->triggerFlags) & 7) == 7)
		{
			if (item->pos.yRot > 0)
			{
				if (item->pos.yRot == ANGLE(90))
				{
					item->pos.xPos += 512;
				}
			}
			else if (item->pos.yRot < 0)
			{
				if (item->pos.yRot == -ANGLE(180))
				{
					item->pos.zPos -= 512;
				}
				else if (item->pos.yRot == -ANGLE(90))
				{
					item->pos.xPos -= 512;
				}
			}
			else
			{
				item->pos.zPos += 512;
			}
		}
	}
}

void FlameEmitter3Control(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			SoundEffect(SFX_TR4_ELEC_ARCING_LOOP, &item->pos, 0);

			byte g = (GetRandomControl() & 0x3F) + 192;
			byte b = (GetRandomControl() & 0x3F) + 192;

			PHD_VECTOR src;
			PHD_VECTOR dest;

			src.x = item->pos.xPos;
			src.y = item->pos.yPos;
			src.z = item->pos.zPos;

			if (!(GlobalCounter & 3))
			{
				if (item->triggerFlags == 2 || item->triggerFlags == 4)
				{
					dest.x = item->pos.xPos + 512 * phd_sin(item->pos.yRot + ANGLE(180));
					dest.y = item->pos.yPos;
					dest.z = item->pos.zPos + 512 * phd_cos(item->pos.yRot + ANGLE(180));
					
					if (GetRandomControl() & 3)
					{
						TriggerLightning(&src, &dest, (GetRandomControl() & 0x1F) + 64, 0, g, b, 24, 0, 32, 3);
					}
					else
					{
						TriggerLightning(&src, &dest, (GetRandomControl() & 0x1F) + 96, 0, g, b, 32, 1, 32, 3);
					}
				}
			}

			if (item->triggerFlags >= 3 && !(GlobalCounter & 1))
			{
				short targetItemNumber = item->itemFlags[((GlobalCounter >> 2) & 1) + 2];
				ITEM_INFO* targetItem = &g_Level.Items[targetItemNumber];

				dest.x = 0;
				dest.y = -64;
				dest.z = 20;
				GetJointAbsPosition(targetItem, &dest, 0);

				if (!(GlobalCounter & 3))
				{
					if (GetRandomControl() & 3)
					{
						//TriggerEnergyArc(&dest.x_rot, &dest, (GetRandomControl() & 0x1F) + 64, b | ((g | 0x180000) << 8), 0, 32, 5);
					}
					else
					{
						//TriggerEnergyArc(&dest.x_rot, &dest, (GetRandomControl() & 0x1F) + 96, b | ((g | 0x200000) << 8), 1, 32, 5);
					}
				}
				if (item->triggerFlags != 3 || targetItem->triggerFlags)
					TriggerLightningGlow(dest.x, dest.y, dest.z, 64, 0, g, b);
			}

			if ((GlobalCounter & 3) == 2)
			{
				src.x = item->pos.xPos;
				src.y = item->pos.yPos;
				src.z = item->pos.zPos;
				
				dest.x = (GetRandomControl() & 0x1FF) + src.x - 256;
				dest.y = (GetRandomControl() & 0x1FF) + src.y - 256;
				dest.z = (GetRandomControl() & 0x1FF) + src.z - 256;

				//TriggerEnergyArc(&src, &dest, (GetRandomControl() & 0xF) + 16, b | ((g | 0x180000) << 8), 3, 32, 3);
				TriggerLightningGlow(dest.x, dest.y, dest.z, 64, 0, g, b);  
			}
		}
		else
		{
			// Small fires

			if (item->itemFlags[0])
			{
				item->itemFlags[0]--;
			}
			else
			{
				item->itemFlags[0] = (GetRandomControl() & 3) + 8;
				int flags = GetRandomControl() & 0x3F;
				if (item->itemFlags[1] == flags)
					flags = (flags + 13) & 0x3F;
				item->itemFlags[1] = flags;
			}

			int x, z, i;

			if (!(Wibble & 4))
			{
				i = 2 * (item->itemFlags[1] & 7);
				x = 16 * (Flame3xzoffs[i][0] - 32);
				z = 16 * (Flame3xzoffs[i][1] - 32);
				TriggerFireFlame(x + item->pos.xPos, item->pos.yPos, z + item->pos.zPos, -1, 2);
			}
			else
			{
				i = 2 * (item->itemFlags[1] >> 3);
				x = 16 * (Flame3xzoffs[i + 8][0] - 32);
				z = 16 * (Flame3xzoffs[i + 8][1] - 32);
				TriggerFireFlame(x + item->pos.xPos, item->pos.yPos, z + item->pos.zPos, -1, 2);
			}

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);

			TriggerDynamicLight(x, item->pos.yPos, z, 12, (GetRandomControl() & 0x3F) + 192, ((GetRandomControl() >> 4) & 0x1F) + 96, 0);
			
			PHD_3DPOS pos;
			pos.xPos = item->pos.xPos;
			pos.yPos = item->pos.yPos;
			pos.zPos = item->pos.zPos;

			if (ItemNearLara(&pos, 600))
			{
				if ((!Lara.burn) && Lara.waterStatus != LW_FLYCHEAT)
				{
					LaraItem->hitPoints -= 5;
					LaraItem->hitStatus = true;

					int dx = LaraItem->pos.xPos - item->pos.xPos;
					int dz = LaraItem->pos.zPos - item->pos.zPos;
			
					if (SQUARE(dx) + SQUARE(dz) < 202500)
						LaraBurn();
				}
			}
		}
	}
}