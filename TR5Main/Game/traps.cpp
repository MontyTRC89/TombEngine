#include "framework.h"
#include "traps.h"

#include "items.h"
#include "effect2.h"
#include "tomb4fx.h"
#include "effect.h"
#include "lara.h"
#include "collide.h"
#include "switch.h"
#include "sphere.h"
#include "camera.h"
#include "objlight.h"
#include "draw.h"
#include "level.h"
#include "input.h"
#include "sound.h"

static short CeilingTrapDoorBounds[12] = {-256, 256, 0, 900, -768, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR CeilingTrapDoorPos = {0, 1056, -480};
static short FloorTrapDoorBounds[12] = {-256, 256, 0, 0, -1024, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR FloorTrapDoorPos = {0, 0, -655};
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
static short FireBounds[12] = {0, 0, 0, 0, 0, 0, -1820, 1820, -5460, 5460, -1820, 1820};

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

	ITEM_INFO* item = &Items[itemNumber];
	if (TriggerActive(item))
	{
		if (item->triggerFlags < 0)
		{
			short flags = -item->triggerFlags;
			if ((flags & 7) == 2 || (flags & 7) == 7)
			{
				PHD_3DPOS* pos = &item->pos;
				SoundEffect(SFX_D_METAL_CAGE_OPEN, &item->pos, 0);
				TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
				g = (GetRandomControl() & 0x1F) + 96;
				r = (GetRandomControl() & 0x3F) + 192;
				falloff = (GetRandomControl() & 3) + 20;
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
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 0, item->roomNumber, item->itemFlags[2]);

				if (item->itemFlags[1])
				{
					SoundEffect(SFX_D_METAL_CAGE_OPEN, &item->pos, 0);
					
					if (item->itemFlags[1] <= -8192)
						TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
					else
						TriggerSuperJetFlame(item, item->itemFlags[1], GlobalCounter & 1);
					
					r = (GetRandomControl() & 0x3F) + 192;
					g = (GetRandomControl() & 0x1F) + 96;
					falloff = (-item->itemFlags[1] >> 10) - (GetRandomControl() & 1) + 16;
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, falloff, r, g, 0);
				}
				else
				{
					r = (GetRandomControl() & 0x3F) + 192;
					g = (GetRandomControl() & 0x1F) + 96;
					falloff = 10 - (GetRandomControl() & 1);
					TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, falloff, r, g, 0);
				}
			}

			SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
			return;
		}

		AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, item->roomNumber, 0);
		
		r = (GetRandomControl() & 0x3F) + 192;
		g = (GetRandomControl() & 0x1F) + 96;
		falloff = 16 - (GetRandomControl() & 1);
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, falloff, r, g, 0);
		
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		
		if (!Lara.burn
			&& item->triggerFlags != 33
			&& ItemNearLara(&item->pos, 600)
			&& (SQUARE(LaraItem->pos.xPos - item->pos.xPos) + SQUARE(LaraItem->pos.zPos - item->pos.zPos) < 0x40000)
			&& Lara.waterStatus != LW_FLYCHEAT)
		{
			LaraBurn();
		}
	}
}

void FlameEmitter2Control(short itemNumber)//5A1BC, 5A638 (F)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags < 0)
		{
			if (item->itemFlags[0])
			{
				if (item->itemFlags[0] == 1)
				{
					DoFlipMap(-item->triggerFlags);
					FlipMap[-item->triggerFlags] ^= 0x3E00;
					item->itemFlags[0] = 2;
				}
			}
			else
			{
				if (item->triggerFlags < -100)
					item->triggerFlags += 100;

				item->itemFlags[0] = 1;
			}
		}
		else
		{
			if (item->triggerFlags != 2)
			{
				if (item->triggerFlags == 123)
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1, item->roomNumber, item->itemFlags[3]);
				else
					AddFire(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1 - item->triggerFlags, item->roomNumber, item->itemFlags[3]);
			}

			if (item->triggerFlags == 0 || item->triggerFlags == 2)
			{
				int r = (GetRandomControl() & 0x3F) + 192;
				int g = (GetRandomControl() & 0x1F) + 96;

				if (item->itemFlags[3])
				{
					r = r * item->itemFlags[3] >> 8;
					g = g * item->itemFlags[3] >> 8;
				}

				TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, 10, r, g, 0);
			}

			SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		}
	}
}

void FlameControl(short fxNumber)
{
	FX_INFO* fx = &EffectList[fxNumber];

	for (int i = 0; i < 14; i++)
	{
		if (!(Wibble & 0xC))
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

			TriggerFireFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, -1, 255 - Lara.burnSmoke);
		}
	}

	byte r = (GetRandomControl() & 0x3F) + 192;
	byte g = (GetRandomControl() & 0x1F) + 96;
	byte b;

	if (!Lara.burnSmoke)
	{
		if (!Lara.burnBlue)
		{
			TriggerDynamicLight(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 13, r, g, 0);
		}
		else
		{
			if (Lara.burnBlue == 128)
			{
				b = r;
				TriggerDynamicLight(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 13, 0, g, b);
			}
			else if (Lara.burnBlue == 256)
			{
				b = g;
				g = r;
				TriggerDynamicLight(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 13, 0, g, b);
			}
		}
	}

	if (LaraItem->roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, LaraItem->roomNumber);
	
	int wh = GetWaterHeight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);
	if (wh == NO_HEIGHT || fx->pos.yPos <= wh || Lara.burnBlue)
	{
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &fx->pos, 0);

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
			item->hitPoints = -1;
			item->hitStatus = true;
			LaraBurn();
		}
	}
}

void InitialiseTrapDoor(short itemNumber) // (F) (D)
{
	ITEM_INFO* item;

	item = &Items[itemNumber];
	CloseTrapDoor(item);
}

void TrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll) // (F) (D)
{
	ITEM_INFO* item;

	item = &Items[itemNumber];
	if (item->currentAnimState == 1 && item->frameNumber == Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll) // (F) (D)
{
	ITEM_INFO* item;
	int result, result2;

	item = &Items[itemNumber];
	result = TestLaraPosition(CeilingTrapDoorBounds, item, l);
	l->pos.yRot += ANGLE(180);
	result2 = TestLaraPosition(CeilingTrapDoorBounds, item, l);
	l->pos.yRot += ANGLE(180);
	if (TrInput & IN_ACTION && item->status != ITEM_DEACTIVATED && l->currentAnimState == STATE_LARA_JUMP_UP && l->gravityStatus && Lara.gunStatus == LG_NO_ARMS && (result || result2))
	{
		AlignLaraPosition(&CeilingTrapDoorPos, item, l);
		if (result2)
			l->pos.yRot += ANGLE(180);
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = LG_HANDS_BUSY;
		l->gravityStatus = false;
		l->fallspeed = 0;
		l->animNumber = ANIMATION_LARA_CEILING_TRAPDOOR_OPEN;
		l->frameNumber = Anims[l->animNumber].frameBase;
		l->currentAnimState = STATE_LARA_FREEFALL_BIS;
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
		item->goalAnimState = 1;

		UseForcedFixedCamera = 1;
		ForcedFixedCamera.x = item->pos.xPos - phd_sin(item->pos.yRot) / 16;
		ForcedFixedCamera.y = item->pos.yPos + 1024;
		ForcedFixedCamera.z = item->pos.zPos - phd_cos(item->pos.yRot) / 16;
		ForcedFixedCamera.roomNumber = item->roomNumber;
	}
	else
	{
		if (item->currentAnimState == 1)
			UseForcedFixedCamera = 0;
	}

	if (item->currentAnimState == 1 && item->frameNumber == Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll) // (F) (D)
{
	ITEM_INFO* item;

	item = &Items[itemNumber];
	if (TrInput & IN_ACTION && item->status != ITEM_DEACTIVATED && l->currentAnimState == STATE_LARA_STOP && l->animNumber == ANIMATION_LARA_STAY_IDLE && Lara.gunStatus == LG_NO_ARMS
		|| Lara.isMoving && Lara.generalPtr == (void *) itemNumber)
	{
		if (TestLaraPosition(FloorTrapDoorBounds, item, l))
		{
			if (MoveLaraPosition(&FloorTrapDoorPos, item, l))
			{
				l->animNumber = ANIMATION_LARA_FLOOR_TRAPDOOR_OPEN;
				l->frameNumber = Anims[l->animNumber].frameBase;
				l->currentAnimState = STATE_LARA_TRAPDOOR_FLOOR_OPEN;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				AddActiveItem(itemNumber);
				item->status = ITEM_ACTIVE;
				item->goalAnimState = 1;

				UseForcedFixedCamera = 1;
				ForcedFixedCamera.x = item->pos.xPos - phd_sin(item->pos.yRot) / 8;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				if (ForcedFixedCamera.y < Rooms[item->roomNumber].maxceiling)
					ForcedFixedCamera.y = Rooms[item->roomNumber].maxceiling;
				ForcedFixedCamera.z = item->pos.zPos - phd_cos(item->pos.yRot) / 8;
				ForcedFixedCamera.roomNumber = item->roomNumber;
			}
			else
			{
				Lara.generalPtr = (void *) itemNumber;
			}
		}
	}
	else
	{
		if (item->currentAnimState == 1)
			UseForcedFixedCamera = 0;
	}

	if (item->currentAnimState == 1 && item->frameNumber == Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void TrapDoorControl(short itemNumber) // (F) (D)
{
	ITEM_INFO* item;

	item = &Items[itemNumber];
	if (TriggerActive(item))
	{
		if (!item->currentAnimState && item->triggerFlags >= 0)
		{
			item->goalAnimState = 1;
		}
		else if (item->frameNumber == Anims[item->animNumber].frameEnd && CurrentLevel == 14 && item->objectNumber == ID_TRAPDOOR1)
		{
			item->status = ITEM_INVISIBLE;
		}
	}
	else
	{
		item->status = ITEM_ACTIVE;

		if (item->currentAnimState == 1)
		{
			item->goalAnimState = 0;
		}
	}

	AnimateItem(item);

	if (item->currentAnimState == 1 && (item->itemFlags[2] || JustLoaded))
	{
		OpenTrapDoor(item);
	}
	else if (!item->currentAnimState && !item->itemFlags[2])
	{
		CloseTrapDoor(item);
	}
}

void CloseTrapDoor(ITEM_INFO* item) // (F) (D)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	unsigned short pitsky;

	r = &Rooms[item->roomNumber];
	floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
	pitsky = 0;

	if (item->pos.yPos == r->minfloor)
	{
		pitsky = floor->pitRoom;
		floor->pitRoom = NO_ROOM;
		r = &Rooms[pitsky];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		pitsky |= floor->skyRoom << 8;
		floor->skyRoom = NO_ROOM;
	}
	else if (item->pos.yPos == r->maxceiling)
	{
		pitsky = floor->skyRoom;
		floor->skyRoom = NO_ROOM;
		r = &Rooms[pitsky];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		pitsky = pitsky << 8 | floor->pitRoom;
		floor->pitRoom = NO_ROOM;
	}

	item->itemFlags[2] = 1;
	item->itemFlags[3] = pitsky;
}

void OpenTrapDoor(ITEM_INFO* item) // (F) (D)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	unsigned short pitsky;

	r = &Rooms[item->roomNumber];
	floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
	pitsky = item->itemFlags[3];

	if (item->pos.yPos == r->minfloor)
	{
		floor->pitRoom = (unsigned char) pitsky;
		r = &Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		floor->skyRoom = pitsky >> 8;
	}
	else
	{
		floor->skyRoom = pitsky >> 8;
		r = &Rooms[floor->skyRoom];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		floor->pitRoom = (unsigned char) pitsky;
	}

	item->itemFlags[2] = 0;
}

void InitialiseFallingBlock(short itemNumber)
{
	Items[itemNumber].meshBits = 1;
}

void FallingBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];
	if (!item->itemFlags[0] && !item->triggerFlags && item->pos.yPos == l->pos.yPos)
	{
		if (!((item->pos.xPos ^ l->pos.xPos) & 0xFFFFFC00) && !((l->pos.zPos ^ item->pos.zPos) & 0xFFFFFC00))
		{
			SoundEffect(SFX_ROCK_FALL_CRUMBLE, &item->pos, 0);
			AddActiveItem(itemNum);

			item->itemFlags[0] = 0;
			item->status = ITEM_ACTIVE;
			item->flags |= 0x3E00;
		}
	}
}

void FallingBlockControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

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

void FallingBlockFloor(ITEM_INFO* item, int x, int y, int z, int* height)
{
	if (!((x ^ item->pos.xPos) & 0xFFFFFC00) && !((z ^ item->pos.zPos) & 0xFFFFFC00))
	{
		if (y <= item->pos.yPos)
		{
			*height = item->pos.yPos;
			HeightType = WALL;
			OnFloor = 1;
		}
	}
}

void FallingBlockCeiling(ITEM_INFO* item, int x, int y, int z, int* height)
{
	if (!((x ^ item->pos.xPos) & 0xFFFFFC00) && !((z ^ item->pos.zPos) & 0xFFFFFC00))
	{
		if (y > item->pos.yPos)
		{
			*height = item->pos.yPos + 256;
		}
	}
}

void InitialiseWreckingBall(short itemNumber)
{
	ITEM_INFO* item;
	short room;

	item = &Items[itemNumber];
	item->itemFlags[3] = find_a_fucking_item(ID_ANIMATING16) - Items.data();
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

	item = &Items[itemNumber];
	if (TestBoundsCollide(item, l, coll->radius))
	{
		x = l->pos.xPos;
		y = l->pos.yPos;
		z = l->pos.zPos;
		test = (x & 1023) > 256 && (x & 1023) < 768 && (z & 1023) > 256 && (z & 1023) < 768;
		damage = item->fallspeed > 0 ? 96 : 0;
		if (ItemPushLara(item, l, coll, coll->enableSpaz, 1))
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
			if (!coll->enableBaddiePush || test)
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

	item = &Items[itemNumber];
	test = 1;
	item2 = &Items[item->itemFlags[3]];
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
	if (GlobalPlayingCutscene)
	{
		room = item->roomNumber;
		item->goalAnimState = 0;
		item->pos.xPos = 47616;
		item->pos.zPos = 34816;
		item->pos.yPos = GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room), item->pos.xPos, item->pos.yPos, item->pos.zPos) + 1664;
	}
	else if (item->itemFlags[1] <= 0)
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
			SoundEffect(SFX_GRAB_MOTOR_B_LP, &item->pos, 0);
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
			SoundEffect(SFX_GRAB_MOTOR_B_LP, &item->pos, 0);
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
			SoundEffect(SFX_GRAB_MOTOR_A, &item->pos, 0);
		}
		if ((item->pos.xPos & 0x3FF) == 512 && (item->pos.zPos & 0x3FF) == 512)
			item->itemFlags[0] = 0;
		if (x == item->pos.xPos && z == item->pos.zPos && test)
		{
			if (item->itemFlags[1] != -1)
			{
				StopSoundEffect(SFX_GRAB_MOTOR_B_LP);
				SoundEffect(SFX_GRAB_MOTOR_C, &item->pos, 0);
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
		else if (item->frameNumber == Anims[item->animNumber].frameEnd)
		{
			SoundEffect(SFX_GRAB_DROP, &item->pos, 0);
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
			StopSoundEffect(SFX_GRAB_WINCH_UP_LP);
			item->itemFlags[0] = 1;
			item->pos.yPos = item2->pos.yPos + 1644;
			if (item->fallspeed < -32)
			{
				SoundEffect(SFX_GRAB_IMPACT, &item->pos, 4104);
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
			SoundEffect(SFX_GRAB_WINCH_UP_LP, &item->pos, 0);
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

void FlameEmitterCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll) // (F) (D)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (Lara.gunType != WEAPON_TORCH
		|| Lara.gunStatus != LG_READY
		|| Lara.leftArm.lock 
		|| Lara.litTorch == (item->status & 1)
		|| item->timer == -1
		|| !(TrInput & IN_ACTION)
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE
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
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -800;
			FireBounds[5] = 800;
			break;

		case ID_FLAME_EMITTER2:
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -600;
			FireBounds[5] = 600;
			break;

		case ID_BURNING_ROOTS:
			FireBounds[0] = -384;
			FireBounds[1] = 384;
			FireBounds[2] = 0;
			FireBounds[3] = 2048;
			FireBounds[4] = -384;
			FireBounds[5] = 384;
			break;

		}

		short oldYrot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;

		if (TestLaraPosition(FireBounds, item, l))
		{
			if (item->objectNumber == ID_BURNING_ROOTS)
			{
				l->animNumber = ANIMATION_LARA_TORCH_LIGHT_5;
			}
			else
			{
				int dy = abs(l->pos.yPos - item->pos.yPos);
				l->itemFlags[3] = 1;
				l->animNumber = (dy >> 8) + ANIMATION_LARA_TORCH_LIGHT_1;
			}
			
			l->currentAnimState = STATE_LARA_MISC_CONTROL;
			l->frameNumber = Anims[l->animNumber].frameBase;
			Lara.flareControlLeft = false;
			Lara.leftArm.lock = 3;
			Lara.generalPtr = (void*)itemNumber;
		}
		
		item->pos.yRot = oldYrot;
	}

	if (Lara.generalPtr == (void*)itemNumber 
		&& item->status != ITEM_ACTIVE 
		&& l->currentAnimState == STATE_LARA_MISC_CONTROL)
	{
		if (l->animNumber >= ANIMATION_LARA_TORCH_LIGHT_1 && l->animNumber <= ANIMATION_LARA_TORCH_LIGHT_5)
		{
			if (l->frameNumber - Anims[l->animNumber].frameBase == 40)
			{
				TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, item->flags & 0x3E00);
				
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
	ITEM_INFO* item = &Items[itemNumber];
	
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
	ITEM_INFO* item = &Items[itemNumber];
	
	if (item->triggerFlags > 0)
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
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			// Electricity bolts

			SoundEffect(SFX_2GUNTEX_HIT_GUNS, &item->pos, 0);

			byte g = (GetRandomControl() & 0x3F) + 192;
			byte b = (GetRandomControl() & 0x3F) + 192;

			PHD_VECTOR src;
			PHD_VECTOR dest;

			src.x = item->pos.xPos;
			src.y = item->pos.yPos;
			src.z = item->pos.zPos;

			// Secondary energy arc
			if (!(GlobalCounter & 3))
			{
				if (item->triggerFlags == 2 || item->triggerFlags == 4)
				{
					dest.x = item->pos.xPos + 2048 * phd_sin(item->pos.yRot - ANGLE(180)) >> W2V_SHIFT;
					dest.y = item->pos.yPos;
					dest.z = item->pos.zPos + 2048 * phd_cos(item->pos.yRot - ANGLE(180)) >> W2V_SHIFT;
					
					if (GetRandomControl() & 3)
					{
						//TriggerEnergyArc(&src, &dest, (GetRandomControl() & 0x1F) + 64, b | ((g | 0x180000) << 8), 0, 32, 3);
					}
					else
					{
						//TriggerEnergyArc(&src, &dest, (GetRandomControl() & 0x1F) + 96, b | ((g | 0x200000) << 8), 1, 32, 3);
					}
				}
			}

			// Connected to item
			if (item->triggerFlags >= 3 && !(GlobalCounter & 1))
			{
				short targetItemNumber = item->itemFlags[((GlobalCounter >> 2) & 1) + 2];
				ITEM_INFO* targetItem = &Items[targetItemNumber];

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

			SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);

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