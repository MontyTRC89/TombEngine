#include "traps.h"
#include "..\Global\global.h"
#include "items.h"
#include "effect2.h"
#include "tomb4fx.h"
#include "effects.h"
#include "lara.h"
#include "collide.h"

static short CeilingTrapDoorBounds[12] = {-256, 256, 0, 900, -768, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR CeilingTrapDoorPos = {0, 1056, -480};
static short FloorTrapDoorBounds[12] = {-256, 256, 0, 0, -1024, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR FloorTrapDoorPos = {0, 0, -655};

void LaraBurn()
{
	if (!Lara.burn && !Lara.burnSmoke)
	{
		short fxNum = CreateNewEffect(LaraItem->roomNumber);
		if (fxNum != NO_ITEM)
		{
			Effects[fxNum].objectNumber = ID_FLAME;
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
			&& (SQUARE(LaraItem->pos.xPos - item->pos.xPos) + SQUARE(LaraItem->pos.zPos - item->pos.zPos) < 0x40000))
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
	FX_INFO* fx = &Effects[fxNumber];

	for (int i = 0; i < 14; i++)
	{
		if (!(Wibble & 0xC))
		{
			fx->pos.xPos = 0;
			fx->pos.yPos = 0;
			fx->pos.zPos = 0;

			GetLaraJointPosition((PHD_VECTOR*)& fx->pos, i);

			if (Lara.BurnCount)
			{
				Lara.BurnCount--;
				if (!Lara.BurnCount)
					Lara.burnSmoke = true;
			}

			TriggerFireFlame(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, -1, 255 - ((Lara.currentZvel >> 10) & 1));
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

void InitialiseTrapDoor(short itemNumber)
{
	ITEM_INFO* item;

	item = &Items[itemNumber];
	CloseTrapDoor(item);
}

void TrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &Items[itemNumber];
	if (item->currentAnimState == 1 && item->frameNumber == Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
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
		ForcedFixedCamera.x = item->pos.xPos - SIN(item->pos.yRot) / 16;
		ForcedFixedCamera.y = item->pos.yPos + 1024;
		ForcedFixedCamera.z = item->pos.zPos - COS(item->pos.yRot) / 16;
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

void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
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
				ForcedFixedCamera.x = item->pos.xPos - SIN(item->pos.yRot) / 8;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				if (ForcedFixedCamera.y < Rooms[item->roomNumber].maxceiling)
					ForcedFixedCamera.y = Rooms[item->roomNumber].maxceiling;
				ForcedFixedCamera.z = item->pos.zPos - COS(item->pos.yRot) / 8;
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

void TrapDoorControl(short itemNumber)
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

void CloseTrapDoor(ITEM_INFO* item)
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

void OpenTrapDoor(ITEM_INFO* item)
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
			ExplodingDeath2(itemNumber, -1, 15265);
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
			HeightType = 0;
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