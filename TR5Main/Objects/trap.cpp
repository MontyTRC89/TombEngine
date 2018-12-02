#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\people.h"
#include "..\Game\effects.h"
#include "..\Game\effect2.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\inventory.h"
#include "..\Game\collide.h"
#include "..\Game\draw.h"

__int16 StargateBounds[24] = 
{
	0xFE00, 0x0200, 0xFC00, 0xFC80, 0xFFA0, 0x0060, 0xFE00, 0x0200, 
	0xFF80, 0x0000, 0xFFA0, 0x0060, 0xFE00, 0xFE80, 0xFC00, 0x0000,
	0xFFA0, 0x0060, 0x0180, 0x0200, 0xFC00, 0x0000, 0xFFA0, 0x0060
};

BITE_INFO sentryGunBite = { 0, 0, 0, 8 };

void __cdecl FourBladesControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (frameNumber <= 5 || frameNumber >= 58 || frameNumber >= 8 && frameNumber <= 54)
			item->itemFlags[0] = 0;
		else
		{
			if (frameNumber >= 6 && frameNumber <= 7)
			{
				item->itemFlags[3] = 20;
				item->itemFlags[0] = 30;
			}
			else
			{
				if (frameNumber >= 55 && frameNumber <= 57)
				{
					item->itemFlags[3] = 200;
					item->itemFlags[0] = 30;
				}
			}
		}

		AnimateItem(item);
	}
}

void __cdecl BirdBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[3] = 100;
	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (frameNumber <= 14 || frameNumber >= 31)
			item->itemFlags[0] = 0;
		else
			item->itemFlags[0] = 6;

		AnimateItem(item);
	}
}

void __cdecl CatwalkBlaldeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd || frameNumber < 38)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 100;

		AnimateItem(item);
	}
}

void __cdecl PlinthBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 200;

		AnimateItem(item);
	}
}

void __cdecl InitialiseSethBlade(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->animNumber = Objects[ID_SETH_BLADE].animIndex + 1;
	item->goalAnimState = 2;
	item->currentAnimState = 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->itemFlags[2] = abs(item->triggerFlags);
}

void __cdecl SethBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[0] = 0;
	if (TriggerActive(item))
	{
		if (item->currentAnimState == 2)
		{
			if (item->itemFlags[2] > 1)
			{
				item->itemFlags[2]--;
			}
			else if (item->itemFlags[2] == 1)
			{
				item->goalAnimState = 1;
				item->itemFlags[2] = 0;
			}
			else if (!item->itemFlags[2])
			{
				if (item->triggerFlags > 0)
				{
					item->itemFlags[2] = item->triggerFlags;
				}
			}
		}
		else
		{
			__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

			if (item->frameNumber != Anims[item->animNumber].frameBase && frameNumber <= 6)
			{
				item->itemFlags[0] = -1;
				item->itemFlags[3] = 1000;
			}
			else if (frameNumber >= 7 && frameNumber <= 15)
			{
				item->itemFlags[0] = 448;
				item->itemFlags[3] = 1000;
			}
			else
			{
				item->itemFlags[0] = 0;
				item->itemFlags[3] = 1000;
			}
		}

		AnimateItem(item);
	}
}

void __cdecl ChainControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags)
	{
		item->itemFlags[2] = 1;
		item->itemFlags[3] = 75;

		if (TriggerActive(item))
		{
			item->itemFlags[0] = 30846;
			AnimateItem(item);
			return;
		}
	}
	else
	{
		item->itemFlags[3] = 25;

		if (TriggerActive(item))
		{
			item->itemFlags[0] = 1920;
			AnimateItem(item);
			return;
		}
	}

	item->itemFlags[0] = 0;
}

void __cdecl PloughControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[3] = 50;
	if (TriggerActive(item))
	{
		item->itemFlags[0] = 258048;
		AnimateItem(item);
	}
	else
	{
		item->itemFlags[0] = 0;
	}
}

void __cdecl CogControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TriggerActive(item))
	{
		item->status = ITEM_ACTIVE;
		// *(_DWORD *)&item->gap4C[5526] = *(_DWORD *)&item->gap4C[5526] & 0xFFFFFFFB | 2;
		AnimateItem(item);

		if (item->triggerFlags == 666)
		{
			PHD_VECTOR pos;
			GetJointAbsPosition(item, &pos, 0);
			SoundEffect(65, (PHD_3DPOS *)&pos, 0);

			if (item->frameNumber == Anims[item->animNumber].frameEnd)
				item->flags &= 0xC1;
		}
	}
	else if (item->triggerFlags == 2)
	{
		item->status |= ITEM_INVISIBLE;
	}
}

void __cdecl SpikeballControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TriggerActive(item))
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if ((frameNumber <= 14 || frameNumber >= 24) && (frameNumber < 138 || frameNumber > 140))
		{
			if (frameNumber < 141)
				item->itemFlags[0] = 0;
			else
			{
				item->itemFlags[3] = 50;
				item->itemFlags[0] = 0x7FF800;
			}
		}
		else
		{
			item->itemFlags[3] = 150;
			item->itemFlags[0] = 0x7FF800;
		}

		AnimateItem(item);
	}
	else
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
}

void __cdecl StargateControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	item->itemFlags[3] = 50;

	if (TriggerActive(item))
	{
		SoundEffect(SFX_TR4_STARGATE_SWIRL_ID23, &item->pos, 0);
		item->itemFlags[0] = 57521664;
		AnimateItem(item);
	}
	else
		item->itemFlags[0] = 0;
}

void __cdecl StargateCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &Items[itemNum];
	
	if (item->status == ITEM_INVISIBLE)
		return;

	if (TestBoundsCollide(item, l, c->radius))
	{
		for (__int32 i = 0; i < 8; i++)
		{
			GlobalCollisionBounds.X1 = StargateBounds[3 * i + 0];
			GlobalCollisionBounds.Y1 = StargateBounds[3 * i + 1];
			GlobalCollisionBounds.Z1 = StargateBounds[3 * i + 2];

			if (TestWithGlobalCollisionBounds(item, l, c))
				ItemPushLara(item, l, c, 0, 2);
		}

		__int32 result = TestCollision(item, l);
		if (result)
		{
			result &= item->itemFlags[0];
			__int32 flags = item->itemFlags[0];

			if (result)
			{
				__int32 j = 0;
				do
				{
					if (result & 1)
					{
						GlobalCollisionBounds.X1 = SphereList[j].x - SphereList[j].r - item->pos.xPos;
						GlobalCollisionBounds.Y1 = SphereList[j].y - SphereList[j].r - item->pos.yPos;
						GlobalCollisionBounds.Z1 = SphereList[j].z - SphereList[j].r - item->pos.zPos;
						GlobalCollisionBounds.X2 = SphereList[j].x + SphereList[j].r - item->pos.xPos;
						GlobalCollisionBounds.Y2 = SphereList[j].y + SphereList[j].r - item->pos.yPos;
						GlobalCollisionBounds.Z2 = SphereList[j].z + SphereList[j].r - item->pos.zPos;

						__int32 oldX = LaraItem->pos.xPos;
						__int32 oldY = LaraItem->pos.yPos;
						__int32 oldZ = LaraItem->pos.zPos;

						if (ItemPushLara(item, l, c, flags & 1, 2))
						{
							if ((flags & 1) &&
								(oldX != LaraItem->pos.xPos || oldY != LaraItem->pos.yPos || oldZ != LaraItem->pos.zPos) &&
								TriggerActive(item))
							{
								DoBloodSplat((GetRandomControl() & 0x3F) + l->pos.xPos - 32, 
											 (GetRandomControl() & 0x1F) + SphereList[j].y - 16, 
											 (GetRandomControl() & 0x3F) + l->pos.zPos - 32, 
											 (GetRandomControl() & 3) + 2, 
											 2 * GetRandomControl(), 
											 l->roomNumber);
								LaraItem->hitPoints -= 100;
							}
						}
					}

					result >>= 1;
					j++;
					flags >>= 1;

				} while (result);
			}
		}
	}
}

void __cdecl ControlSpikeWall(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	/* Move wall */
	if (TriggerActive(item) && item->status != ITEM_DEACTIVATED)
	{
		__int32 x = item->pos.xPos + SIN(item->pos.yRot) >> WALL_SHIFT;
		__int32 z = item->pos.zPos + COS(item->pos.yRot) >> WALL_SHIFT;

		__int16 roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);

		if (GetFloorHeight(floor, x, item->pos.yPos, z) != item->pos.yPos)
		{
			item->status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_ROLLING_BALL);
		}
		else
		{
			item->pos.xPos = x;
			item->pos.zPos = z;
			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNum, roomNumber);
			SoundEffect(SFX_ROLLING_BALL, &item->pos, 0);
		}
	}

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 15;
		LaraItem->hitStatus = true;

		DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - 512, LaraItem->pos.zPos, 4, item->pos.yRot, LaraItem->roomNumber, 3);
		item->touchBits = 0;

		SoundEffect(56, &item->pos, 0);
	}
}

void __cdecl InitialiseSpinningBlade(__int16 item_number)
{
	ITEM_INFO* item = &Items[item_number];

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
}

void __cdecl SpinningBlade(__int16 item_number)
{
	bool spinning = false;

	ITEM_INFO* item = &Items[item_number];

	if (item->currentAnimState == 2)
	{
		if (item->goalAnimState != 1)
		{
			__int32 x = item->pos.xPos + (WALL_SIZE * 3 / 2 * SIN(item->pos.yRot) >> W2V_SHIFT);
			__int32 z = item->pos.zPos + (WALL_SIZE * 3 / 2 * COS(item->pos.yRot) >> W2V_SHIFT);

			__int16 roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
			__int32 height = GetFloorHeight(floor, x, item->pos.yPos, z);

			if (height == NO_HEIGHT)
				item->goalAnimState = 1;
		}

		spinning = true;

		if (item->touchBits)
		{
			LaraItem->hitStatus = true;
			LaraItem->hitPoints -= 100;

			DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - STEP_SIZE * 2, LaraItem->pos.zPos, (__int16)(item->speed * 2), LaraItem->pos.yRot, LaraItem->roomNumber, 2);
		}

		SoundEffect(231, &item->pos, 0);
	}
	else
	{
		if (TriggerActive(item))
			item->goalAnimState = 2;
		spinning = false;
	}

	AnimateItem(item);

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO*  floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	item->floor = item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	if (roomNumber != item->roomNumber)
		ItemNewRoom(item_number, roomNumber);

	if (spinning && item->currentAnimState == 1)
		item->pos.yRot += -ANGLE(180);
}

void __cdecl InitialiseKillerStatue(__int16 item_number)
{
	ITEM_INFO* item = &Items[item_number];

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
}

void __cdecl KillerStatueControl(__int16 item_number)
{
	ITEM_INFO *item;
	__int32 x, y, z;
	__int16 d;

	item = &Items[item_number];

	if (TriggerActive(item) && item->currentAnimState == 1)
		item->goalAnimState = 2;
	else
		item->goalAnimState = 1;

	if ((item->touchBits & 0x80) && item->currentAnimState == 2)
	{
		LaraItem->hitStatus = 1;
		LaraItem->hitPoints -= 20;

		__int32 x = LaraItem->pos.xPos + (GetRandomControl() - 16384) / 256;
		__int32 z = LaraItem->pos.zPos + (GetRandomControl() - 16384) / 256;
		__int32 y = LaraItem->pos.yPos - GetRandomControl() / 44;
		__int32 d = (GetRandomControl() - 16384) / 8 + LaraItem->pos.yRot;
		DoBloodSplat(x, y, z, LaraItem->speed, d, LaraItem->roomNumber);
	}

	AnimateItem(item);
}

void __cdecl SpringBoardControl(__int16 item_number)
{
	ITEM_INFO* item = &Items[item_number];

	if (item->currentAnimState == 0 && LaraItem->pos.yPos == item->pos.yPos &&
		(LaraItem->pos.xPos >> WALL_SHIFT) == (item->pos.xPos >> WALL_SHIFT) &&
		(LaraItem->pos.zPos >> WALL_SHIFT) == (item->pos.zPos >> WALL_SHIFT))
	{
		if (LaraItem->hitPoints <= 0)
			return;

		if (LaraItem->currentAnimState == STATE_LARA_WALK_BACK || LaraItem->currentAnimState == STATE_LARA_RUN_BACK)
			LaraItem->speed = -LaraItem->speed;

		LaraItem->fallspeed = -240;
		LaraItem->gravityStatus = true;
		LaraItem->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
		LaraItem->frameNumber = GF2(ID_LARA, ANIMATION_LARA_FREE_FALL_FORWARD, 0);
		LaraItem->currentAnimState = STATE_LARA_JUMP_FORWARD;
		LaraItem->goalAnimState = STATE_LARA_JUMP_FORWARD;

		item->goalAnimState = 1;
	}

	AnimateItem(item);
}

void __cdecl InitialiseSlicerDicer(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	__int32 dx = SIN(item->pos.yRot + ANGLE(90)) >> 5;
	__int32 dz = COS(item->pos.yRot + ANGLE(90)) >> 5;

	item->pos.xPos += dx;
	item->pos.zPos += dz;

	item->itemFlags[0] = item->pos.xPos >> 8;
	item->itemFlags[1] = (item->pos.yPos - 4608) >> 8;
	item->itemFlags[2] = item->pos.zPos >> 8;
	item->itemFlags[3] = 50;
}

void __cdecl SlicerDicerControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP_2_ID20, &item->pos, 0);
	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP_1_ID12, &item->pos, 0);
	
	__int32 factor = (9 * COS(item->triggerFlags) << 9 >> W2V_SHIFT) * COS(item->pos.yRot) >> W2V_SHIFT;

	item->pos.xPos = (item->itemFlags[0] << 8) + factor;
	item->pos.yPos = (item->itemFlags[1] << 8) - 4608 * SIN(item->triggerFlags);
	item->pos.zPos = (item->itemFlags[2] << 8) + factor;

	item->triggerFlags += 170;

	__int16 roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNum, roomNumber);

	AnimateItem(item);
}

void __cdecl BladeCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->status == ITEM_INVISIBLE)
		return;

	if (item->itemFlags[3]) // Check this
	{
		if (TestBoundsCollide(item, l, coll->radius))
		{
			__int32 oldX = LaraItem->pos.xPos;
			__int32 oldY = LaraItem->pos.yPos;
			__int32 oldZ = LaraItem->pos.zPos;

			__int32 dx = 0;
			__int32 dy = 0;
			__int32 dz = 0;

			if (ItemPushLara(item, l, coll, 1, 1))
			{
				LaraItem->hitPoints -= item->itemFlags[3];

				dx = oldX - LaraItem->pos.xPos;
				dy = oldY - LaraItem->pos.yPos;
				dz = oldZ - LaraItem->pos.zPos;

				if ((dx || dy || dz) && TriggerActive(item))
				{
					DoBloodSplat((GetRandomControl() & 0x3F) + l->pos.xPos - 32,
						l->pos.yPos - (GetRandomControl() & 0x1FF) - 256,
						(GetRandomControl() & 0x3F) + l->pos.zPos - 32,
						(GetRandomControl() & 3) + (item->itemFlags[3] >> 5) + 2,
						2 * GetRandomControl(),
						l->roomNumber);
				}

				if (!coll->enableBaddiePush)
				{
					LaraItem->pos.xPos += dx;
					LaraItem->pos.yPos += dy;
					LaraItem->pos.zPos += dz;
				}
			}
		}
	}
}

void __cdecl InitialiseMine(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags)
		item->meshBits = 0;
}

void __cdecl MineControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	__int32 num = GetSpheres(item, SphereList, true);
	if (item->itemFlags[0] >= 150)
	{
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0);
		SoundEffect(SFX_EXPLOSION2, &item->pos, 0);
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0x1800004);

		if (num > 0)
		{
			SPHERE* sphere = &SphereList[0];

			for (__int32 i = 0; i < num; i++)
			{
				if (i >= 7 && i != 9)
				{
					TriggerExplosionSparks(sphere->x, sphere->y, sphere->z, 3, -2, 0, -item->roomNumber);
					TriggerExplosionSparks(sphere->x, sphere->y, sphere->z, 3, -1, 0, -item->roomNumber);
					TriggerShockwave((PHD_3DPOS*)sphere, 19922992, (GetRandomControl() & 0x1F) + 112, 545284096, 2048, 0);
				}

				sphere++;
			}

			for (__int32 i = 0; i < num; i++)
				ExplodeItemNode(item, i, 0, -128);
		}

		FlashFadeR = 255;
		FlashFadeG = 192;
		FlashFadeB = 64;
		FlashFader = 32;

		__int16 currentItemNumber = Rooms[item->roomNumber].itemNumber;

		// Make the sentry gun explode?
		while (currentItemNumber != NO_ITEM)
		{
			ITEM_INFO* currentItem = &Items[currentItemNumber];
			
			if (currentItem->objectNumber == ID_SENTRY_GUN)
				currentItem->meshBits &= ~0x40;

			currentItemNumber = currentItem->nextItem;
		}
		
		KillItem(itemNum);
	}
	else
	{
		item->itemFlags[0]++;

		__int32 something = 4 * item->itemFlags[0];
		if (something > 255)
			something = 0;

		for (__int32 i = 0; i < num; i++)
		{
			SPHERE* sphere = &SphereList[i];

			if (i == 0 || i > 5)
				AddFire(sphere->x, sphere->y, sphere->z, 2, item->roomNumber, something);

			sphere++;
		}
		
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
	}
}

void __cdecl MineCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags && !item->itemFlags[3])
	{
		if (l->animNumber != 432 || l->frameNumber < Anims[item->animNumber].frameBase + 57)
		{
			if (TestBoundsCollide(item, l, 512))
			{
				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, 0, item->roomNumber);
				for (__int32 i = 0; i < 2; i++)
					TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -1, 0, item->roomNumber);

				item->meshBits = 1;

				ExplodeItemNode(item, 0, 0, 128);
				KillItem(itemNum);

				l->animNumber = 438;
				l->frameNumber = Anims[item->animNumber].frameBase;
				l->currentAnimState = 8;
				l->speed = 0;

				SoundEffect(SFX_TR4_MINE_EXPLOSION_OVERLAY_ID103, &item->pos, 0);
			}
		}
		else
		{
			for (__int32 i = 0; i < LevelItems; i++)
			{
				ITEM_INFO* currentItem = &Items[i];

				// Explode other mines
				if (currentItem->objectNumber == ID_MINE && currentItem->status != ITEM_INVISIBLE && !currentItem->triggerFlags)
				{
					TriggerExplosionSparks(
						currentItem->pos.xPos,
						currentItem->pos.yPos,
						currentItem->pos.zPos,
						3,
						-2,
						0,
						currentItem->roomNumber);

					for (__int32 j = 0; j < 2; j++)
						TriggerExplosionSparks(
							currentItem->pos.xPos,
							currentItem->pos.yPos,
							currentItem->pos.zPos,
							3,
							-1,
							0,
							currentItem->roomNumber);

					currentItem->meshBits = 1;

					ExplodeItemNode(currentItem, 0, 0, -32);
					KillItem(i);

					if (!(GetRandomControl() & 3))
						SoundEffect(SFX_TR4_MINE_EXPLOSION_OVERLAY_ID103, &currentItem->pos, 0);

					currentItem->status = ITEM_INVISIBLE;
				}
			}
		}
	}
}

void __cdecl InitialiseSentryGun(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->itemFlags[0] = 0;
	item->itemFlags[1] = 768;
	item->itemFlags[2] = 0;
}

void __cdecl SentryGunControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!CreatureActive(itemNum))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	AI_INFO info;
	__int32 c;

	if (creature)
	{
		// Flags set by the ID_MINE object?
		if (item->meshBits & 0x40)
		{
			if (item->itemFlags[0])
			{
				PHD_VECTOR pos;

				pos.x = sentryGunBite.x;
				pos.y = sentryGunBite.y;
				pos.z = sentryGunBite.z;

				GetJointAbsPosition(item, &pos, sentryGunBite.meshNum);

				TriggerDynamics(pos.x, pos.y, pos.z, 4 * item->itemFlags[0] + 12, 24, 16, 4);

				item->itemFlags[0]--;
			}

			if (item->itemFlags[0] & 1)
				item->meshBits |= 0x100;
			else
				item->meshBits &= ~0x100;

			if (item->triggerFlags == 0)
			{
				item->pos.yPos -= 512;
				CreatureAIInfo(item, &info);
				item->pos.yPos += 512;

				__int32 deltaAngle = info.angle - creature->jointRotation[0];

				info.ahead = true;
				if (deltaAngle <= -ANGLE(90) || deltaAngle >= ANGLE(90))
					info.ahead = false;

				if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(9 * WALL_SIZE))
					{
						bool gotPuzzle = HaveIGotItemInInventory(ID_PUZZLE_ITEM5);

						if (!gotPuzzle && !item->itemFlags[0])
						{
							if (info.distance <= SQUARE(2048))
							{
								SentryGunEffect(item);
								c = SIN((GlobalCounter & 0x1F) << 11) >> 2;
							}
							else
							{
								c = 0;
								item->itemFlags[0] = 2;

								ShotLara(item, &info, &sentryGunBite, creature->jointRotation[0], 5);
								SoundEffect(SFX_TR4_AUTOGUNS_ID358, &item->pos, 0);

								item->itemFlags[2] += 256;
								if (item->itemFlags[2] > 6144)
								{
									item->itemFlags[2] = 6144;
								}
							}
						}

						deltaAngle = c + info.angle - creature->jointRotation[0];
						if (deltaAngle <= ANGLE(10))
						{
							if (deltaAngle < -ANGLE(10))
							{
								deltaAngle = -ANGLE(10);
							}
						}
						else
						{
							deltaAngle = ANGLE(10);
						}

						creature->jointRotation[0] = deltaAngle - info.xAngle;

						CreatureJoint(item, 1, -info.xAngle);
					}
				}

				item->itemFlags[2] -= 32;

				if ((item->itemFlags[2] & 0x8000u) != 0)
				{
					item->itemFlags[2] = 0;
				}

				creature->jointRotation[3] += item->itemFlags[2];
				creature->jointRotation[2] += item->itemFlags[1];

				if (creature->jointRotation[2] > ANGLE(90) ||
					creature->jointRotation[2] < -ANGLE(90))
				{
					item->itemFlags[1] = -item->itemFlags[1];
				}
			}
			else
			{
				CreatureJoint(item, 0, (GetRandomControl() & 0x7FF) - 1024);
				CreatureJoint(item, 1, ANGLE(45));
				CreatureJoint(item, 2, (GetRandomControl() & 0x3FFF) - ANGLE(45));
			}
		}
		else
		{
			ExplodingDeath(itemNum, -1, 257);
			DisableBaddieAI(itemNum);
			KillItem(itemNum);

			item->flags |= 1u;
			item->status = ITEM_DEACTIVATED;

			RemoveAllItemsInRoom(item->roomNumber, ID_SMOKE_EMITTER_BLACK);

			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos - 768, item->pos.zPos, 3, -2, 0, item->roomNumber);
			for (__int32 i = 0; i < 2; i++)
				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos - 768, item->pos.zPos, 3, -1, 0, item->roomNumber);

			SoundEffect(SFX_EXPLOSION1, &item->pos, 25165828);
			SoundEffect(SFX_EXPLOSION2, &item->pos, 0);
		}
	}
}



void __cdecl SentryGunEffect(ITEM_INFO* item)
{
	for (__int32 i = 0; i < 3; i++)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = 1;
		spark->sR = (GetRandomControl() & 0x1F) + 48;
		spark->sG = 48;
		spark->sB = 255;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dG = (GetRandomControl() & 0x3F) + -128;
		spark->dB = 32;
		spark->colFadeSpeed = 12;
		spark->fadeToBlack = 8;
		spark->transType = 2;
		spark->life = spark->sLife = (GetRandomControl() & 0x1F) + 16;

		PHD_VECTOR pos1;
		pos1.x = -140;
		pos1.y = -30;
		pos1.z = -4;

		GetJointAbsPosition(item, &pos1, 7);

		spark->x = (GetRandomControl() & 0x1F) + pos1.x - 16;
		spark->y = (GetRandomControl() & 0x1F) + pos1.y - 16;
		spark->z = (GetRandomControl() & 0x1F) + pos1.z - 16;

		PHD_VECTOR pos2;
		pos2.x = -280;
		pos2.y = -30;
		pos2.z = -4;

		GetJointAbsPosition(item, &pos2, 7);

		__int32 v = (GetRandomControl() & 0x3F) + 192;

		spark->xVel = v * (pos2.x - pos1.x) / 10;
		spark->yVel = v * (pos2.y - pos1.y) / 10;
		spark->zVel = v * (pos2.z - pos1.z) / 10;

		spark->friction = 85;
		spark->gravity = -16 - (GetRandomControl() & 0x1F);
		spark->maxYvel = 0;
		spark->flags = 538;

		if ((GlobalCounter & 1) != 0)
		{
			v = 255;
			spark->flags = 539;
		}

		spark->scalar = 3;
		spark->dSize = v * ((GetRandomControl() & 7) + 60) >> 8;
		spark->sSize = spark->dSize >> 4;
		spark->size = spark->dSize >> 4;
	}
}

void __cdecl InitialiseBurningFloor(__int16 itemNum)
{
	Items[itemNum].requiredAnimState = 127;
}

void __cdecl BurningFloorControl(__int16 itemNum)
{

}