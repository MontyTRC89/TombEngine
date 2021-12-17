#include "framework.h"
#include "lara_one_gun.h"
#include "items.h"
#include "Lara.h"
#include "lara_fire.h"
#include "animation.h"
#include "control/box.h"
#include "control/control.h"
#include "effects/effects.h"
#include "effects/tomb4fx.h"
#include "collide.h"
#include "effects/debris.h"
#include "effects/weather.h"
#include "lara_two_guns.h"
#include "objects.h"
#include "camera.h"
#include "level.h"
#include "setup.h"
#include "input.h"
#include "savegame.h"
#include "Sound/sound.h"
#include "effects/bubble.h"
#include "generic_switch.h"
#include "Game/effects/lara_fx.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Switches;
using namespace TEN::Effects::Environment;

//int HKCounter = 0;
//int HKTimer = 0;
//int HKFlag = 0;
//byte HKFlag2 = 0;

void FireHarpoon()
{
	Ammo& ammos = GetAmmo(LaraItem, WEAPON_HARPOON_GUN);
	if (!ammos)
		return;

	Lara.hasFired = true;

	// Create a new item for harpoon
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		if (!ammos.hasInfinite())
			(ammos)--;

		GAME_VECTOR pos;
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->shade = 0x4210 | 0x8000;
		item->objectNumber = ID_HARPOON;
		item->roomNumber = LaraItem->roomNumber;

		PHD_VECTOR jointPos;
		
		jointPos.x = -2;
		jointPos.y = 273 + 100;
		jointPos.z = 77;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		FLOOR_INFO* floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->roomNumber);
		int height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);

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

		item->pos.zRot = 0;

		item->fallspeed = -HARPOON_SPEED * phd_sin(item->pos.xRot);
		item->speed = HARPOON_SPEED * phd_cos(item->pos.xRot);
		item->hitPoints = HARPOON_TIME;

		AddActiveItem(itemNumber);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void HarpoonBoltControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	// Store old position for later
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;
	short roomNumber = item->roomNumber;

	bool aboveWater = false;

	// Update speed and check if above water

	if (item->hitPoints == HARPOON_TIME)
	{
		item->pos.zRot += ANGLE(35);
		if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER))
		{
			item->pos.xRot -= ANGLE(1);
			if (item->pos.xRot < -ANGLE(90))
				item->pos.xRot = -ANGLE(90);
			item->fallspeed = -HARPOON_SPEED * phd_sin(item->pos.xRot);
			item->speed = HARPOON_SPEED * phd_cos(item->pos.xRot);
			aboveWater = true;
		}
		else
		{
			// Create bubbles
			if ((Wibble & 15) == 0)
				CreateBubble((PHD_VECTOR*)&item->pos, item->roomNumber, 0, 0, BUBBLE_FLAG_CLUMP | BUBBLE_FLAG_HIGH_AMPLITUDE, 0, 0, 0); // CHECK
			TriggerRocketSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, 64);
			item->fallspeed = -HARPOON_SPEED * phd_sin(item->pos.xRot) / 2;
			item->speed = HARPOON_SPEED * phd_cos(item->pos.xRot) / 2;
			aboveWater = false;
		}

		// Update bolt's position
		item->pos.xPos += item->speed * phd_cos(item->pos.xRot) * phd_sin(item->pos.yRot);
		item->pos.yPos += item->speed * phd_sin(-item->pos.xRot);
		item->pos.zPos += item->speed * phd_cos(item->pos.xRot) * phd_cos(item->pos.yRot);
	}
	else
	{
		if (item->hitPoints > 0)
			item->hitPoints--;
		else
			KillItem(itemNumber);
		return;
	}

	roomNumber = item->roomNumber;
	FLOOR_INFO * floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	// Check if bolt has hit a solid wall
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos ||
		GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos)
	{
		// I have hit a solid wall, this is the end for the bolt
		item->hitPoints--;
		return;
	}

	// Has harpoon changed room?
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER) && aboveWater)
	{
		SetupRipple(item->pos.xPos, g_Level.Rooms[item->roomNumber].minfloor, item->pos.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
	}

	int n = 0;
	bool foundCollidedObjects = false;

	// Found possible collided items and statics
	GetCollidedObjects(item, HARPOON_HIT_RADIUS, 1, &CollidedItems[0], &CollidedMeshes[0], 1);

	// If no collided items and meshes are found, then exit the loop
	if (!CollidedItems[0] && !CollidedMeshes[0])
		return;

	if (CollidedItems[0])
	{
		ITEM_INFO* currentItem = CollidedItems[0];
		
		int k = 0;
		do
		{
			OBJECT_INFO* currentObj = &Objects[currentItem->objectNumber];

			if (!currentObj->isPickup && currentObj->collision && currentItem->collidable)
				foundCollidedObjects = true;

			if (currentObj->intelligent && currentObj->collision && currentItem->status == ITEM_ACTIVE && !currentObj->undead)
				HitTarget(LaraItem, currentItem, (GAME_VECTOR*)&item->pos, Weapons[WEAPON_HARPOON_GUN].damage, 0);

			// All other items (like puzzles) can't be hit
			k++;
			currentItem = CollidedItems[k];

		} while (currentItem);
	}

	if (CollidedMeshes[0])
	{
		MESH_INFO* currentMesh = CollidedMeshes[0];
		int k = 0;

		do
		{
			STATIC_INFO* s = &StaticObjects[currentMesh->staticNumber];
			if (s->shatterType != SHT_NONE)
			{
				currentMesh->hitPoints -= Weapons[WEAPON_CROSSBOW].damage;
				if (currentMesh->hitPoints <= 0)
				{
					TriggerExplosionSparks(currentMesh->pos.xPos, currentMesh->pos.yPos, currentMesh->pos.zPos, 3, -2, 0, item->roomNumber);
					auto pos = PHD_3DPOS(currentMesh->pos.xPos, currentMesh->pos.yPos - 128, currentMesh->pos.zPos, 0, currentMesh->pos.yRot, 0);
					TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
					ShatterObject(NULL, currentMesh, -128, item->roomNumber, 0);
					SmashedMeshRoom[SmashedMeshCount] = item->roomNumber;
					SmashedMesh[SmashedMeshCount] = currentMesh;
					SmashedMeshCount++;
					currentMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
				}
			}

			k++;
			currentMesh = CollidedMeshes[k];

		} while (currentMesh);
	}

	// If harpoon has hit some objects then shatter itself
	if (foundCollidedObjects)
	{
		ExplodeItemNode(item, 0, 0, EXPLODE_NORMAL);
		KillItem(itemNumber);
	}
}

long tbx, tby, tbz;

void FireGrenade()
{
	int x = 0;
	int y = 0;
	int z = 0;
	
	Ammo& ammo = GetAmmo(LaraItem, WEAPON_GRENADE_LAUNCHER);
	if (!ammo)
		return;

	Lara.hasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		
		item->shade = 0xC210;
		item->objectNumber = ID_GRENADE;
		item->roomNumber = LaraItem->roomNumber;

		PHD_VECTOR jointPos;
		jointPos.x = 0;
		jointPos.y = 276;
		jointPos.z = 80;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		item->pos.xPos = x = jointPos.x;
		item->pos.yPos = y = jointPos.y;
		item->pos.zPos = z = jointPos.z;

		FLOOR_INFO* floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->roomNumber);
		int height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);
		if (height < jointPos.y)
		{
			item->pos.xPos = LaraItem->pos.xPos;
			item->pos.yPos = jointPos.y;
			item->pos.zPos = LaraItem->pos.zPos;
			item->roomNumber = LaraItem->roomNumber;
		}

		jointPos.x = 0;
		jointPos.y = 1204;
		jointPos.z = 5;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_GRENADE_LAUNCHER;

		if (LaraItem->meshBits)
		{
			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(x, y, z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 1, WEAPON_GRENADE_LAUNCHER, 32);

		}

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
		item->fallspeed = -512 * phd_sin(item->pos.xRot);
		item->currentAnimState = item->pos.xRot;
		item->goalAnimState = item->pos.yRot;
		item->requiredAnimState = 0;
		item->hitPoints = 120;	
		item->itemFlags[0] = WEAPON_AMMO2;

		AddActiveItem(itemNumber);

		if (!ammo.hasInfinite())
			(ammo)--;

		item->itemFlags[0] = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo;

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void GrenadeControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->itemFlags[1])
	{
		item->itemFlags[1]--;

		if (item->itemFlags[1])
		{
			if (item->itemFlags[0] == GRENADE_FLASH)
			{
				// Flash grenades
				int R, G, B;
				if (item->itemFlags[1] == 1)
				{
					WeaponEnemyTimer = 120;
					R = 255;
					G = 255;
					B = 255;
				}
				else
				{
					R = (GetRandomControl() & 0x1F) + 224;
					G = B = R - GetRandomControl() & 0x1F;
				}
				Weather.Flash(R, G, B, 0.03f);

				TriggerFlashSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
				TriggerFlashSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
			}
			else
			{
				// Trigger a new grenade in the case of GRENADE_SUPER until itemFlags[1] is > 0
				short newGrenadeItemNumber = CreateItem();
				if (newGrenadeItemNumber != NO_ITEM)
				{
					ITEM_INFO* newGrenade = &g_Level.Items[newGrenadeItemNumber];

					newGrenade->shade = 0xC210;
					newGrenade->objectNumber = ID_GRENADE;
					newGrenade->roomNumber = item->roomNumber;
					newGrenade->pos.xPos = (GetRandomControl() & 0x1FF) + item->pos.xPos - 256;
					newGrenade->pos.yPos = item->pos.yPos - 256;
					newGrenade->pos.zPos = (GetRandomControl() & 0x1FF) + item->pos.zPos - 256;
					
					InitialiseItem(newGrenadeItemNumber);
					
					newGrenade->pos.xRot = (GetRandomControl() & 0x3FFF) + ANGLE(45);
					newGrenade->pos.yRot = GetRandomControl() * 2;
					newGrenade->pos.zRot = 0;
					newGrenade->speed = 64;
					newGrenade->fallspeed = -64 * phd_sin(newGrenade->pos.xRot);
					newGrenade->currentAnimState = newGrenade->pos.xRot;
					newGrenade->goalAnimState = newGrenade->pos.yRot;
					newGrenade->requiredAnimState = 0;
					
					AddActiveItem(newGrenadeItemNumber);
					
					newGrenade->status = ITEM_INVISIBLE;
					newGrenade->itemFlags[2] = item->itemFlags[2];
					newGrenade->hitPoints = 3000; // 60; // 3000;
					newGrenade->itemFlags[0] = GRENADE_ULTRA;

					if (g_Level.Rooms[newGrenade->roomNumber].flags & ENV_FLAG_WATER)
						newGrenade->hitPoints = 1;
				}
			}

			return;
		}

		KillItem(itemNumber);
		return;
	}
	   
	// Store old position for later
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;

	int xv;
	int yv;
	int zv;

	item->shade = 0xC210;

	// Check if above water and update speed and fallspeed
	bool aboveWater = false;
	bool someFlag = false;
	if (g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		aboveWater = false;
		someFlag = false;
		item->fallspeed += (5 - item->fallspeed) >> 1;
		item->speed -= item->speed >> 2;
		if (item->speed)
		{
			item->pos.zRot += (((item->speed >> 4) + 3) * ANGLE(1));
			if (item->requiredAnimState)
				item->pos.yRot += (((item->speed >> 2) + 3) * ANGLE(1));
			else
				item->pos.xRot += (((item->speed >> 2) + 3) * ANGLE(1));
		}
	}
	else
	{
		aboveWater = true;
		someFlag = true;
		item->fallspeed += 3;
		if (item->speed)
		{
			item->pos.zRot += (((item->speed >> 2) + 7) * ANGLE(1));
			if (item->requiredAnimState)
				item->pos.yRot += (((item->speed >> 1) + 7) * ANGLE(1));
			else
				item->pos.xRot += (((item->speed >> 1) + 7) * ANGLE(1));

		}
	}

	// Trigger fire and smoke sparks in the direction of motion
	if (item->speed && aboveWater)
	{
		Matrix world = Matrix::CreateFromYawPitchRoll(
			TO_RAD(item->pos.yRot - ANGLE(180)),
			TO_RAD(item->pos.xRot),
			TO_RAD(item->pos.zRot)
		) * Matrix::CreateTranslation(0, 0, -64);

		int wx = world.Translation().x;
		int wy = world.Translation().y;
		int wz = world.Translation().z;

		TriggerRocketSmoke(wx + item->pos.xPos, wy + item->pos.yPos, wz + item->pos.zPos, -1);
		TriggerRocketFire(wx + item->pos.xPos, wy + item->pos.yPos, wz + item->pos.zPos);
	}

	// Update grenade position
	xv = item->speed * phd_sin(item->goalAnimState);
	yv = item->fallspeed;
	zv = item->speed * phd_cos(item->goalAnimState);

	item->pos.xPos += xv;
	item->pos.yPos += yv;
	item->pos.zPos += zv;

	FLOOR_INFO* floor;
	int height;
	int ceiling;
	short roomNumber;

	// Grenades that originate from first grenade when special ammo is selected
	if (item->itemFlags[0] == GRENADE_ULTRA)
	{
		roomNumber = item->roomNumber;
		
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (height < item->pos.yPos || ceiling > item->pos.yPos)
			item->hitPoints = 1;
	}
	else
	{
		// Do grenade's physics
		short sYrot = item->pos.yRot;
		item->pos.yRot = item->goalAnimState;

		DoProjectileDynamics(itemNumber, oldX, oldY, oldZ, xv, yv, zv);

		item->goalAnimState = item->pos.yRot;
		item->pos.yRot = sYrot;
	}

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	// TODO: splash effect
	/*
	if ( *(Rooms + 148 * v78 + 78) & 1 && someFlag )
  {
    dword_804E20 = item->pos.xPos;
    dword_804E24 = *(Rooms + 148 * v78 + 36);
    dword_804E28 = item->pos.zPos;
    word_804E2C = 32;
    word_804E2E = 8;
    word_804E30 = 320;
    v45 = item->fallSpeed;
    word_804E34 = 48;
    word_804E32 = -40 * v45;
    word_804E36 = 32;
    word_804E38 = 480;
    word_804E3A = -20 * item->fallSpeed;
    word_804E3C = 32;
    word_804E3E = 128;
    word_804E40 = 544;
    SetupSplash(&dword_804E20);
    if ( item->itemFlags[0] != 4 )
    {
      goto LABEL_35;
    }
    item->hitPoints = 1;
  }*/

	if (item->itemFlags[0] == GRENADE_ULTRA)
		TriggerFireFlame(item->pos.xPos, item->pos.yPos, item->pos.zPos, -1, 1);

	// Check if it's time to explode
	int radius = 0;
	bool explode = false; 

	if (item->hitPoints)
	{
		item->hitPoints--;

		if (item->hitPoints)
		{
			if (item->hitPoints > 118)
			{
				return;
			}
		}
		else
		{
			radius = 2048;
			explode = true;
		}
	}

	// If is not a flash grenade then try to destroy surrounding objects
	if (!(item->itemFlags[0] == GRENADE_FLASH && explode))
	{
		//int radius = (explode ? GRENADE_EXPLODE_RADIUS : GRENADE_HIT_RADIUS);
		bool foundCollidedObjects = false;

		for (int n = 0; n < 2; n++)
		{
			// Step 0: check for specific collision in a small radius
			// Step 1: done only if explosion, try to smash all objects in the blast radius

			// Found possible collided items and statics
			GetCollidedObjects(item, radius, 1, &CollidedItems[0], &CollidedMeshes[0], false);

			if (explode)
			{
				for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
				{
					if (CollidedItems[i] == NULL)
						break;

					ITEM_INFO* currentItem = CollidedItems[i];

					if (currentItem->objectNumber < ID_SMASH_OBJECT1 || currentItem->objectNumber > ID_SMASH_OBJECT16)
					{
						if (currentItem->objectNumber < ID_SHOOT_SWITCH1 || currentItem->objectNumber > ID_SHOOT_SWITCH4 || (currentItem->flags & 0x40))
						{
							if (Objects[currentItem->objectNumber].intelligent || currentItem->objectNumber == ID_LARA)
							{
								DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_GRENADE_LAUNCHER);
							}
						}
						else
						{
							if ((currentItem->flags & IFLAG_ACTIVATION_MASK) &&
								(currentItem->flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
							{
								TestTriggers(currentItem->pos.xPos, currentItem->pos.yPos - 256, currentItem->pos.zPos, roomNumber, true, IFLAG_ACTIVATION_MASK);
							}
							else
							{
								short itemNos[100];
								int numSwitchItems = GetSwitchTrigger(currentItem, itemNos, 0);
								if (numSwitchItems > 0)
								{
									for (int j = 0; j < numSwitchItems; j++)
									{
										AddActiveItem(itemNos[j]);
										g_Level.Items[itemNos[j]].status = ITEM_ACTIVE;
										g_Level.Items[itemNos[j]].flags |= 0x3E00;
									}
								}
							}

							if (currentItem->objectNumber == ID_SHOOT_SWITCH1)
							{
								ExplodeItemNode(currentItem, Objects[currentItem->objectNumber].nmeshes - 1, 0, 64);
							}

							AddActiveItem(currentItem - g_Level.Items.data());

							currentItem->status = ITEM_ACTIVE;
							currentItem->flags |= 0x3E40;
						}
					}
					else
					{
						// Smash objects are legacy objects from TRC, let's make them explode in the legacy way
						TriggerExplosionSparks(currentItem->pos.xPos, currentItem->pos.yPos, currentItem->pos.zPos, 3, -2, 0, currentItem->roomNumber);
						auto pos = PHD_3DPOS(currentItem->pos.xPos, currentItem->pos.yPos - 128, currentItem->pos.zPos);
						TriggerShockwave(&pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);
						ExplodeItemNode(currentItem, 0, 0, 128);
						short currentItemNumber = (currentItem - CollidedItems[0]);
						SmashObject(currentItemNumber);
						KillItem(currentItemNumber);
					}
				}

				if (CollidedMeshes[0])
				{
					MESH_INFO* currentMesh = CollidedMeshes[0];
					int k = 0;

					do
					{
						STATIC_INFO* s = &StaticObjects[currentMesh->staticNumber];
						if (s->shatterType != SHT_NONE)
						{
							currentMesh->hitPoints -= Weapons[WEAPON_GRENADE_LAUNCHER].damage;
							if (currentMesh->hitPoints <= 0)
							{
								TriggerExplosionSparks(currentMesh->pos.xPos, currentMesh->pos.yPos, currentMesh->pos.zPos, 3, -2, 0, item->roomNumber);
								auto pos = PHD_3DPOS(currentMesh->pos.xPos, currentMesh->pos.yPos - 128, currentMesh->pos.zPos, 0, currentMesh->pos.yRot, 0);
								TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
								ShatterObject(NULL, currentMesh, -128, item->roomNumber, 0);
								SmashedMeshRoom[SmashedMeshCount] = item->roomNumber;
								SmashedMesh[SmashedMeshCount] = currentMesh;
								SmashedMeshCount++;
								currentMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
							}
						}

						k++;
						currentMesh = CollidedMeshes[k];

					} while (currentMesh);
				}
			}
			else
			{
				// If no collided items and meshes are found, then exit the loop
				if (!CollidedItems[0] && !CollidedMeshes[0])
					return;

				explode = true;
				if (item->itemFlags[0] == GRENADE_FLASH)
				{
					break;
				}

				radius = GRENADE_EXPLODE_RADIUS;
			}
		}
	}

	// Handle explosion effects
	if (explode || (item->itemFlags[0] == GRENADE_FLASH && explode))
	{
		if (item->itemFlags[0] == GRENADE_FLASH)
		{
			Weather.Flash(255, 255, 255, 0.03f);
			TriggerFlashSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
			TriggerFlashSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
		}
		else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		{
			TriggerUnderwaterExplosion(item, 0);
		}
		else
		{
			item->pos.yPos -= 128;
			TriggerShockwave(&item->pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);

			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->roomNumber);
			for (int x = 0; x < 2; x++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->roomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->pos, 0, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->pos, 0);

		// Setup the counter for spawned grenades in the case of flash and super grenades ammos
		if (item->itemFlags[0] != GRENADE_NORMAL && item->itemFlags[0] != GRENADE_ULTRA)
		{
			item->meshBits = 0;
			item->itemFlags[1] = (item->itemFlags[0] != GRENADE_SUPER ? 16 : 4);
			return;
		}

		KillItem(itemNumber);
		return;
	}
}

void RocketControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	// Save old position for later
	short oldroom = item->roomNumber;
	int oldx = item->pos.xPos;
	int oldy = item->pos.yPos;
	int oldz = item->pos.zPos;

	// Update speed and rotation and check if above water or underwater
	bool abovewater = false;
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		if (item->speed > ROCKET_SPEED / 4)
			item->speed -= (item->speed / 4);
		else
		{
			item->speed += (item->speed / 4) + 4;
			if (item->speed > ROCKET_SPEED / 4)
				item->speed = ROCKET_SPEED / 4;
		}

		item->pos.zRot += (((item->speed / 8) + 3) * ANGLE(1));
		abovewater = false;
	}
	else
	{
		if (item->speed < ROCKET_SPEED)
			item->speed += (item->speed / 4) + 4;
		item->pos.zRot += (((item->speed / 4) + 7) * ANGLE(1));
		abovewater = true;
	}

	item->shade = 0x4210 | 0x8000;

	// Calculate offset in rocket direction for fire and smoke sparks
	Matrix world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot - ANGLE(180)),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	) * Matrix::CreateTranslation(0, 0, -64);

	int wx = world.Translation().x;
	int wy = world.Translation().y;
	int wz = world.Translation().z;

	// Trigger fire, smoke and lighting
	TriggerRocketSmoke(wx + item->pos.xPos, wy + item->pos.yPos, wz + item->pos.zPos, -1);
	TriggerRocketFire(wx + item->pos.xPos, wy + item->pos.yPos, wz + item->pos.zPos);
	TriggerDynamicLight(wx + item->pos.xPos + (GetRandomControl() & 15) - 8, wy + item->pos.yPos + (GetRandomControl() & 15) - 8, wz + item->pos.zPos + (GetRandomControl() & 15) - 8, 14, 28 + (GetRandomControl() & 3), 16 + (GetRandomControl() & 7), (GetRandomControl() & 7));

	// If underwater generate bubbles
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		PHD_VECTOR pos;
		pos.x = wx + item->pos.xPos;
		pos.y = wy + item->pos.yPos;
		pos.z = wz + item->pos.zPos;
		CreateBubble(&pos, item->roomNumber, 4, 8, 0, 0, 0, 0);
	}

	// Update rocket's position
	short speed = item->speed * phd_cos(item->pos.xRot);
	item->pos.xPos += speed * phd_sin(item->pos.yRot);
	item->pos.yPos += -item->speed * phd_sin(item->pos.xRot);
	item->pos.zPos += speed * phd_cos(item->pos.yRot);

	bool explode = false;
	
	// Check if solid wall and then decide if explode or not
	short roomNumber = item->roomNumber;
	FLOOR_INFO * floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos ||
		GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos)
	{
		item->pos.xPos = oldx;
		item->pos.yPos = oldy;
		item->pos.zPos = oldz;
		explode = true;
	}

	// Has bolt changed room?
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER) && abovewater)
		SetupRipple(item->pos.xPos, g_Level.Rooms[item->roomNumber].minfloor, item->pos.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);

	int radius = (explode ? ROCKET_EXPLODE_RADIUS : ROCKET_HIT_RADIUS);
	bool foundCollidedObjects = false;

	for (int n = 0; n < 2; n++)
	{
		// Step 0: check for specific collision in a small radius
		// Step 1: done only if explosion, try to smash all objects in the blast radius

		// Found possible collided items and statics
		GetCollidedObjects(item, radius, 1, &CollidedItems[0], &CollidedMeshes[0], true);

		// If no collided items and meshes are found, then exit the loop
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		if (CollidedItems[0])
		{
			ITEM_INFO* currentItem = CollidedItems[0];
			
			int k = 0;
			do
			{
				OBJECT_INFO* currentObj = &Objects[currentItem->objectNumber];

				if ((currentObj->intelligent && currentObj->collision && currentItem->status == ITEM_ACTIVE)
					|| currentItem->objectNumber == ID_LARA
					|| (currentItem->flags & 0x40 &&
					(Objects[currentItem->objectNumber].explodableMeshbits || currentItem == LaraItem)))
				{
					// All active intelligent creatures explode, if their HP is <= 0
					// Explosion is handled by CreatureDie()
					// Also Lara can be damaged
					// HitTarget() is called inside this
					DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_ROCKET_LAUNCHER);
				}
				else if (currentItem->objectNumber >= ID_SMASH_OBJECT1 && currentItem->objectNumber <= ID_SMASH_OBJECT8)
				{
					// Smash objects are legacy objects from TRC, let's make them explode in the legacy way
					TriggerExplosionSparks(currentItem->pos.xPos, currentItem->pos.yPos, currentItem->pos.zPos, 3, -2, 0, currentItem->roomNumber);
					auto pos = PHD_3DPOS(currentItem->pos.xPos, currentItem->pos.yPos - 128, currentItem->pos.zPos);
					TriggerShockwave(&pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);
					ExplodeItemNode(currentItem, 0, 0, 128);
					short currentItemNumber = (currentItem - CollidedItems[0]);
					SmashObject(currentItemNumber);
					KillItem(currentItemNumber);
				}
				// TODO_LUA: we need to handle it with an event like OnDestroy
				/*else if (currentObj->hitEffect == HIT_SPECIAL)
				{
					// Some objects need a custom behaviour
					//HitSpecial(item, currentItem, 1);
				}*/

				// All other items (like puzzles) don't explode

				k++;
				currentItem = CollidedItems[k];

			} while (currentItem);
		}

		if (CollidedMeshes[0])
		{
			MESH_INFO* currentMesh = CollidedMeshes[0];
			int k = 0;

			do
			{
				STATIC_INFO* s = &StaticObjects[currentMesh->staticNumber];
				if (s->shatterType != SHT_NONE)
				{
					currentMesh->hitPoints -= Weapons[WEAPON_ROCKET_LAUNCHER].damage;
					if (currentMesh->hitPoints <= 0)
					{
						TriggerExplosionSparks(currentMesh->pos.xPos, currentMesh->pos.yPos, currentMesh->pos.zPos, 3, -2, 0, item->roomNumber);
						auto pos = PHD_3DPOS(currentMesh->pos.xPos, currentMesh->pos.yPos - 128, currentMesh->pos.zPos, 0, currentMesh->pos.yRot, 0);
						TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
						ShatterObject(NULL, currentMesh, -128, item->roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->roomNumber;
						SmashedMesh[SmashedMeshCount] = currentMesh;
						SmashedMeshCount++;
						currentMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
					}
				}

				k++;
				currentMesh = CollidedMeshes[k];

			} while (currentMesh);
		}

		explode = true;
		radius = ROCKET_EXPLODE_RADIUS;
	}

	// Do explosion if needed
	if (explode)
	{
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			TriggerShockwave(&item->pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);
			item->pos.yPos += 128;
			TriggerExplosionSparks(oldx, oldy, oldz, 3, -2, 0, item->roomNumber);
			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(oldx, oldy, oldz, 3, -1, 0, item->roomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->pos, 0, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->pos, 0);

		ExplodeItemNode(item, 0, 0, EXPLODE_NORMAL);
		KillItem(itemNumber);
	}
}

void draw_shotgun(int weaponType)
{
	ITEM_INFO* item;

	if (Lara.weaponItem == NO_ITEM)
	{
		Lara.weaponItem = CreateItem();
		
		item = &g_Level.Items[Lara.weaponItem];

		item->objectNumber = WeaponObject(weaponType);

		if (weaponType == WEAPON_ROCKET_LAUNCHER)
			item->animNumber = Objects[item->objectNumber].animIndex + 1;
		else if (weaponType == WEAPON_GRENADE_LAUNCHER)
			item->animNumber = Objects[item->objectNumber].animIndex + 0;
		else
			item->animNumber = Objects[item->objectNumber].animIndex + 1;
		
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = WSTATE_DRAW;
		item->currentAnimState = WSTATE_DRAW;
		item->status = ITEM_ACTIVE;
		item->roomNumber = NO_ROOM;
		
		Lara.rightArm.frameBase = Objects[item->objectNumber].frameBase;
		Lara.leftArm.frameBase = Lara.rightArm.frameBase;
	}
	else
	{
		item = &g_Level.Items[Lara.weaponItem];
	}

	AnimateItem(item);

	if (item->currentAnimState != 0 && item->currentAnimState != 6)
	{
		if (item->frameNumber - g_Level.Anims[item->animNumber].frameBase == Weapons[weaponType].drawFrame)
			draw_shotgun_meshes(weaponType);
		else if (Lara.waterStatus == LW_UNDERWATER)
			item->goalAnimState = 6;
	}
	else
	{
		ready_shotgun(weaponType);
	}

	Lara.leftArm.frameBase = Lara.rightArm.frameBase = g_Level.Anims[item->animNumber].framePtr;
	Lara.leftArm.frameNumber = Lara.rightArm.frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
	Lara.leftArm.animNumber = Lara.rightArm.animNumber = item->animNumber;
}

void AnimateShotgun(int weaponType)
{
//	if (HKTimer)
//	{
//		HKFlag = 0;
//		HKTimer--;
//	}

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
			pos.x = 0;
			pos.y = 228;
			pos.z = 0;
		}
		else if (SmokeWeapon == WEAPON_GRENADE_LAUNCHER)
		{
			pos.x = 0;
			pos.y = 180;
			pos.z = 80;
		}
		else if (SmokeWeapon == WEAPON_ROCKET_LAUNCHER)
		{
			pos.x = 0;
			pos.y = 84;
			pos.z = 72;
		}

		GetLaraJointPosition(&pos, LM_RHAND);

		if (LaraItem->meshBits)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
	}

	ITEM_INFO* item = &g_Level.Items[Lara.weaponItem];
	bool running = (weaponType == WEAPON_HK && LaraItem->speed != 0);
	bool harpoonFired = false;

	switch (item->currentAnimState)
	{
	case WSTATE_AIM:
//		HKFlag = 0;
//		HKTimer = 0;
//		HKFlag2 = 0;

		if (Lara.waterStatus == LW_UNDERWATER || running)
			item->goalAnimState = WSTATE_UW_AIM;
		else if ((!(TrInput & IN_ACTION) || Lara.target) && Lara.leftArm.lock == 0)
			item->goalAnimState = WSTATE_UNAIM;
		else
			item->goalAnimState = WSTATE_RECOIL;

		break;

	case WSTATE_UW_AIM:
//		HKFlag = 0;
//		HKTimer = 0;
//		HKFlag2 = 0;

		if (Lara.waterStatus == LW_UNDERWATER || running)
		{
			if ((!(TrInput & IN_ACTION) || Lara.target) && Lara.leftArm.lock == 0)
				item->goalAnimState = WSTATE_UW_UNAIM;
			else
				item->goalAnimState = WSTATE_UW_RECOIL;
		}
		else
			item->goalAnimState = WSTATE_AIM;
		
		break;

	case WSTATE_RECOIL:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			item->goalAnimState = WSTATE_UNAIM;
			
			if (Lara.waterStatus != LW_UNDERWATER && !running && !harpoonFired)
			{
				if ((TrInput & IN_ACTION) && (!Lara.target || Lara.leftArm.lock))
				{
					if (weaponType == WEAPON_HARPOON_GUN)
					{
						FireHarpoon();
						if (!(Lara.Weapons[WEAPON_HARPOON_GUN].Ammo->getCount() & 3)) 
							harpoonFired = true;
					}
					else if (weaponType == WEAPON_ROCKET_LAUNCHER)
					{
						FireRocket();
					}
					else if (weaponType == WEAPON_GRENADE_LAUNCHER)
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
//						HKFlag = 1;

						if (Lara.Weapons[WEAPON_HK].HasSilencer)
						{
							SoundEffect(SFX_TR5_HK_SILENCED, 0, 0);
						}
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, &LaraItem->pos, 0);
						}
					}
					else
						FireShotgun();

					item->goalAnimState = WSTATE_RECOIL;
				}
				else if (Lara.leftArm.lock)
					item->goalAnimState = 0; 
			}

			if (item->goalAnimState != WSTATE_RECOIL 
//				&& HKFlag 
				&& !(Lara.Weapons[WEAPON_HK].HasSilencer))
			{
				StopSoundEffect(SFX_TR5_HK_FIRE);
				SoundEffect(SFX_TR5_HK_STOP, &LaraItem->pos, 0);
//				HKFlag = 0;
			}
		}
/*		else if (HKFlag)
		{
			if (Lara.Weapons[WEAPON_HK].HasSilencer)
			{
				SoundEffect(SFX_HK_SILENCED, 0, 0);
			}
			else
			{
				SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 83888140);
				SoundEffect(SFX_HK_FIRE, &LaraItem->pos, 0);
			}
		}*/
		else if (weaponType == WEAPON_SHOTGUN && !(TrInput & IN_ACTION) && !Lara.leftArm.lock)
		{
			item->goalAnimState = WSTATE_UNAIM;
		}

		if (item->frameNumber - g_Level.Anims[item->animNumber].frameBase == 12 
			&& weaponType == WEAPON_SHOTGUN)
			TriggerGunShell(1, ID_SHOTGUNSHELL, WEAPON_SHOTGUN);
		break;

	case WSTATE_UW_RECOIL:
		if (item->frameNumber - g_Level.Anims[item->animNumber].frameBase == 0)
		{
			item->goalAnimState = WSTATE_UW_UNAIM;

			if ((Lara.waterStatus == LW_UNDERWATER || running)
				&& !harpoonFired)
			{
				if ((TrInput & IN_ACTION) 
					&& (!Lara.target 
						|| Lara.leftArm.lock))
				{
					if (weaponType == WEAPON_HARPOON_GUN)
					{
						FireHarpoon();
						if (!(Lara.Weapons[WEAPON_HARPOON_GUN].Ammo->getCount() & 3))
							harpoonFired = true;
					}
					else if (weaponType == WEAPON_HK)// && (/*!(Lara.HKtypeCarried & 0x18) || */!HKTimer))
					{
						FireHK(1);
//						HKFlag = 1;
						item->goalAnimState = 8;
						if (Lara.Weapons[WEAPON_HK].HasSilencer)
						{
							SoundEffect(14, 0, 0);
						}
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, &LaraItem->pos, 0);
						}
					}
					else
					{
						item->goalAnimState = WSTATE_UW_AIM;
					}

					item->goalAnimState = WSTATE_UW_RECOIL;
				}
				else if (Lara.leftArm.lock)
					item->goalAnimState = WSTATE_UW_AIM;
			}
			else if (item->goalAnimState != WSTATE_UW_RECOIL 
//				&& HKFlag 
				&& !(Lara.Weapons[WEAPON_HK].HasSilencer))
			{
				StopSoundEffect(SFX_TR5_HK_FIRE);
				SoundEffect(SFX_TR5_HK_STOP, &LaraItem->pos, 0);
//				HKFlag = 0;
			}
/*			else if (HKFlag)
			{
				if (Lara.Weapons[WEAPON_HK].HasSilencer)
				{
					SoundEffect(SFX_HK_SILENCED, 0, 0);
				}
				else
				{
					SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 83888140);
					SoundEffect(SFX_HK_FIRE, &LaraItem->pos, 0);
				}
			}*/
		}		

		break;

	default:
		break;
	}

	AnimateItem(item);

	Lara.leftArm.frameBase = Lara.rightArm.frameBase = g_Level.Anims[item->animNumber].framePtr;
	Lara.leftArm.frameNumber = Lara.rightArm.frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
	Lara.leftArm.animNumber = Lara.rightArm.animNumber = item->animNumber;
}

enum CROSSBOW_TYPE
{
	CROSSBOW_NORMAL,
	CROSSBOW_POISON,
	CROSSBOW_EXPLODE
};

void CrossbowBoltControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	// Store old position for later
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;
	short roomNumber = item->roomNumber;

	bool aboveWater = false;
	bool explode = false;

	// Update speed and check if above water
	if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
	{
		PHD_VECTOR bubblePos(item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (item->speed > 64)
			item->speed -= (item->speed >> 4);
		if (GlobalCounter & 1)
			CreateBubble(&bubblePos, roomNumber, 4, 7, 0, 0, 0, 0);
		aboveWater = false;
	}
	else
	{
		aboveWater = true;
	}

	// Update bolt's position
	item->pos.xPos += item->speed * phd_cos(item->pos.xRot) * phd_sin(item->pos.yRot);
	item->pos.yPos += item->speed * phd_sin(-item->pos.xRot);
	item->pos.zPos += item->speed * phd_cos(item->pos.xRot) * phd_cos(item->pos.yRot);

	roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	
	// Check if bolt has hit a solid wall
	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos ||
		GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos)
	{
		// I have hit a solid wall, this is the end for the bolt
		item->pos.xPos = oldX;
		item->pos.yPos = oldY;
		item->pos.zPos = oldZ;

		// If ammos are normal, then just shatter the bolt and quit
		if (item->itemFlags[0] != CROSSBOW_EXPLODE)
		{
			ExplodeItemNode(item, 0, 0, EXPLODE_NORMAL);
			KillItem(itemNumber);
			return;
		}
		else
		{
			// Otherwise, bolt must explode
			explode = true;
		}
	}

	// Has bolt changed room?
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER) && aboveWater)
	{
		SetupRipple(item->pos.xPos, g_Level.Rooms[item->roomNumber].minfloor, item->pos.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
	}

	int radius = (explode ? CROSSBOW_EXPLODE_RADIUS : CROSSBOW_HIT_RADIUS);
	bool foundCollidedObjects = false;

	for (int n = 0; n < 2; n++)
	{
		// Step 0: check for specific collision in a small radius
		// Step 1: done only if explosion, try to smash all objects in the blast radius

		// Found possible collided items and statics
		GetCollidedObjects(item, radius, 1, &CollidedItems[0], &CollidedMeshes[0], true);
		
		// If no collided items and meshes are found, then exit the loop
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		foundCollidedObjects = true;

		// If explosive ammos selected and item hit, then blast everything
		if (item->itemFlags[0] == CROSSBOW_EXPLODE)
			explode = true;

		if (CollidedItems[0])
		{
			ITEM_INFO* currentItem = CollidedItems[0];
			
			int k = 0;
			do
			{
				OBJECT_INFO* currentObj = &Objects[currentItem->objectNumber];

				if ((currentObj->intelligent && currentObj->collision && currentItem->status == ITEM_ACTIVE && !currentObj->undead)
					|| (currentItem->objectNumber == ID_LARA && explode)
					|| (currentItem->flags & 0x40 &&
					(Objects[currentItem->objectNumber].explodableMeshbits || currentItem == LaraItem)))
				{
					if (explode)
					{
						// All active intelligent creatures explode, if their HP is <= 0
						// Explosion is handled by CreatureDie()
						// Also Lara can be damaged
						// HitTarget() is called inside this
						DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_CROSSBOW);
					}
					else if (currentItem->objectNumber != ID_LARA)
					{
						// Normal hit
						HitTarget(LaraItem, currentItem, (GAME_VECTOR*)& item->pos, Weapons[WEAPON_CROSSBOW].damage << item->itemFlags[0], 0);

						// Poisoned ammos
						if (item->itemFlags[0] == CROSSBOW_POISON)
							currentItem->poisoned = true;
					}
				}
				else if (currentItem->objectNumber >= ID_SMASH_OBJECT1 && currentItem->objectNumber <= ID_SMASH_OBJECT8)
				{
					// Smash objects are legacy objects from TRC, let's make them explode in the legacy way

					if (explode)
						ExplodeItemNode(currentItem, 0, 0, 128);

					short currentItemNumber = (currentItem - CollidedItems[0]);
					SmashObject(currentItemNumber);
					KillItem(currentItemNumber);
				}

				// TODO_LUA: we need to handle it with an event like OnDestroy
				/*else if (currentObj->hitEffect == HIT_SPECIAL)
				{
					// Some objects need a custom behaviour
					//HitSpecial(item, currentItem, 1);
				}*/

				// All other items (like puzzles) don't explode

				k++;
				currentItem = CollidedItems[k];

			} while (currentItem);
		}

		if (CollidedMeshes[0])
		{
			MESH_INFO* currentMesh = CollidedMeshes[0];
			int k = 0;

			do
			{
				STATIC_INFO* s = &StaticObjects[currentMesh->staticNumber];
				if (s->shatterType != SHT_NONE)
				{
					currentMesh->hitPoints -= Weapons[WEAPON_CROSSBOW].damage;
					if (currentMesh->hitPoints <= 0)
					{
						ShatterObject(NULL, currentMesh, -128, item->roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->roomNumber;
						SmashedMesh[SmashedMeshCount] = currentMesh;
						SmashedMeshCount++;
						currentMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
					}
				}

				k++;
				currentMesh = CollidedMeshes[k];

			} while (currentMesh);
		}

		break;

		explode = true;
		radius = CROSSBOW_EXPLODE_RADIUS;
	};
		
	if (!explode)
	{
		// If bolt has hit some objects then shatter itself
		if (foundCollidedObjects)
		{
			ExplodeItemNode(item, 0, 0, EXPLODE_NORMAL);
			KillItem(itemNumber);
		}
	}
	else
	{
		// Explode
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		{
			TriggerUnderwaterExplosion(item, 0);
		}
		else
		{
			TriggerShockwave(&item->pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);
			item->pos.yPos += 128;
			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->roomNumber);
			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->roomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->pos, 0, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->pos, 0);

		ExplodeItemNode(item, 0, 0, EXPLODE_NORMAL);
		KillItem(itemNumber);
	}
}

void RifleHandler(int weaponType)
{
	if (BinocularRange)
		return; // Never handle weapons when in binocular mode!

	WEAPON_INFO* weapon = &Weapons[weaponType];

	LaraGetNewTarget(LaraItem, weapon);
	if (TrInput & IN_ACTION)
		LaraTargetInfo(LaraItem, weapon);
	AimWeapon(LaraItem, weapon, &Lara.leftArm);

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
			PHD_VECTOR pos = {};
			pos.y = -64;
			GetLaraJointPosition(&pos, LM_RHAND);
			TriggerDynamicLight(
				pos.x,
				pos.y,
				pos.z,
				12,
				(GetRandomControl() & 0x3F) + 192,
				(GetRandomControl() & 0x1F) + 128,
				GetRandomControl() & 0x3F
			);
		}
		else if (weaponType == WEAPON_REVOLVER)
		{
			PHD_VECTOR pos = {};
			pos.y = -32;
			GetLaraJointPosition(&pos, LM_RHAND);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, (GetRandomControl() & 0x3F));
		}
	}
}

void FireCrossbow(PHD_3DPOS* pos)
{
	Ammo& ammos = GetAmmo(LaraItem, WEAPON_CROSSBOW);
	if (!ammos)
		return;

	Lara.hasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		item->objectNumber = ID_CROSSBOW_BOLT;
		item->shade = 0xC210;
		if (!ammos.hasInfinite())
			(ammos)--;
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

			PHD_VECTOR jointPos;
			jointPos.x = 0;
			jointPos.y = 228;
			jointPos.z = 32;

			GetLaraJointPosition(&jointPos, LM_RHAND);

			item->roomNumber = LaraItem->roomNumber;

			FLOOR_INFO * floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->roomNumber);
			int height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);

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

		item->itemFlags[0] = Lara.Weapons[WEAPON_CROSSBOW].SelectedAmmo;

		SoundEffect(SFX_TR4_LARA_CROSSBOW, 0, 0);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void FireCrossBowFromLaserSight(GAME_VECTOR* src, GAME_VECTOR* target)
{
	short angles[2];
	PHD_3DPOS pos;

	/* this part makes arrows fire at bad angles
	target->x &= ~1023;
	target->z &= ~1023;
	target->x |= 512;
	target->z |= 512;*/

	phd_GetVectorAngles(target->x - src->x, target->y - src->y, target->z - src->z, &angles[0]);

	pos.xPos = src->x;
	pos.yPos = src->y;
	pos.zPos = src->z;
	pos.xRot = angles[1];
	pos.yRot = angles[0];
	pos.zRot = 0;

	FireCrossbow(&pos);
}

void FireRocket()
{
	Ammo& ammos = GetAmmo(LaraItem, WEAPON_ROCKET_LAUNCHER);
	if (!ammos)
		return;

	Lara.hasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		item->objectNumber = ID_ROCKET;
		item->roomNumber = LaraItem->roomNumber;

		if (!ammos.hasInfinite())
			(ammos)--;

		PHD_VECTOR jointPos;
		jointPos.x = 0;
		jointPos.y = 180;
		jointPos.z = 72;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		int x, y, z;
		item->pos.xPos = x = jointPos.x;
		item->pos.yPos = y = jointPos.y;
		item->pos.zPos = z = jointPos.z;

		jointPos.x = 0;
		jointPos.y = 180 + 1024;
		jointPos.z = 72;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_ROCKET_LAUNCHER;

		for(int i = 0; i < 5; i++)
			TriggerGunSmoke(x, y, z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 1, WEAPON_ROCKET_LAUNCHER, 32);

		jointPos.x = 0;
		jointPos.y = -256;
		jointPos.z = 0;
		
		GetLaraJointPosition(&jointPos, LM_RHAND);

		for(int i = 0; i < 10; i++)
			TriggerGunSmoke(jointPos.x, jointPos.y, jointPos.z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 2, WEAPON_ROCKET_LAUNCHER, 32);

		InitialiseItem(itemNumber);

		item->pos.xRot = LaraItem->pos.xRot + Lara.leftArm.xRot;
		item->pos.yRot = LaraItem->pos.yRot + Lara.leftArm.yRot;
		item->pos.zRot = 0;

		if (!Lara.leftArm.lock)
		{
			item->pos.xRot += Lara.torsoXrot;
			item->pos.yRot += Lara.torsoYrot;
		}

		item->speed = 512 >> 5;
		item->itemFlags[0] = 0;

		AddActiveItem(itemNumber);

		SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void DoExplosiveDamageOnBaddie(ITEM_INFO* dest, ITEM_INFO* src, int weapon)
{
	if (!(dest->flags & 0x8000))
	{
		if (dest != LaraItem || LaraItem->hitPoints <= 0)
		{
			if (!src->itemFlags[2])
			{
				dest->hitStatus = true;

				OBJECT_INFO* obj = &Objects[dest->objectNumber];
				// TODO: in TR4 condition was objectNumber != (ID_MUMMY, ID_SKELETON, ID_SETHA)
				if (!obj->undead)
				{
					HitTarget(LaraItem, dest, 0, Weapons[weapon].explosiveDamage, 1);
					if (dest != LaraItem)
					{
						Statistics.Game.AmmoHits++;
						if (dest->hitPoints <= 0)
						{
							Statistics.Level.Kills++;
							CreatureDie((dest - g_Level.Items.data()), 1);
						}
					}
				}
			}
		}
		else
		{
			LaraItem->hitPoints -= (Weapons[weapon].damage * 5);
			if (!(g_Level.Rooms[dest->roomNumber].flags & ENV_FLAG_WATER) && LaraItem->hitPoints <= Weapons[weapon].damage)
				LaraBurn(LaraItem);
		}
	}
}

void SomeSparkEffect(int x, int y, int z, int count)
{
	for (int i = 0; i < count; i++)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
		spark->on = 1;
		spark->sR = 112;
		spark->sG = (GetRandomControl() & 0x1F) + -128;
		spark->sB = (GetRandomControl() & 0x1F) + -128;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 8;
		spark->life = 24;
		spark->dR = spark->sR >> 1;
		spark->dG = spark->sG >> 1;
		spark->dB = spark->sB >> 1;
		spark->sLife = 24;
		spark->transType = TransTypeEnum::COLADD;
		spark->friction = 5;
		int random = GetRandomControl() & 0xFFF;
		spark->xVel = -128 * phd_sin(random << 4);
		spark->yVel = -640 - (byte)GetRandomControl();
		spark->zVel = 128 * phd_cos(random << 4);
		spark->flags = 0;
		spark->x = x + (spark->xVel >> 3);
		spark->y = y - (spark->yVel >> 5);
		spark->z = z + (spark->zVel >> 3);
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 0xF) + 64;
	}
}

void TriggerUnderwaterExplosion(ITEM_INFO* item, int flag)
{
	if (flag)
	{
		int x = (GetRandomControl() & 0x1FF) + item->pos.xPos - 256;
		int y = item->pos.yPos;
		int z = (GetRandomControl() & 0x1FF) + item->pos.zPos - 256;
		
		TriggerExplosionBubbles(x, y, z, item->roomNumber);
		TriggerExplosionSparks(x, y, z, 2, -1, 1, item->roomNumber);
		
		int wh = GetWaterHeight(x, y, z, item->roomNumber);
		if (wh != NO_HEIGHT)
			SomeSparkEffect(x, wh, z, 8);
	}
	else
	{
		TriggerExplosionBubble(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
		TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, -2, 1, item->roomNumber);

		for (int i = 0; i < 3; i++)
		{
			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 2, -1, 1, item->roomNumber);
		}

		int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
		if (wh != NO_HEIGHT)
		{
			int dy = item->pos.yPos - wh;
			if (dy < 2048)
			{
				SplashSetup.y = wh;
				SplashSetup.x = item->pos.xPos;
				SplashSetup.z = item->pos.zPos;
				SplashSetup.innerRadius = 160;
				SplashSetup.splashPower = 2048 - dy;

				SetupSplash(&SplashSetup, item->roomNumber);
			}
		}
	}
}

void undraw_shotgun(int weapon)
{
	ITEM_INFO* item = &g_Level.Items[Lara.weaponItem];
	item->goalAnimState = 3;
	
	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED)
	{
		Lara.gunStatus = LG_NO_ARMS;
		Lara.target = nullptr;
		Lara.rightArm.lock = false;
		Lara.leftArm.lock = false;
		KillItem(Lara.weaponItem);
		Lara.weaponItem = NO_ITEM;
		Lara.rightArm.frameNumber = 0;
		Lara.leftArm.frameNumber = 0;
	}
	else if (item->currentAnimState == 3 && item->frameNumber - g_Level.Anims[item->animNumber].frameBase == 21)
	{
		undraw_shotgun_meshes(weapon);
	}
	
	Lara.rightArm.frameBase = g_Level.Anims[item->animNumber].framePtr;
	Lara.leftArm.frameBase = g_Level.Anims[item->animNumber].framePtr;
	Lara.rightArm.frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
	Lara.leftArm.frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
	Lara.rightArm.animNumber = item->animNumber;
	Lara.leftArm.animNumber = Lara.rightArm.animNumber;
}

void undraw_shotgun_meshes(int weapon)
{
	Lara.holsterInfo.backHolster = HolsterSlotForWeapon(static_cast<LARA_WEAPON_TYPE>(weapon));
	Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
}

void draw_shotgun_meshes(int weaponType)
{
	Lara.holsterInfo.backHolster = HOLSTER_SLOT::Empty;
	Lara.meshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(LaraItem, weaponType)].meshIndex + LM_RHAND;
}

void HitSpecial(ITEM_INFO* projectile, ITEM_INFO* target, int flags)
{
	
}

void FireHK(int mode)
{
/*	if (Lara.Weapons[WEAPON_HK].SelectedAmmo == WEAPON_AMMO1)
	{
		HKTimer = 12;
	}
	else if (Lara.Weapons[WEAPON_HK].SelectedAmmo == WEAPON_AMMO2)
	{
		HKCounter++;
		if (HKCounter == 5)
		{
			HKCounter = 0;
			HKTimer = 12;
		}
	}*/

	short angles[2];

	angles[1] = Lara.leftArm.xRot;
	angles[0] = Lara.leftArm.yRot + LaraItem->pos.yRot;

	if (!Lara.leftArm.lock)
	{
		angles[0] = Lara.torsoYrot + Lara.leftArm.yRot + LaraItem->pos.yRot;
		angles[1] = Lara.torsoXrot + Lara.leftArm.xRot;
	}

	if (mode)
	{
		Weapons[WEAPON_HK].shotAccuracy = 2184;
		Weapons[WEAPON_HK].damage = 1;
	}
	else
	{
		Weapons[WEAPON_HK].shotAccuracy = 728;
		Weapons[WEAPON_HK].damage = 3;
	}

	if (FireWeapon(WEAPON_HK, Lara.target, LaraItem, angles) != FW_NOAMMO)
	{
		SmokeCountL = 12;
		SmokeWeapon = WEAPON_HK;
		TriggerGunShell(1, ID_GUNSHELL, WEAPON_HK);
		Lara.rightArm.flash_gun = Weapons[WEAPON_HK].flashTime;
	}
}

void FireShotgun()
{
	short angles[2];
	
	angles[1] = Lara.leftArm.xRot;
	angles[0] = Lara.leftArm.yRot + LaraItem->pos.yRot;

	if (!Lara.leftArm.lock)
	{
		angles[0] = Lara.torsoYrot + Lara.leftArm.yRot + LaraItem->pos.yRot;
		angles[1] = Lara.torsoXrot + Lara.leftArm.xRot;
	}

	short loopAngles[2];
	bool fired = false;
	int value = (Lara.Weapons[WEAPON_SHOTGUN].SelectedAmmo == WEAPON_AMMO1 ? 1820 : 5460);

	for (int i = 0; i < 6; i++)
	{
		loopAngles[0] = angles[0] + value * (GetRandomControl() - 0x4000) / 0x10000;
		loopAngles[1] = angles[1] + value * (GetRandomControl() - 0x4000) / 0x10000;

		if (FireWeapon(WEAPON_SHOTGUN, Lara.target, LaraItem, loopAngles) != FW_NOAMMO)
			fired = true;
	}

	if (fired)
	{
		PHD_VECTOR pos;

		pos.x = 0;
		pos.y = 228;
		pos.z = 32;

		GetLaraJointPosition(&pos, LM_RHAND);

		PHD_VECTOR pos2;

		pos2.x = pos.x;
		pos2.y = pos.y;
		pos2.z = pos.z;

		pos.x = 0;
		pos.y = 1508;
		pos.z = 32;

		GetLaraJointPosition(&pos, LM_RHAND);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_SHOTGUN;

		if (LaraItem->meshBits != 0)
		{
			for (int i = 0; i < 7; i++)
				TriggerGunSmoke(pos2.x, pos2.y, pos2.z, pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z, 1, SmokeWeapon, SmokeCountL);
		}

		Lara.rightArm.flash_gun = Weapons[WEAPON_SHOTGUN].flashTime;
		
		SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 20971524);
		SoundEffect(Weapons[WEAPON_SHOTGUN].sampleNum, &LaraItem->pos, 0);
		
		Statistics.Game.AmmoUsed++;
	}
}

void ready_shotgun(int weaponType)
{
	Lara.gunStatus = LG_READY;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.rightArm.frameNumber = 0;
	Lara.leftArm.frameNumber = 0;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
	Lara.target = nullptr;
	Lara.rightArm.frameBase = Objects[WeaponObject(weaponType)].frameBase;
	Lara.leftArm.frameBase = Objects[WeaponObject(weaponType)].frameBase;
}
