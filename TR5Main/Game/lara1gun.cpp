#include "lara1gun.h"
#include "..\Global\global.h"
#include "items.h"
#include "lara.h"
#include "larafire.h"
#include "draw.h"
#include "Box.h"
#include "control.h"
#include "effects.h"
#include "effect2.h"
#include "lot.h"
#include "collide.h"
#include "debris.h"
#include "lara2gun.h"

extern LaraExtraInfo g_LaraExtra;

void __cdecl FireHarpoon()
{
	// If no ammo then exit
	if (g_LaraExtra.numHarpoonAmmos <= 0)
		return;

	// Create a new item for harpoon
	__int16 itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		GAME_VECTOR pos;
		D3DXVECTOR3 dxPos;

		ITEM_INFO* item = &Items[itemNumber];

		item->shade = 0x4210 | 0x8000;
		item->objectNumber = ID_HARPOON;
		item->roomNumber = LaraItem->roomNumber;
		pos.x = dxPos.x = -2;
		pos.y = dxPos.y = 0; // -273 - 100;
		pos.z = dxPos.z = 77;

		g_Renderer->GetLaraBonePosition(&dxPos, HAND_R);
		GetLaraJointPosition((PHD_VECTOR*)&pos, HAND_R);

		/*item->pos.xPos = pos.x = dxPos.x;
		item->pos.yPos = pos.y = dxPos.y;
		item->pos.zPos = pos.z = dxPos.z;*/

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;

		InitialiseItem(itemNumber);

		if (Lara.target)
		{
			FindTargetPoint(Lara.target, &pos);

			item->pos.yRot = ATAN(pos.z - item->pos.zPos, pos.x - item->pos.xPos);
			__int32 distance = SQRT_ASM(SQUARE(pos.z - item->pos.zPos) + SQUARE(pos.x - item->pos.xPos));
			item->pos.xRot = -ATAN(distance, pos.y - item->pos.yPos);
		}
		else
		{
			item->pos.xRot = LaraItem->pos.xRot + Lara.torsoXrot;
			item->pos.yRot = LaraItem->pos.yRot + Lara.torsoYrot;
		}

		item->pos.zRot = 0;

		item->fallspeed = (__int16)(-HARPOON_SPEED * SIN(item->pos.xRot) >> W2V_SHIFT);
		item->speed = (__int16)(HARPOON_SPEED * COS(item->pos.xRot) >> W2V_SHIFT);
		item->hitPoints = HARPOON_TIME;

		AddActiveItem(itemNumber);

		Savegame.Level.AmmoUsed++;
		Savegame.Game.AmmoUsed++;
	}
}

void __cdecl ControlHarpoonBolt(__int16 itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	__int32 oldX = item->pos.xPos;
	__int32 oldY = item->pos.yPos;
	__int32 oldZ = item->pos.zPos;
	__int16 oldRoom = item->roomNumber;

	item->pos.xPos += item->speed * SIN(item->pos.yRot) >> W2V_SHIFT;
	item->pos.yPos += item->fallspeed;
	item->pos.zPos += item->speed * COS(item->pos.yRot) >> W2V_SHIFT;

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	//  First check if the harpoon has it an item
	__int16 targetItemNumber = 0;
	ITEM_INFO* target;
	for (targetItemNumber = Rooms[item->roomNumber].itemNumber; targetItemNumber != NO_ITEM; targetItemNumber = target->nextItem)
	{
		target = &Items[targetItemNumber];
		if (target == LaraItem || !target->collidable)
			continue;

		if (/*target->objectNumber == SMASH_WINDOW ||
			target->objectNumber == SMASH_OBJECT1 ||
			target->objectNumber == SMASH_OBJECT2 ||
			target->objectNumber == SMASH_OBJECT3 ||
			target->objectNumber == CARCASS ||
			target->objectNumber == EXTRAFX6 ||*/
			(target->status != ITEM_INVISIBLE && Objects[target->objectNumber].collision))
		{
			// check against bounds of target for collision
			__int16* bounds = GetBestFrame(target);
			if (item->pos.yPos < target->pos.yPos + bounds[2] || item->pos.yPos > target->pos.yPos + bounds[3])
				continue;

			// get vector from target to bolt and check against x,z bounds
			__int16 c = COS(target->pos.yRot);
			__int16 s = SIN(target->pos.yRot);

			__int32 x = item->pos.xPos - target->pos.xPos;
			__int32 z = item->pos.zPos - target->pos.zPos;
			__int32 rx = (c * x - s * z) >> W2V_SHIFT;

			__int32 ox = oldX - target->pos.xPos;
			__int32 oz = oldZ - target->pos.zPos;
			__int32 sx = (c * ox - s * oz) >> W2V_SHIFT;

			if ((rx < bounds[0] && sx < bounds[0]) || (rx > bounds[1] && sx > bounds[1]))
				continue;

			__int32 rz = (c * z + s * x) >> W2V_SHIFT;
			__int32 sz = (c * oz + s * ox) >> W2V_SHIFT;

			if ((rz < bounds[4] && sz < bounds[4]) || (rz > bounds[5] && sz > bounds[5]))
				continue;

			// TODO:
			/*if (target->objectNumber == SMASH_OBJECT1 && CurrentLevel != LV_CRASH)
			{
				SmashWindow(targetItemNumber);
			}
			else if (target->objectNumber == SMASH_WINDOW ||
				target->objectNumber == SMASH_OBJECT2 ||
				target->objectNumber == SMASH_OBJECT3)
			{
				SmashWindow(targetItemNumber);
			}
			else if (target->objectNumber == CARCASS || target->objectNumber == EXTRAFX6)
			{
				if (item->status != ACTIVE)
				{
					item->status = ACTIVE;
					AddActiveItem(targetItemNumber);
				}
			}
			else if (target->objectNumber != SMASH_OBJECT1)
			{*/
			if (Objects[target->objectNumber].intelligent)
			{
				DoLotsOfBlood(item->pos.xPos, item->pos.yPos, item->pos.zPos, 0, 0, item->roomNumber, 3);
				HitTarget(target, NULL, Weapons[WEAPON_HARPOON].damage << item->itemFlags[0], 0);
				Savegame.Level.AmmoHits++;
				Savegame.Game.AmmoHits++;
			}

			KillItem(itemNumber);
			item->afterDeath = 0;
			return;
		}		
	}

	// Has harpoon hit a wall?
	if (item->pos.yPos >= item->floor || item->pos.yPos <= GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos))
	{
		if (item->hitPoints == HARPOON_TIME)
		{
			item->currentAnimState = item->pos.xRot;
		}

		if (item->hitPoints >= 192)
		{
			item->pos.xRot = item->currentAnimState + ((((rcossin_tbl[((item->hitPoints - 192) << 9) & 4095] >> 1) - 1024)*(item->hitPoints - 192)) >> 6);
			item->hitPoints--;
		}

		item->hitPoints--;
		if (item->hitPoints <= 0)
		{
			KillItem(itemNumber);
			item->afterDeath = 0;
			return;
		}
		item->speed = item->fallspeed = 0;
	}
	else
	{
		item->pos.zRot += ANGLE(35);
		if (!(Rooms[item->roomNumber].flags & 1))
		{
			item->pos.xRot -= ANGLE(1);
			if (item->pos.xRot < -16384)
				item->pos.xRot = -16384;
			item->fallspeed = (__int16)(-HARPOON_SPEED * SIN(item->pos.xRot) >> W2V_SHIFT);
			item->speed = (__int16)(HARPOON_SPEED * COS(item->pos.xRot) >> W2V_SHIFT);
		}
		else
		{
			// Create bubbles
			if ((Wibble & 15) == 0)
				CreateBubble(&item->pos, item->roomNumber, 0);
			//TriggerRocketSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, 64);
			item->fallspeed = (__int16)(-(HARPOON_SPEED >> 1) * SIN(item->pos.xRot) >> W2V_SHIFT);
			item->speed = (__int16)((HARPOON_SPEED >> 1) * COS(item->pos.xRot) >> W2V_SHIFT);
		}
	}


	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);
}

long	tbx, tby, tbz;

void __cdecl FireGrenade()
{
	__int32 x, y, z;

	if (g_LaraExtra.numGrenadeAmmos <= 0)
		return;

	Lara.hasFired = true;

	/* Create a grenade object and launch it on its way */
	__int16 itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &Items[itemNumber];

		item->shade = 0x4210 | 0x8000;

		item->objectNumber = ID_GRENADE;
		item->roomNumber = LaraItem->roomNumber;
		
		D3DXVECTOR3 pos;
		pos.x = 0;
		pos.y = -(GRENADE_YOFF + 96);
		pos.z = GRENADE_ZOFF;
		g_Renderer->GetLaraBonePosition(&pos, HAND_R);

		item->pos.xPos = x = pos.x;
		item->pos.yPos = y = pos.y;
		item->pos.zPos = z = pos.z;
		FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &item->roomNumber);
		if (GetFloorHeight(floor, pos.x, pos.y, pos.z) < pos.y)
		{
			item->pos.xPos = LaraItem->pos.xPos;
			item->pos.yPos = pos.y;
			item->pos.zPos = LaraItem->pos.zPos;
			item->roomNumber = LaraItem->roomNumber;
		} 

		pos.x = 0;
		pos.y = -(GRENADE_YOFF + 1024);
		pos.z = GRENADE_ZOFF;

		g_Renderer->GetLaraBonePosition(&pos, HAND_R);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_GRENADE;

		for (__int32 i = 0; i < 5; i++)
			TriggerGunSmoke(x, y, z, pos.x - x, pos.y - y, pos.z - z, 1, WEAPON_GRENADE, 32);

		InitialiseItem(itemNumber);

		item->pos.xRot = LaraItem->pos.xRot + Lara.leftArm.xRot;
		item->pos.yRot = LaraItem->pos.yRot + Lara.leftArm.yRot;
		item->pos.zRot = 0;

		if (!Lara.leftArm.lock)
		{
			item->pos.xRot += Lara.torsoXrot;
			item->pos.yRot += Lara.torsoYrot;
		}

		item->speed = GRENADE_SPEED;
		item->fallspeed = (-item->speed * SIN(item->pos.xRot)) >> W2V_SHIFT;
		item->currentAnimState = item->pos.xRot;	
		item->goalAnimState = item->pos.yRot;	// Goal anim state is Y rotation so the object can rotate in Y axis blah blah
		item->requiredAnimState = 0;	// Flag to say if rolling on floor.
		item->hitPoints = 4 * 30;	// Explode after 3 seconds.

		AddActiveItem(itemNumber);

		Savegame.Level.AmmoUsed++;
		Savegame.Game.AmmoUsed++;
	}
}

#define GRENADE_BLAST_RADIUS (WALL_SIZE)

void __cdecl ControlGrenade(__int16 itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	__int32 oldX = item->pos.xPos;
	__int32 oldY = item->pos.yPos;
	__int32 oldZ = item->pos.zPos;

	__int32 xv;
	__int32 yv;
	__int32 zv;

	item->shade = 0x4210 | 0x8000;
	// Fall more slowly in water.

	bool aboveWater = false;
	if (Rooms[item->roomNumber].flags & 1)
	{
		aboveWater = false;
		item->fallspeed += (5 - item->fallspeed) >> 1;
		item->speed -= item->speed >> 2;
		if (item->speed)
		{
			item->pos.zRot += (((item->speed >> 4) + 3)*ONE_DEGREE);
			if (item->requiredAnimState)
				item->pos.yRot += (((item->speed >> 2) + 3)*ONE_DEGREE);
			else
				item->pos.xRot += (((item->speed >> 2) + 3)*ONE_DEGREE);
		}
	}
	else
	{
		aboveWater = true;
		item->fallspeed += GRAVITY >> 1;
		if (item->speed)
		{
			item->pos.zRot += (((item->speed >> 2) + 7)*ONE_DEGREE);
			if (item->requiredAnimState)
				item->pos.yRot += (((item->speed >> 1) + 7)*ONE_DEGREE);
			else
				item->pos.xRot += (((item->speed >> 1) + 7)*ONE_DEGREE);

		}
	}

	/*phd_PushUnitMatrix();

	*(phd_mxptr + M03) = 0;
	*(phd_mxptr + M13) = 0;
	*(phd_mxptr + M23) = 0;

	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);
	phd_TranslateRel(0, 0, -64);*/
	/*#
		wx = (*(phd_mxptr + M03) >> W2V_SHIFT);
		wy = (*(phd_mxptr + M13) >> W2V_SHIFT);
		wz = (*(phd_mxptr + M23) >> W2V_SHIFT);
	endif
		phd_PopMatrix();*/

	D3DXMATRIX transform;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;

	D3DXMatrixRotationYawPitchRoll(&rotation, TR_ANGLE_TO_RAD(item->pos.yRot), TR_ANGLE_TO_RAD(item->pos.xRot),
		TR_ANGLE_TO_RAD(item->pos.zRot));
	D3DXMatrixTranslation(&translation, 0, 0, -64);
	D3DXMatrixMultiply(&transform, &rotation, &translation);

	__int32 wx = transform._14;
	__int32 wy = transform._24;
	__int32 wz = transform._34;

	if (item->speed && aboveWater)
		TriggerRocketSmoke(wx + item->pos.xPos, wy + item->pos.yPos, wz + item->pos.zPos, -1);

	xv = ((item->speed * SIN(item->goalAnimState)) >> W2V_SHIFT);
	yv = item->fallspeed;
	zv = ((item->speed * COS(item->goalAnimState)) >> W2V_SHIFT);

	item->pos.xPos += xv;
	item->pos.yPos += yv;
	item->pos.zPos += zv;

	__int16 sYrot = item->pos.yRot;
	item->pos.yRot = item->goalAnimState;

	DoProperDetection(itemNumber, oldX, oldY, oldZ, xv, yv, zv);

	item->goalAnimState = item->pos.yRot;
	item->pos.yRot = sYrot;

	/*if (Rooms[item->roomNumber].flags & 1 && abovewater)
	{
		splash_setup.x = item->pos.xPos;
		splash_setup.y = room[item->roomNumber].maxceiling;
		splash_setup.z = item->pos.zPos;
		splash_setup.InnerXZoff = 16;
		splash_setup.InnerXZsize = 12;
		splash_setup.InnerYsize = -96;
		splash_setup.InnerXZvel = 0xa0;
		splash_setup.InnerYvel = -(item->fallspeed << 5) - (64 << 5);
		splash_setup.InnerGravity = 0x80;
		splash_setup.InnerFriction = 7;
		splash_setup.MiddleXZoff = 24;
		splash_setup.MiddleXZsize = 24;
		splash_setup.MiddleYsize = -64;
		splash_setup.MiddleXZvel = 0xe0;
		splash_setup.MiddleYvel = -(item->fallspeed << 4) - (64 << 4);
		splash_setup.MiddleGravity = 0x48;
		splash_setup.MiddleFriction = 8;
		splash_setup.OuterXZoff = 32;
		splash_setup.OuterXZsize = 32;
		splash_setup.OuterXZvel = 0x110;
		splash_setup.OuterFriction = 9;
		SetupSplash(&splash_setup);
	}*/

	// Has the timer ran out ? If so, explodes with blast radius.

	__int32 radius = 0;
	bool explode = false; 

	if (item->hitPoints)
	{
		item->hitPoints--;
		if (!item->hitPoints)
		{
			radius = GRENADE_BLAST_RADIUS;
			explode = 1;
		}
	}

	// Get all surrounding rooms inside the radius
	ROOM_INFO* room = &Rooms[item->roomNumber];

	vector<__int32> items;
	items.push_back(Lara.itemNumber);

	if (room->door)
	{
		__int32 numPortals = *(room->door);
		__int16* portals = room->door;
		portals++;

		for (__int32 i = 0; i < numPortals; i++)
		{
			__int16 roomNumber = *portals;

			room = &Rooms[roomNumber];
			ITEM_INFO* target = NULL;

			for (__int16 targetItemNumber = room->itemNumber; targetItemNumber != NO_ITEM; targetItemNumber = target->nextItem)
			{
				target = &Items[targetItemNumber];

				// Ignore not collidable items
				if (target != LaraItem && !target->collidable)
					continue;

				items.push_back(targetItemNumber);
			}

			portals += 16;
		}
	}

	for (__int32 i = 0; i < items.size(); i++)
	{
		ITEM_INFO* target = &Items[items[i]];

		// Destroy only smashable objects, shatters (check later), Lara and intelligent entities
		if (target->objectNumber == ID_SMASH_OBJECT1 ||
			target->objectNumber == ID_SMASH_OBJECT2 ||
			target->objectNumber == ID_SMASH_OBJECT3 ||
			target->objectNumber == ID_SMASH_OBJECT4 ||
			target->objectNumber == ID_SMASH_OBJECT5 ||
			target->objectNumber == ID_SMASH_OBJECT6 ||
			target->objectNumber == ID_SMASH_OBJECT7 ||
			target->objectNumber == ID_SMASH_OBJECT8 ||
			target->objectNumber == ID_LARA ||
			(Objects[target->objectNumber].intelligent &&
				target->status != ITEM_INVISIBLE &&
				Objects[target->objectNumber].collision)
			)
		{
			__int16* bounds = GetBestFrame(target);
			if (item->pos.yPos + radius < target->pos.yPos + bounds[2] || item->pos.yPos - radius > target->pos.yPos + bounds[3])
				continue;

			// get vector from target to bolt and check against x,z bounds
			__int32 c = COS(target->pos.yRot);
			__int32 s = SIN(target->pos.yRot);

			__int32 x = item->pos.xPos - target->pos.xPos;
			__int32 z = item->pos.zPos - target->pos.zPos;
			__int32 rx = (c*x - s * z) >> W2V_SHIFT;

			__int32 ox = oldX - target->pos.xPos;
			__int32 oz = oldZ - target->pos.zPos;
			__int32 sx = (c*ox - s * oz) >> W2V_SHIFT;

			if ((rx + radius < bounds[0] && sx + radius < bounds[0]) ||
				(rx - radius > bounds[1] && sx - radius > bounds[1]))
				continue;

			__int32 rz = (c*z + s * x) >> W2V_SHIFT;
			__int32 sz = (c*oz + s * ox) >> W2V_SHIFT;

			if ((rz + radius < bounds[4] && sz + radius < bounds[4]) ||
				(rz - radius > bounds[5] && sz - radius > bounds[5]))
				continue;

			if (target->objectNumber == ID_SMASH_OBJECT1 ||
				target->objectNumber == ID_SMASH_OBJECT2 ||
				target->objectNumber == ID_SMASH_OBJECT3 ||
				target->objectNumber == ID_SMASH_OBJECT4 ||
				target->objectNumber == ID_SMASH_OBJECT5 ||
				target->objectNumber == ID_SMASH_OBJECT6 ||
				target->objectNumber == ID_SMASH_OBJECT7 ||
				target->objectNumber == ID_SMASH_OBJECT8)
			{
				SmashItem(items[i]);
			}
			else
			{
				// Hit the target
				HitTarget(target, NULL, 500, 0);

				// Update level statistics
				Savegame.Level.AmmoHits++;
				Savegame.Game.AmmoHits++;

				// Check if entity is dead
				if (target->hitPoints <= 0)
				{
					Savegame.Level.Kills++;
					Savegame.Game.Kills++;

					if (target->objectNumber != ID_LARA)
						CreatureDie(items[i], 1);
					else
					{
						LaraItem->hitPoints = -16384;
						LaraItem->hitStatus = true;
					}
				}

				if (!explode)
				{
					explode = true;
					radius = GRENADE_BLAST_RADIUS;
					break;
				}
			}
		}

	}

	if (explode)
	{
		if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)	// Just broken water surface ?
			TriggerUnderwaterExplosion(item);
		else
		{
			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->roomNumber);	// -2 = Set off a dynamic light controller.
			for (__int32 x = 0; x < 2; x++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->roomNumber);
		}

		AlertNearbyGuards(item);
		SoundEffect(105, &item->pos, ENV_FLAG_PITCH_SHIFT | 0x1800000);
		SoundEffect(106, &item->pos, 0);	// Explosion ?
		if (itemNumber != Lara.itemNumber)
			KillItem(itemNumber);
	}
}

void __cdecl DrawShotgun(__int32 weaponType)
{
	ITEM_INFO* item;

	if (Lara.weaponItem == NO_ITEM)
	{
		Lara.weaponItem = CreateItem();
		
		item = &Items[Lara.weaponItem];

		item->objectNumber = WeaponObject(weaponType);

		if (weaponType == WEAPON_ROCKET)
			item->animNumber = Objects[item->objectNumber].animIndex + 1;
		else if (weaponType == WEAPON_GRENADE)
			item->animNumber = Objects[item->objectNumber].animIndex + ROCKET_DRAW_ANIM;
		else
			item->animNumber = Objects[item->objectNumber].animIndex + HARPOON_DRAW_ANIM; // M16 too
		
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 1;
		item->currentAnimState = 1;
		item->status = ITEM_ACTIVE;
		item->roomNumber = 255;
		
		Lara.rightArm.frameBase = Objects[item->objectNumber].frameBase;
		Lara.leftArm.frameBase = Lara.rightArm.frameBase;
	}
	else
	{
		item = &Items[Lara.weaponItem];
	}

	AnimateItem(item);

	if (item->currentAnimState != 0 && item->currentAnimState != 6)
	{
		if (item->frameNumber - Anims[item->animNumber].frameBase == Weapons[weaponType].drawFrame)
		{
			DrawShotgunMeshes(weaponType);
		}
		else if (Lara.waterStatus == 1)
		{
			item->goalAnimState = 6;
		}
	}
	else
	{
		ReadyShotgun(weaponType);
	}

	Lara.leftArm.frameBase = Lara.rightArm.frameBase = Anims[item->animNumber].framePtr;
	Lara.leftArm.frameNumber = Lara.rightArm.frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;
	Lara.leftArm.animNumber = Lara.rightArm.animNumber = item->animNumber;
}

void __cdecl AnimateShotgun(__int32 weaponType)
{
	bool harpoonFired = false;

	if (HKTimer)
	{
		HKFlag = 0;
		--HKTimer;
	}

	if (SmokeCountL)
	{
		PHD_VECTOR pos;

		if (SmokeWeapon == WEAPON_HK)
		{
			pos.x = 0;
			pos.y = 228;
			pos.z = 96;
		}
		else if (SmokeWeapon == WEAPON_SHOTGUN)
		{
			pos.x = -16;
			pos.y = 228;
			pos.z = 32;
		}
		else if (SmokeWeapon == WEAPON_GRENADE)
		{
			pos.x = 0;
			pos.y = 180;
			pos.z = 80;
		}
		else if (SmokeWeapon == WEAPON_ROCKET)
		{
			pos.x = 0;
			pos.y = 84;
			pos.z = 72;
		}

		GetLaraJointPosition(&pos, UARM_L);

		if (LaraItem->meshBits)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
	}

	ITEM_INFO* item = &Items[Lara.weaponItem];
	bool running = (weaponType == WEAPON_HK && LaraItem->speed != 0);

	switch (item->currentAnimState)
	{
	case 0:
		HKFlag = 0;
		HKTimer = 0;
		HKFlag2 = 0;

		if (Lara.waterStatus == 1 || running)
			item->goalAnimState = 6;
		else if ((!(TrInput & IN_ACTION) || Lara.target) && Lara.leftArm.lock == 0)
			item->goalAnimState = 4;
		else
			item->goalAnimState = 2;

		break;

	case 6:
		HKFlag = 0;
		HKTimer = 0;
		HKFlag2 = 0;

		if (Lara.waterStatus == 1 || running)
		{
			if ((!(TrInput & IN_ACTION) || Lara.target) && Lara.leftArm.lock == 0)
				item->goalAnimState = 7;
			else
				item->goalAnimState = 8;
		}
		else
			item->goalAnimState = 0;
		
		break;

	case 2:
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			item->goalAnimState = 4;
			
			if (Lara.waterStatus != 1 && !running && !harpoonFired)
			{
				if ((TrInput & IN_ACTION) && (!Lara.target || Lara.leftArm.lock))
				{
					if (weaponType == WEAPON_HARPOON)
					{
						FireHarpoon();
						if (!(g_LaraExtra.harpoonAmmo & 3)) 
							harpoonFired = true;
					}
					else if (weaponType == WEAPON_ROCKET)
					{
						//FireRocket();
					}
					else if (weaponType == WEAPON_GRENADE)
					{
						FireGrenade();
					}
					else if (weaponType == WEAPON_CROSSBOW)
					{
						FireCrossbow(NULL);
					}
					else if (weaponType == WEAPON_HK)
					{
						FireHK(0);
						HKFlag = 1;

						if (Lara.HKtypeCarried & 2)
						{
							SoundEffect(14, 0, 0);
						}
						else
						{
							SoundEffect(105, &LaraItem->pos, 83888140);
							SoundEffect(68, &LaraItem->pos, 0);
						}
					}
					else
						FireShotgun();

					item->goalAnimState = 2;
				}
				else if (Lara.leftArm.lock)
					item->goalAnimState = 0; 
			}

			if (item->goalAnimState != 2 && HKFlag && !(Lara.HKtypeCarried & 2))
			{
				StopSoundEffect(68);
				SoundEffect(69, &LaraItem->pos, 0);
				HKFlag = 0;
			}
		}
		else if (HKFlag)
		{
			if (Lara.HKtypeCarried & 2)
			{
				SoundEffect(14, 0, 0);
			}
			else
			{
				SoundEffect(105, &LaraItem->pos, 83888140);
				SoundEffect(68, &LaraItem->pos, 0);
			}
		}
		else if (weaponType == WEAPON_SHOTGUN && !(TrInput & IN_ACTION) && !Lara.leftArm.lock)
		{
			item->goalAnimState = 4;
		}

		if (item->frameNumber - Anims[item->animNumber].frameBase == 12 && weaponType == WEAPON_SHOTGUN)
			TriggerGunShell(1, 370, 4);
		break;

	case 8:
		if (item->frameNumber - Anims[item->animNumber].frameBase == 0)
		{
			item->goalAnimState = 7;

			if ((Lara.waterStatus == 1 || running) && !harpoonFired)
			{
				if ((TrInput & IN_ACTION) && (!Lara.target || Lara.leftArm.lock))
				{
					if (weaponType == WEAPON_HARPOON)
					{
						FireHarpoon();
						if (!(g_LaraExtra.harpoonAmmo & 3))
							harpoonFired = true;
					}
					else if (weaponType == WEAPON_HK && (!(Lara.HKtypeCarried & 0x18) || !HKTimer))
					{
						FireHK(1);
						HKFlag = 1;
						item->goalAnimState = 8;
						if (Lara.HKtypeCarried & 2)
						{
							SoundEffect(14, 0, 0);
						}
						else
						{
							SoundEffect(105, &LaraItem->pos, 83888140);
							SoundEffect(68, &LaraItem->pos, 0);
						}
					}
					else
					{
						item->goalAnimState = 6;
					}

					item->goalAnimState = 8;
				}
				else if (Lara.leftArm.lock)
					item->goalAnimState = 6;
			}
			else if (item->goalAnimState != 8 && HKFlag && !(Lara.HKtypeCarried & 2))
			{
				StopSoundEffect(68);
				SoundEffect(69, &LaraItem->pos, 0);
				HKFlag = 0;
			}
			else if (HKFlag)
			{
				if (Lara.HKtypeCarried & 2)
				{
					SoundEffect(14, 0, 0);
				}
				else
				{
					SoundEffect(105, &LaraItem->pos, 83888140);
					SoundEffect(68, &LaraItem->pos, 0);
				}
			}
		}		

		break;

	default:
		break;
	}

	AnimateItem(item);

	Lara.leftArm.frameBase = Lara.rightArm.frameBase = Anims[item->animNumber].framePtr;
	Lara.leftArm.frameNumber = Lara.rightArm.frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;
	Lara.leftArm.animNumber = Lara.rightArm.animNumber = item->animNumber;
}

void __cdecl ControlCrossbowBolt(__int16 itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	__int32 oldX = item->pos.xPos;
	__int32 oldY = item->pos.yPos;
	__int32 oldZ = item->pos.zPos;
	__int16 roomNumber = item->roomNumber;

	bool land = false;
	bool explode = false;

	if (Rooms[roomNumber].flags & ENV_FLAG_WATER)
	{
		if (item->speed > 64)
			item->speed -= (item->speed >> 4);
		if (GlobalCounter & 1)
			CreateBubble(&item->pos, roomNumber, 4, 7);
	}
	else
	{
		land = true;
	}

	item->pos.xPos += ((item->speed * COS(item->pos.xRot) >> W2V_SHIFT) * SIN(item->pos.yRot)) >> W2V_SHIFT;
	item->pos.yPos += item->speed * SIN(-item->pos.xRot) >> W2V_SHIFT;
	item->pos.zPos += ((item->speed * COS(item->pos.xRot) >> W2V_SHIFT) * COS(item->pos.yRot)) >> W2V_SHIFT;

	roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos ||
		GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos)
	{
		// I have hit the room, this is the end for the bolt
		item->pos.xPos = oldX;
		item->pos.yPos = oldY;
		item->pos.zPos = oldZ;

		// If ammos are normal, then just shatter the bolt and quit
		if (item->itemFlags[0] != 2)
		{
			ExplodeItemNode(item, 0, 0, 256);
			KillItem(itemNumber);
			return;
		}

		// Otherwise, bolt must explode
		explode = true;
	}

	// Has bolt changed room?
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((Rooms[item->roomNumber].flags & ENV_FLAG_WATER) && land)
	{
		SetupRipple(item->pos.xPos, Rooms[item->roomNumber].minfloor, item->pos.zPos, (GetRandomControl() & 7) + 8, 0);
	}

	__int32 radius = (explode ? CROSSBOW_EXPLODE_RADIUS : CROSSBOW_HIT_RADIUS);

	__int32 n = 0;
	bool foundCollidedObjects = false;

	do
	{
		// Found possible collided items and statics
		GetCollidedObjects(item, radius, 1, &CollidedItems[0], &CollidedMeshes[0], 1);
		
		// If no collided items and meshes are found, then exit the loop
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		foundCollidedObjects = true;

		if (item->itemFlags[0] != 3 || explode)
		{
			if (CollidedItems[0])
			{
				ITEM_INFO* currentItem = CollidedItems[0];
				__int32 k = 0;

				do
				{
					if (explode)
					{
						// Item is inside the radius and must explode
						if (item->objectNumber < ID_SMASH_OBJECT1 || item->objectNumber > ID_SMASH_OBJECT8)
						{
							if (currentItem->objectNumber == ID_SWITCH_TYPE7 || currentItem->objectNumber == ID_SWITCH_TYPE8)
								CrossbowHitSwitchType78(item, currentItem, 0);
							else if (Objects[item->objectNumber].hitEffect)
								DoGrenadeDamageOnBaddie(currentItem, item);
						}
						else
						{
							TriggerExplosionSparks(currentItem->pos.xPos, currentItem->pos.yPos, currentItem->pos.zPos, 3, -2, 0, currentItem->roomNumber);
							currentItem->pos.yPos -= 128;
							TriggerShockwave(&currentItem->pos, 19922992, 96, 411066368, 0, 0);
							currentItem->pos.yPos += 128;
							ExplodeItemNode(currentItem, 0, 0, 128);
							__int16 currentItemNumber = (currentItem - CollidedItems[0]) / sizeof(ITEM_INFO);
							SmashObject(currentItemNumber);
							KillItem(currentItemNumber);
						}
					}
					else if (currentItem->objectNumber == ID_SWITCH_TYPE7 || currentItem->objectNumber == ID_SWITCH_TYPE8)
					{
						// Special case for ID_SWITCH_TYPE7 and ID_SWITCH_TYPE8
						CrossbowHitSwitchType78(item, currentItem, 1);
					}
					else if (Objects[currentItem->objectNumber].hitEffect)
					{
						HitTarget(currentItem, (GAME_VECTOR*)&item->pos, Weapons[WEAPON_CROSSBOW].damage, 0);

						// Enable item if hit but only with normal ammos
						if (item->itemFlags[0] == 1 && !Objects[currentItem->objectNumber].explodableMeshbits)  
							item->active = true;
					}

					k++;
					currentItem = CollidedItems[k];

				} while (currentItem);
			}

			if (CollidedMeshes[0])
			{
				MESH_INFO* currentMesh = CollidedMeshes[0];
				__int32 k = 0;

				do
				{
					if (currentMesh->staticNumber >= 50 && currentMesh->staticNumber < 58)
					{
						if (explode)
						{
							TriggerExplosionSparks(currentMesh->x, currentMesh->y, currentMesh->z, 3, -2, 0, item->roomNumber);
							currentMesh->y -= 128;
							TriggerShockwave((PHD_3DPOS*)&currentMesh, 0xB00028, 64, 0x10806000, 0, 0);
							currentMesh->y += 128;
						}
						ShatterObject((SHATTER_ITEM*)item, NULL, -128, item->roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->roomNumber;
						SmashedMesh[SmashedMeshCount] = currentMesh;
						SmashedMeshCount++;
						currentMesh->Flags &= ~1;
					}

					k++;
					currentMesh = CollidedMeshes[k];

				} while (currentMesh);
			}

			break;
		}

		n++;
		explode = true;
		radius = CROSSBOW_EXPLODE_RADIUS;
	} while (n < 2);
		
	if (!explode)
	{
		if (foundCollidedObjects)
		{
			ExplodeItemNode(item, 0, 0, 256);
			KillItem(itemNumber);
		}
		return;
	}

	// At this point, for sure bolt must explode

	if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		TriggerUnderwaterExplosion(item);
	else
	{
		TriggerShockwave(&item->pos, 19922992, 96, 411066368, 0, 0);
		item->pos.yPos += 128;
		TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->roomNumber);
		for (__int32 j = 0; j < 2; j++)
			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->roomNumber);
	}

	AlertNearbyGuards(item);
	
	SoundEffect(105, &item->pos, 0x1800004);
	SoundEffect(106, &item->pos, 0);

	if (foundCollidedObjects)
	{
		ExplodeItemNode(item, 0, 0, 256);
		KillItem(itemNumber);
	}

	return;
}

void __cdecl RifleHandler(__int32 weaponType)
{
	WEAPON_INFO* weapon = &Weapons[weaponType];

	LaraGetNewTarget(weapon);

	if (TrInput & IN_ACTION)
		LaraTargetInfo(weapon);

	AimWeapon(weapon, &Lara.leftArm);

	if (Lara.leftArm.lock)
	{
		Lara.torsoXrot = Lara.leftArm.xRot;
		Lara.torsoYrot = Lara.leftArm.yRot;
		if (Camera.oldType != CAMERA_TYPE::LOOK_CAMERA && !BinocularRange)
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
		}
	}

	if (weaponType == WEAPON_REVOLVER)
		AnimatePistols(WEAPON_REVOLVER);
	else
		AnimateShotgun(weaponType);

	if (Lara.rightArm.flash_gun)
	{
		if (weaponType == WEAPON_SHOTGUN || weaponType == WEAPON_HK)
		{
			TriggerDynamics(
				LaraItem->pos.xPos + (SIN(LaraItem->pos.yRot) >> 4) + GetRandomControl() - 128,
				LaraItem->pos.yPos + (GetRandomControl() & 0x7F) - 575,
				LaraItem->pos.zPos + (COS(LaraItem->pos.yRot) >> 4) + GetRandomControl() - 128,
				12,
				(GetRandomControl() & 0x3F) + 192,
				(GetRandomControl() & 0x1F) + 128,
				GetRandomControl() & 0x3F
			);
		}
		else if (weaponType == WEAPON_REVOLVER)
		{
			PHD_VECTOR pos;

			pos.x = GetRandomControl() - 128;
			pos.y = (GetRandomControl() & 0x7F) - 63;
			pos.z = GetRandomControl() - 128;

			GetLaraJointPosition(&pos, 11);

			TriggerDynamics(pos.x, pos.y, pos.z, 12,
				(GetRandomControl() & 0x3F) + 192,
				(GetRandomControl() & 0x1F) + 128,
				(GetRandomControl() & 0x3F));
		}
	}
}

void __cdecl FireCrossbow(PHD_3DPOS* pos)
{
	__int16* ammos = GetAmmo(WEAPON_CROSSBOW);
	if (*ammos <= 0)
		return;

	Lara.hasFired = true;
	
	__int16 itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &Items[itemNumber];
		item->objectNumber = ID_CROSSBOW_BOLT;
		item->shade = 0xC210;
		if (pos)
		{
			item->roomNumber = LaraItem->roomNumber;
			item->pos.xPos = pos->xPos;
			item->pos.yPos = pos->yPos;
			item->pos.zPos = pos->zPos;

			InitialiseItem(itemNumber);

			item->pos.xRot = pos->xRot;
			item->pos.yRot = pos->yRot;
			item->pos.zRot = pos->zRot;
		}
		else
		{
			if (*ammos != -1)
				(*ammos)--;

			PHD_VECTOR jointPos;
			jointPos.x = 0;
			jointPos.y = 228;
			jointPos.z = 32;

			GetLaraJointPosition(&jointPos, 11);

			item->roomNumber = LaraItem->roomNumber;
			
			FLOOR_INFO* floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->roomNumber);
			__int32 height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);

			if (height >= jointPos.y)
			{
				item->pos.xPos = jointPos.x;
				item->pos.yPos = jointPos.y;
				item->pos.zPos = jointPos.z;
			}
			else
			{
				item->pos.xPos = LaraItem->pos.xPos;
				item->pos.yPos = jointPos.y;
				item->pos.zPos = LaraItem->pos.zPos;
				item->roomNumber = LaraItem->roomNumber;
			}

			InitialiseItem(itemNumber);

			item->pos.xRot = Lara.leftArm.xRot + LaraItem->pos.xRot;
			item->pos.zRot = 0;
			item->pos.yRot = Lara.leftArm.yRot + LaraItem->pos.yRot;

			if (!Lara.leftArm.lock)
			{
				item->pos.xRot += Lara.torsoXrot;
				item->pos.yRot += Lara.torsoYrot;
			}
		}

		item->speed = 512;

		AddActiveItem(itemNumber);
		
		if (Lara.crossbowTypeCarried & 0x08)
		{
			item->itemFlags[0] = 1;
		}
		else if (Lara.crossbowTypeCarried & 0x10)
		{
			item->itemFlags[0] = 2;
		}
		else
		{
			item->itemFlags[0] = 3;
		}

		SoundEffect(235, 0, 0);

		Savegame.Level.AmmoUsed++;
		Savegame.Game.AmmoUsed++;
	}
}

void __cdecl Inject_Lara1Gun()
{
	INJECT(0x0044EAC0, DrawShotgun);
	INJECT(0x0044EE00, AnimateShotgun);
	INJECT(0x0044DCC0, RifleHandler);
	INJECT(0x00429ED0, FireCrossbow)
}