#include "framework.h"
#include "Game/Lara/lara_one_gun.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/effects/bubble.h"
#include "Game/control/control.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/lara_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/savegame.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/input.h"
#include "Sound/sound.h"

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

	Lara.Control.WeaponControl.HasFired = true;

	// Create a new item for harpoon
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		if (!ammos.hasInfinite())
			(ammos)--;

		GAME_VECTOR pos;
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		item->Shade = 0x4210 | 0x8000;
		item->ObjectNumber = ID_HARPOON;
		item->RoomNumber = LaraItem->RoomNumber;

		PHD_VECTOR jointPos;
		
		jointPos.x = -2;
		jointPos.y = 273 + 100;
		jointPos.z = 77;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		FLOOR_INFO* floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->RoomNumber);
		int height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);

		if (height >= jointPos.y)
		{
			item->Position.xPos = jointPos.x;
			item->Position.yPos = jointPos.y;
			item->Position.zPos = jointPos.z;
		}
		else
		{
			item->Position.xPos = LaraItem->Position.xPos;
			item->Position.yPos = jointPos.y;
			item->Position.zPos = LaraItem->Position.zPos;
			item->RoomNumber = LaraItem->RoomNumber;
		}

		InitialiseItem(itemNumber);

		item->Position.xRot = Lara.LeftArm.Rotation.xRot + LaraItem->Position.xRot;
		item->Position.zRot = 0;
		item->Position.yRot = Lara.LeftArm.Rotation.yRot + LaraItem->Position.yRot;

		if (!Lara.LeftArm.Locked)
		{
			item->Position.xRot += Lara.Control.ExtraTorsoRot.zRot;
			item->Position.yRot += Lara.Control.ExtraTorsoRot.yRot;
		}

		item->Position.zRot = 0;

		item->VerticalVelocity = -HARPOON_SPEED * phd_sin(item->Position.xRot);
		item->Velocity = HARPOON_SPEED * phd_cos(item->Position.xRot);
		item->HitPoints = HARPOON_TIME;

		AddActiveItem(itemNumber);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void HarpoonBoltControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	// Store old position for later
	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;
	short roomNumber = item->RoomNumber;

	bool aboveWater = false;

	// Update speed and check if above water

	if (item->HitPoints == HARPOON_TIME)
	{
		item->Position.zRot += ANGLE(35);
		if (!(g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER))
		{
			item->Position.xRot -= ANGLE(1);
			if (item->Position.xRot < -ANGLE(90))
				item->Position.xRot = -ANGLE(90);
			item->VerticalVelocity = -HARPOON_SPEED * phd_sin(item->Position.xRot);
			item->Velocity = HARPOON_SPEED * phd_cos(item->Position.xRot);
			aboveWater = true;
		}
		else
		{
			// Create bubbles
			if ((Wibble & 15) == 0)
				CreateBubble((PHD_VECTOR*)&item->Position, item->RoomNumber, 0, 0, BUBBLE_FLAG_CLUMP | BUBBLE_FLAG_HIGH_AMPLITUDE, 0, 0, 0); // CHECK
			TriggerRocketSmoke(item->Position.xPos, item->Position.yPos, item->Position.zPos, 64);
			item->VerticalVelocity = -HARPOON_SPEED * phd_sin(item->Position.xRot) / 2;
			item->Velocity = HARPOON_SPEED * phd_cos(item->Position.xRot) / 2;
			aboveWater = false;
		}

		// Update bolt's position
		item->Position.xPos += item->Velocity * phd_cos(item->Position.xRot) * phd_sin(item->Position.yRot);
		item->Position.yPos += item->Velocity * phd_sin(-item->Position.xRot);
		item->Position.zPos += item->Velocity * phd_cos(item->Position.xRot) * phd_cos(item->Position.yRot);
	}
	else
	{
		if (item->HitPoints > 0)
			item->HitPoints--;
		else
			KillItem(itemNumber);
		return;
	}

	roomNumber = item->RoomNumber;
	FLOOR_INFO * floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);

	// Check if bolt has hit a solid wall
	if (GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) < item->Position.yPos ||
		GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) > item->Position.yPos)
	{
		// I have hit a solid wall, this is the end for the bolt
		item->HitPoints--;
		return;
	}

	// Has harpoon changed room?
	if (item->RoomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER) && aboveWater)
	{
		SetupRipple(item->Position.xPos, g_Level.Rooms[item->RoomNumber].minfloor, item->Position.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
	}

	int n = 0;
	bool foundCollidedObjects = false;
	bool explodeItem = true;

	// Found possible collided items and statics
	GetCollidedObjects(item, HARPOON_HIT_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], 1);

	// If no collided items and meshes are found, then exit the loop
	if (!CollidedItems[0] && !CollidedMeshes[0])
		return;

	if (CollidedItems[0])
	{
		ITEM_INFO* currentItem = CollidedItems[0];
		
		int k = 0;
		do
		{
			OBJECT_INFO* currentObj = &Objects[currentItem->ObjectNumber];

			if (!currentObj->isPickup && currentObj->collision && currentItem->Collidable)
				foundCollidedObjects = true;

			if (currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE && !currentObj->undead)
			{
				explodeItem = false;
				HitTarget(LaraItem, currentItem, (GAME_VECTOR*)&item->Position, Weapons[WEAPON_HARPOON_GUN].damage, 0);
			}

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
				currentMesh->HitPoints -= Weapons[WEAPON_CROSSBOW].damage;
				if (currentMesh->HitPoints <= 0)
				{
					TriggerExplosionSparks(currentMesh->pos.xPos, currentMesh->pos.yPos, currentMesh->pos.zPos, 3, -2, 0, item->RoomNumber);
					auto pos = PHD_3DPOS(currentMesh->pos.xPos, currentMesh->pos.yPos - 128, currentMesh->pos.zPos, 0, currentMesh->pos.yRot, 0);
					TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
					ShatterObject(NULL, currentMesh, -128, item->RoomNumber, 0);
					SmashedMeshRoom[SmashedMeshCount] = item->RoomNumber;
					SmashedMesh[SmashedMeshCount] = currentMesh;
					SmashedMeshCount++;
					currentMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
				}
			}

			foundCollidedObjects = true;

			k++;
			currentMesh = CollidedMeshes[k];

		} while (currentMesh);
	}

	// If harpoon has hit some objects then shatter itself
	if (foundCollidedObjects)
	{
		if (explodeItem)
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

	Lara.Control.WeaponControl.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		
		item->Shade = 0xC210;
		item->ObjectNumber = ID_GRENADE;
		item->RoomNumber = LaraItem->RoomNumber;

		PHD_VECTOR jointPos;
		jointPos.x = 0;
		jointPos.y = 276;
		jointPos.z = 80;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		item->Position.xPos = x = jointPos.x;
		item->Position.yPos = y = jointPos.y;
		item->Position.zPos = z = jointPos.z;

		FLOOR_INFO* floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->RoomNumber);
		int height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);
		if (height < jointPos.y)
		{
			item->Position.xPos = LaraItem->Position.xPos;
			item->Position.yPos = jointPos.y;
			item->Position.zPos = LaraItem->Position.zPos;
			item->RoomNumber = LaraItem->RoomNumber;
		}

		jointPos.x = 0;
		jointPos.y = 1204;
		jointPos.z = 5;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_GRENADE_LAUNCHER;

		if (LaraItem->MeshBits)
		{
			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(x, y, z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 1, WEAPON_GRENADE_LAUNCHER, 32);

		}

		InitialiseItem(itemNumber);

		item->Position.xRot = LaraItem->Position.xRot + Lara.LeftArm.Rotation.xRot;
		item->Position.yRot = LaraItem->Position.yRot + Lara.LeftArm.Rotation.yRot;
		item->Position.zRot = 0;

		if (!Lara.LeftArm.Locked)
		{
			item->Position.xRot += Lara.Control.ExtraTorsoRot.zRot;
			item->Position.yRot += Lara.Control.ExtraTorsoRot.yRot;
		}

		item->Velocity = GRENADE_SPEED;
		item->VerticalVelocity = -512 * phd_sin(item->Position.xRot);
		item->ActiveState = item->Position.xRot;
		item->TargetState = item->Position.yRot;
		item->RequiredState = 0;
		item->HitPoints = 120;	
		item->ItemFlags[0] = WEAPON_AMMO2;

		AddActiveItem(itemNumber);

		if (!ammo.hasInfinite())
			(ammo)--;

		item->ItemFlags[0] = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo;

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void GrenadeControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->ItemFlags[1])
	{
		item->ItemFlags[1]--;

		if (item->ItemFlags[1])
		{
			if (item->ItemFlags[0] == GRENADE_FLASH)
			{
				// Flash grenades
				int R, G, B;
				if (item->ItemFlags[1] == 1)
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

				TriggerFlashSmoke(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
				TriggerFlashSmoke(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
			}
			else
			{
				// Trigger a new grenade in the case of GRENADE_SUPER until itemFlags[1] is > 0
				short newGrenadeItemNumber = CreateItem();
				if (newGrenadeItemNumber != NO_ITEM)
				{
					ITEM_INFO* newGrenade = &g_Level.Items[newGrenadeItemNumber];

					newGrenade->Shade = 0xC210;
					newGrenade->ObjectNumber = ID_GRENADE;
					newGrenade->RoomNumber = item->RoomNumber;
					newGrenade->Position.xPos = (GetRandomControl() & 0x1FF) + item->Position.xPos - 256;
					newGrenade->Position.yPos = item->Position.yPos - 256;
					newGrenade->Position.zPos = (GetRandomControl() & 0x1FF) + item->Position.zPos - 256;
					
					InitialiseItem(newGrenadeItemNumber);
					
					newGrenade->Position.xRot = (GetRandomControl() & 0x3FFF) + ANGLE(45);
					newGrenade->Position.yRot = GetRandomControl() * 2;
					newGrenade->Position.zRot = 0;
					newGrenade->Velocity = 64;
					newGrenade->VerticalVelocity = -64 * phd_sin(newGrenade->Position.xRot);
					newGrenade->ActiveState = newGrenade->Position.xRot;
					newGrenade->TargetState = newGrenade->Position.yRot;
					newGrenade->RequiredState = 0;
					
					AddActiveItem(newGrenadeItemNumber);
					
					newGrenade->Status = ITEM_INVISIBLE;
					newGrenade->ItemFlags[2] = item->ItemFlags[2];
					newGrenade->HitPoints = 3000; // 60; // 3000;
					newGrenade->ItemFlags[0] = GRENADE_ULTRA;

					if (g_Level.Rooms[newGrenade->RoomNumber].flags & ENV_FLAG_WATER)
						newGrenade->HitPoints = 1;
				}
			}

			return;
		}

		KillItem(itemNumber);
		return;
	}
	   
	// Store old position for later
	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;

	int xv;
	int yv;
	int zv;

	item->Shade = 0xC210;

	// Check if above water and update speed and fallspeed
	bool aboveWater = false;
	bool someFlag = false;
	if (g_Level.Rooms[item->RoomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		aboveWater = false;
		someFlag = false;
		item->VerticalVelocity += (5 - item->VerticalVelocity) >> 1;
		item->Velocity -= item->Velocity >> 2;
		if (item->Velocity)
		{
			item->Position.zRot += (((item->Velocity >> 4) + 3) * ANGLE(1));
			if (item->RequiredState)
				item->Position.yRot += (((item->Velocity >> 2) + 3) * ANGLE(1));
			else
				item->Position.xRot += (((item->Velocity >> 2) + 3) * ANGLE(1));
		}
	}
	else
	{
		aboveWater = true;
		someFlag = true;
		item->VerticalVelocity += 3;
		if (item->Velocity)
		{
			item->Position.zRot += (((item->Velocity >> 2) + 7) * ANGLE(1));
			if (item->RequiredState)
				item->Position.yRot += (((item->Velocity >> 1) + 7) * ANGLE(1));
			else
				item->Position.xRot += (((item->Velocity >> 1) + 7) * ANGLE(1));

		}
	}

	// Trigger fire and smoke sparks in the direction of motion
	if (item->Velocity && aboveWater)
	{
		Matrix world = Matrix::CreateFromYawPitchRoll(
			TO_RAD(item->Position.yRot - ANGLE(180)),
			TO_RAD(item->Position.xRot),
			TO_RAD(item->Position.zRot)
		) * Matrix::CreateTranslation(0, 0, -64);

		int wx = world.Translation().x;
		int wy = world.Translation().y;
		int wz = world.Translation().z;

		TriggerRocketSmoke(wx + item->Position.xPos, wy + item->Position.yPos, wz + item->Position.zPos, -1);
		TriggerRocketFire(wx + item->Position.xPos, wy + item->Position.yPos, wz + item->Position.zPos);
	}

	// Update grenade position
	xv = item->Velocity * phd_sin(item->TargetState);
	yv = item->VerticalVelocity;
	zv = item->Velocity * phd_cos(item->TargetState);

	item->Position.xPos += xv;
	item->Position.yPos += yv;
	item->Position.zPos += zv;

	FLOOR_INFO* floor;
	int height;
	int ceiling;
	short roomNumber;

	// Grenades that originate from first grenade when special ammo is selected
	if (item->ItemFlags[0] == GRENADE_ULTRA)
	{
		roomNumber = item->RoomNumber;
		
		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
		ceiling = GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

		if (height < item->Position.yPos || ceiling > item->Position.yPos)
			item->HitPoints = 1;
	}
	else
	{
		// Do grenade's physics
		short sYrot = item->Position.yRot;
		item->Position.yRot = item->TargetState;

		DoProjectileDynamics(itemNumber, oldX, oldY, oldZ, xv, yv, zv);

		item->TargetState = item->Position.yRot;
		item->Position.yRot = sYrot;
	}

	roomNumber = item->RoomNumber;
	floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);

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
    item->HitPoints = 1;
  }*/

	if (item->ItemFlags[0] == GRENADE_ULTRA)
		TriggerFireFlame(item->Position.xPos, item->Position.yPos, item->Position.zPos, -1, 1);

	// Check if it's time to explode
	int radius = 0;
	bool explode = false; 

	if (item->HitPoints)
	{
		item->HitPoints--;

		if (item->HitPoints)
		{
			if (item->HitPoints > 118)
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
	if (!(item->ItemFlags[0] == GRENADE_FLASH && explode))
	{
		//int radius = (explode ? GRENADE_EXPLODE_RADIUS : GRENADE_HIT_RADIUS);
		bool foundCollidedObjects = false;

		for (int n = 0; n < 2; n++)
		{
			// Step 0: check for specific collision in a small radius
			// Step 1: done only if explosion, try to smash all objects in the blast radius

			// Found possible collided items and statics
			GetCollidedObjects(item, radius, true, &CollidedItems[0], &CollidedMeshes[0], false);

			if (explode)
			{
				for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
				{
					if (CollidedItems[i] == NULL)
						break;

					ITEM_INFO* currentItem = CollidedItems[i];

					if (currentItem->ObjectNumber < ID_SMASH_OBJECT1 || currentItem->ObjectNumber > ID_SMASH_OBJECT16)
					{
						if (currentItem->ObjectNumber < ID_SHOOT_SWITCH1 || currentItem->ObjectNumber > ID_SHOOT_SWITCH4 || (currentItem->Flags & 0x40))
						{
							if (Objects[currentItem->ObjectNumber].intelligent || currentItem->ObjectNumber == ID_LARA)
							{
								DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_GRENADE_LAUNCHER);
							}
						}
						else
						{
							if ((currentItem->Flags & IFLAG_ACTIVATION_MASK) &&
								(currentItem->Flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
							{
								TestTriggers(currentItem->Position.xPos, currentItem->Position.yPos - 256, currentItem->Position.zPos, roomNumber, true, IFLAG_ACTIVATION_MASK);
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
										g_Level.Items[itemNos[j]].Status = ITEM_ACTIVE;
										g_Level.Items[itemNos[j]].Flags |= 0x3E00;
									}
								}
							}

							if (currentItem->ObjectNumber == ID_SHOOT_SWITCH1)
							{
								ExplodeItemNode(currentItem, Objects[currentItem->ObjectNumber].nmeshes - 1, 0, 64);
							}

							AddActiveItem(currentItem - g_Level.Items.data());

							currentItem->Status = ITEM_ACTIVE;
							currentItem->Flags |= 0x3E40;
						}
					}
					else
					{
						// Smash objects are legacy objects from TRC, let's make them explode in the legacy way
						TriggerExplosionSparks(currentItem->Position.xPos, currentItem->Position.yPos, currentItem->Position.zPos, 3, -2, 0, currentItem->RoomNumber);
						auto pos = PHD_3DPOS(currentItem->Position.xPos, currentItem->Position.yPos - 128, currentItem->Position.zPos);
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
							currentMesh->HitPoints -= Weapons[WEAPON_GRENADE_LAUNCHER].damage;
							if (currentMesh->HitPoints <= 0)
							{
								TriggerExplosionSparks(currentMesh->pos.xPos, currentMesh->pos.yPos, currentMesh->pos.zPos, 3, -2, 0, item->RoomNumber);
								auto pos = PHD_3DPOS(currentMesh->pos.xPos, currentMesh->pos.yPos - 128, currentMesh->pos.zPos, 0, currentMesh->pos.yRot, 0);
								TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
								ShatterObject(NULL, currentMesh, -128, item->RoomNumber, 0);
								SmashedMeshRoom[SmashedMeshCount] = item->RoomNumber;
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
				if (item->ItemFlags[0] == GRENADE_FLASH)
				{
					break;
				}

				radius = GRENADE_EXPLODE_RADIUS;
			}
		}
	}

	// Handle explosion effects
	if (explode || (item->ItemFlags[0] == GRENADE_FLASH && explode))
	{
		if (item->ItemFlags[0] == GRENADE_FLASH)
		{
			Weather.Flash(255, 255, 255, 0.03f);
			TriggerFlashSmoke(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
			TriggerFlashSmoke(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
		}
		else if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
		{
			TriggerUnderwaterExplosion(item, 0);
		}
		else
		{
			item->Position.yPos -= 128;
			TriggerShockwave(&item->Position, 48, 304, 96, 0, 96, 128, 24, 0, 0);

			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->RoomNumber);
			for (int x = 0; x < 2; x++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->RoomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->Position, 0, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->Position, 0);

		// Setup the counter for spawned grenades in the case of flash and super grenades ammos
		if (item->ItemFlags[0] != GRENADE_NORMAL && item->ItemFlags[0] != GRENADE_ULTRA)
		{
			item->MeshBits = 0;
			item->ItemFlags[1] = (item->ItemFlags[0] != GRENADE_SUPER ? 16 : 4);
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
	short oldroom = item->RoomNumber;
	int oldx = item->Position.xPos;
	int oldy = item->Position.yPos;
	int oldz = item->Position.zPos;

	// Update speed and rotation and check if above water or underwater
	bool abovewater = false;
	if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
	{
		if (item->Velocity > ROCKET_SPEED / 4)
			item->Velocity -= (item->Velocity / 4);
		else
		{
			item->Velocity += (item->Velocity / 4) + 4;
			if (item->Velocity > ROCKET_SPEED / 4)
				item->Velocity = ROCKET_SPEED / 4;
		}

		item->Position.zRot += (((item->Velocity / 8) + 3) * ANGLE(1));
		abovewater = false;
	}
	else
	{
		if (item->Velocity < ROCKET_SPEED)
			item->Velocity += (item->Velocity / 4) + 4;
		item->Position.zRot += (((item->Velocity / 4) + 7) * ANGLE(1));
		abovewater = true;
	}

	item->Shade = 0x4210 | 0x8000;

	// Calculate offset in rocket direction for fire and smoke sparks
	Matrix world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Position.yRot - ANGLE(180)),
		TO_RAD(item->Position.xRot),
		TO_RAD(item->Position.zRot)
	) * Matrix::CreateTranslation(0, 0, -64);

	int wx = world.Translation().x;
	int wy = world.Translation().y;
	int wz = world.Translation().z;

	// Trigger fire, smoke and lighting
	TriggerRocketSmoke(wx + item->Position.xPos, wy + item->Position.yPos, wz + item->Position.zPos, -1);
	TriggerRocketFire(wx + item->Position.xPos, wy + item->Position.yPos, wz + item->Position.zPos);
	TriggerDynamicLight(wx + item->Position.xPos + (GetRandomControl() & 15) - 8, wy + item->Position.yPos + (GetRandomControl() & 15) - 8, wz + item->Position.zPos + (GetRandomControl() & 15) - 8, 14, 28 + (GetRandomControl() & 3), 16 + (GetRandomControl() & 7), (GetRandomControl() & 7));

	// If underwater generate bubbles
	if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
	{
		PHD_VECTOR pos;
		pos.x = wx + item->Position.xPos;
		pos.y = wy + item->Position.yPos;
		pos.z = wz + item->Position.zPos;
		CreateBubble(&pos, item->RoomNumber, 4, 8, 0, 0, 0, 0);
	}

	// Update rocket's position
	short speed = item->Velocity * phd_cos(item->Position.xRot);
	item->Position.xPos += speed * phd_sin(item->Position.yRot);
	item->Position.yPos += -item->Velocity * phd_sin(item->Position.xRot);
	item->Position.zPos += speed * phd_cos(item->Position.yRot);

	bool explode = false;
	
	// Check if solid wall and then decide if explode or not
	short roomNumber = item->RoomNumber;
	FLOOR_INFO * floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
	if (GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) < item->Position.yPos ||
		GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) > item->Position.yPos)
	{
		item->Position.xPos = oldx;
		item->Position.yPos = oldy;
		item->Position.zPos = oldz;
		explode = true;
	}

	// Has bolt changed room?
	if (item->RoomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER) && abovewater)
		SetupRipple(item->Position.xPos, g_Level.Rooms[item->RoomNumber].minfloor, item->Position.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);

	int radius = (explode ? ROCKET_EXPLODE_RADIUS : ROCKET_HIT_RADIUS);
	bool foundCollidedObjects = false;

	for (int n = 0; n < 2; n++)
	{
		// Step 0: check for specific collision in a small radius
		// Step 1: done only if explosion, try to smash all objects in the blast radius

		// Found possible collided items and statics
		GetCollidedObjects(item, radius, true, &CollidedItems[0], &CollidedMeshes[0], true);

		// If no collided items and meshes are found, then exit the loop
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		if (CollidedItems[0])
		{
			ITEM_INFO* currentItem = CollidedItems[0];
			
			int k = 0;
			do
			{
				OBJECT_INFO* currentObj = &Objects[currentItem->ObjectNumber];

				if ((currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE)
					|| currentItem->ObjectNumber == ID_LARA
					|| (currentItem->Flags & 0x40 &&
					(Objects[currentItem->ObjectNumber].explodableMeshbits || currentItem == LaraItem)))
				{
					// All active intelligent creatures explode, if their HP is <= 0
					// Explosion is handled by CreatureDie()
					// Also Lara can be damaged
					// HitTarget() is called inside this
					DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_ROCKET_LAUNCHER);
				}
				else if (currentItem->ObjectNumber >= ID_SMASH_OBJECT1 && currentItem->ObjectNumber <= ID_SMASH_OBJECT8)
				{
					// Smash objects are legacy objects from TRC, let's make them explode in the legacy way
					TriggerExplosionSparks(currentItem->Position.xPos, currentItem->Position.yPos, currentItem->Position.zPos, 3, -2, 0, currentItem->RoomNumber);
					auto pos = PHD_3DPOS(currentItem->Position.xPos, currentItem->Position.yPos - 128, currentItem->Position.zPos);
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
					currentMesh->HitPoints -= Weapons[WEAPON_ROCKET_LAUNCHER].damage;
					if (currentMesh->HitPoints <= 0)
					{
						TriggerExplosionSparks(currentMesh->pos.xPos, currentMesh->pos.yPos, currentMesh->pos.zPos, 3, -2, 0, item->RoomNumber);
						auto pos = PHD_3DPOS(currentMesh->pos.xPos, currentMesh->pos.yPos - 128, currentMesh->pos.zPos, 0, currentMesh->pos.yRot, 0);
						TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
						ShatterObject(NULL, currentMesh, -128, item->RoomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->RoomNumber;
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
		if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			TriggerShockwave(&item->Position, 48, 304, 96, 0, 96, 128, 24, 0, 0);
			item->Position.yPos += 128;
			TriggerExplosionSparks(oldx, oldy, oldz, 3, -2, 0, item->RoomNumber);
			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(oldx, oldy, oldz, 3, -1, 0, item->RoomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->Position, 0, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->Position, 0);

		ExplodeItemNode(item, 0, 0, EXPLODE_NORMAL);
		KillItem(itemNumber);
	}
}

void draw_shotgun(int weaponType)
{
	ITEM_INFO* item;

	if (Lara.Control.WeaponControl.WeaponItem == NO_ITEM)
	{
		Lara.Control.WeaponControl.WeaponItem = CreateItem();
		
		item = &g_Level.Items[Lara.Control.WeaponControl.WeaponItem];

		item->ObjectNumber = WeaponObject(weaponType);

		if (weaponType == WEAPON_ROCKET_LAUNCHER)
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
		else if (weaponType == WEAPON_GRENADE_LAUNCHER)
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 0;
		else
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
		
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = WSTATE_DRAW;
		item->ActiveState = WSTATE_DRAW;
		item->Status = ITEM_ACTIVE;
		item->RoomNumber = NO_ROOM;
		
		Lara.RightArm.FrameBase = Objects[item->ObjectNumber].frameBase;
		Lara.LeftArm.FrameBase = Lara.RightArm.FrameBase;
	}
	else
	{
		item = &g_Level.Items[Lara.Control.WeaponControl.WeaponItem];
	}

	AnimateItem(item);

	if (item->ActiveState != 0 && item->ActiveState != 6)
	{
		if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == Weapons[weaponType].drawFrame)
			draw_shotgun_meshes(weaponType);
		else if (Lara.Control.WaterStatus == WaterStatus::Underwater)
			item->TargetState = 6;
	}
	else
	{
		ready_shotgun(weaponType);
	}

	Lara.LeftArm.FrameBase = Lara.RightArm.FrameBase = g_Level.Anims[item->AnimNumber].framePtr;
	Lara.LeftArm.FrameNumber = Lara.RightArm.FrameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
	Lara.LeftArm.AnimNumber = Lara.RightArm.AnimNumber = item->AnimNumber;
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

		if (LaraItem->MeshBits)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
	}

	ITEM_INFO* item = &g_Level.Items[Lara.Control.WeaponControl.WeaponItem];
	bool running = (weaponType == WEAPON_HK && LaraItem->Velocity != 0);
	bool harpoonFired = false;

	switch (item->ActiveState)
	{
	case WSTATE_AIM:
//		HKFlag = 0;
//		HKTimer = 0;
//		HKFlag2 = 0;

		if (Lara.Control.WaterStatus == WaterStatus::Underwater || running)
			item->TargetState = WSTATE_UW_AIM;
		else if ((!(TrInput & IN_ACTION) || Lara.target) && Lara.LeftArm.Locked == false)
			item->TargetState = WSTATE_UNAIM;
		else
			item->TargetState = WSTATE_RECOIL;

		break;

	case WSTATE_UW_AIM:
//		HKFlag = 0;
//		HKTimer = 0;
//		HKFlag2 = 0;

		if (Lara.Control.WaterStatus == WaterStatus::Underwater || running)
		{
			if ((!(TrInput & IN_ACTION) || Lara.target) && Lara.LeftArm.Locked == false)
				item->TargetState = WSTATE_UW_UNAIM;
			else
				item->TargetState = WSTATE_UW_RECOIL;
		}
		else
			item->TargetState = WSTATE_AIM;
		
		break;

	case WSTATE_RECOIL:
		if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
		{
			item->TargetState = WSTATE_UNAIM;
			
			if (Lara.Control.WaterStatus != WaterStatus::Underwater && !running && !harpoonFired)
			{
				if ((TrInput & IN_ACTION) && (!Lara.target || Lara.LeftArm.Locked))
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
							SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->Position, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, &LaraItem->Position, 0);
						}
					}
					else
						FireShotgun();

					item->TargetState = WSTATE_RECOIL;
				}
				else if (Lara.LeftArm.Locked)
					item->TargetState = 0; 
			}

			if (item->TargetState != WSTATE_RECOIL 
//				&& HKFlag 
				&& !(Lara.Weapons[WEAPON_HK].HasSilencer))
			{
				StopSoundEffect(SFX_TR5_HK_FIRE);
				SoundEffect(SFX_TR5_HK_STOP, &LaraItem->Position, 0);
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
		else if (weaponType == WEAPON_SHOTGUN && !(TrInput & IN_ACTION) && !Lara.LeftArm.Locked)
		{
			item->TargetState = WSTATE_UNAIM;
		}

		if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 12 
			&& weaponType == WEAPON_SHOTGUN)
			TriggerGunShell(1, ID_SHOTGUNSHELL, WEAPON_SHOTGUN);
		break;

	case WSTATE_UW_RECOIL:
		if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 0)
		{
			item->TargetState = WSTATE_UW_UNAIM;

			if ((Lara.Control.WaterStatus == WaterStatus::Underwater || running)
				&& !harpoonFired)
			{
				if ((TrInput & IN_ACTION) 
					&& (!Lara.target 
						|| Lara.LeftArm.Locked))
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
						item->TargetState = 8;
						if (Lara.Weapons[WEAPON_HK].HasSilencer)
						{
							SoundEffect(14, 0, 0);
						}
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->Position, 83888140);
							SoundEffect(SFX_TR5_HK_FIRE, &LaraItem->Position, 0);
						}
					}
					else
					{
						item->TargetState = WSTATE_UW_AIM;
					}

					item->TargetState = WSTATE_UW_RECOIL;
				}
				else if (Lara.LeftArm.Locked)
					item->TargetState = WSTATE_UW_AIM;
			}
			else if (item->TargetState != WSTATE_UW_RECOIL 
//				&& HKFlag 
				&& !(Lara.Weapons[WEAPON_HK].HasSilencer))
			{
				StopSoundEffect(SFX_TR5_HK_FIRE);
				SoundEffect(SFX_TR5_HK_STOP, &LaraItem->Position, 0);
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

	Lara.LeftArm.FrameBase = Lara.RightArm.FrameBase = g_Level.Anims[item->AnimNumber].framePtr;
	Lara.LeftArm.FrameNumber = Lara.RightArm.FrameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
	Lara.LeftArm.AnimNumber = Lara.RightArm.AnimNumber = item->AnimNumber;
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
	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;
	short roomNumber = item->RoomNumber;

	bool aboveWater = false;
	bool explode = false;

	// Update speed and check if above water
	if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
	{
		PHD_VECTOR bubblePos(item->Position.xPos, item->Position.yPos, item->Position.zPos);
		if (item->Velocity > 64)
			item->Velocity -= (item->Velocity >> 4);
		if (GlobalCounter & 1)
			CreateBubble(&bubblePos, roomNumber, 4, 7, 0, 0, 0, 0);
		aboveWater = false;
	}
	else
	{
		aboveWater = true;
	}

	// Update bolt's position
	item->Position.xPos += item->Velocity * phd_cos(item->Position.xRot) * phd_sin(item->Position.yRot);
	item->Position.yPos += item->Velocity * phd_sin(-item->Position.xRot);
	item->Position.zPos += item->Velocity * phd_cos(item->Position.xRot) * phd_cos(item->Position.yRot);

	roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
	
	// Check if bolt has hit a solid wall
	if (GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) < item->Position.yPos ||
		GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) > item->Position.yPos)
	{
		// I have hit a solid wall, this is the end for the bolt
		item->Position.xPos = oldX;
		item->Position.yPos = oldY;
		item->Position.zPos = oldZ;

		// If ammos are normal, then just shatter the bolt and quit
		if (item->ItemFlags[0] != CROSSBOW_EXPLODE)
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
	if (item->RoomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	// If now in water and before in land, add a ripple
	if ((g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER) && aboveWater)
	{
		SetupRipple(item->Position.xPos, g_Level.Rooms[item->RoomNumber].minfloor, item->Position.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
	}

	int radius = (explode ? CROSSBOW_EXPLODE_RADIUS : CROSSBOW_HIT_RADIUS);
	bool foundCollidedObjects = false;

	for (int n = 0; n < 2; n++)
	{
		// Step 0: check for specific collision in a small radius
		// Step 1: done only if explosion, try to smash all objects in the blast radius

		// Found possible collided items and statics
		GetCollidedObjects(item, radius, true, &CollidedItems[0], &CollidedMeshes[0], true);
		
		// If no collided items and meshes are found, then exit the loop
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		foundCollidedObjects = true;

		// If explosive ammos selected and item hit, then blast everything
		if (item->ItemFlags[0] == CROSSBOW_EXPLODE)
			explode = true;

		if (CollidedItems[0])
		{
			ITEM_INFO* currentItem = CollidedItems[0];
			
			int k = 0;
			do
			{
				OBJECT_INFO* currentObj = &Objects[currentItem->ObjectNumber];

				if ((currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE && !currentObj->undead)
					|| (currentItem->ObjectNumber == ID_LARA && explode)
					|| (currentItem->Flags & 0x40 &&
					(Objects[currentItem->ObjectNumber].explodableMeshbits || currentItem == LaraItem)))
				{
					if (explode)
					{
						// All active intelligent creatures explode, if their HP is <= 0
						// Explosion is handled by CreatureDie()
						// Also Lara can be damaged
						// HitTarget() is called inside this
						DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_CROSSBOW);
					}
					else if (currentItem->ObjectNumber != ID_LARA)
					{
						// Normal hit
						HitTarget(LaraItem, currentItem, (GAME_VECTOR*)& item->Position, Weapons[WEAPON_CROSSBOW].damage << item->ItemFlags[0], 0);

						// Poisoned ammos
						if (item->ItemFlags[0] == CROSSBOW_POISON)
							currentItem->Poisoned = true;
					}
				}
				else if (currentItem->ObjectNumber >= ID_SMASH_OBJECT1 && currentItem->ObjectNumber <= ID_SMASH_OBJECT8)
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
					currentMesh->HitPoints -= Weapons[WEAPON_CROSSBOW].damage;
					if (currentMesh->HitPoints <= 0)
					{
						ShatterObject(NULL, currentMesh, -128, item->RoomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->RoomNumber;
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
		if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
		{
			TriggerUnderwaterExplosion(item, 0);
		}
		else
		{
			TriggerShockwave(&item->Position, 48, 304, 96, 0, 96, 128, 24, 0, 0);
			item->Position.yPos += 128;
			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->RoomNumber);
			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->RoomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->Position, 0, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->Position, 0);

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
	AimWeapon(LaraItem, weapon, &Lara.LeftArm);

	if (Lara.LeftArm.Locked)
	{
		Lara.Control.ExtraTorsoRot.zRot = Lara.LeftArm.Rotation.xRot;
		Lara.Control.ExtraTorsoRot.yRot = Lara.LeftArm.Rotation.yRot;
		if (Camera.oldType != CAMERA_TYPE::LOOK_CAMERA && !BinocularRange)
		{
			Lara.Control.ExtraHeadRot = { 0, 0, 0 };
		}
	}

	if (weaponType == WEAPON_REVOLVER)
		AnimatePistols(WEAPON_REVOLVER);
	else
		AnimateShotgun(weaponType);

	if (Lara.RightArm.FlashGun)
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

	Lara.Control.WeaponControl.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		item->ObjectNumber = ID_CROSSBOW_BOLT;
		item->Shade = 0xC210;
		if (!ammos.hasInfinite())
			(ammos)--;
		if (pos)
		{
			item->RoomNumber = LaraItem->RoomNumber;
			item->Position.xPos = pos->xPos;
			item->Position.yPos = pos->yPos;
			item->Position.zPos = pos->zPos;

			InitialiseItem(itemNumber);

			item->Position.xRot = pos->xRot;
			item->Position.yRot = pos->yRot;
			item->Position.zRot = pos->zRot;
		}
		else
		{

			PHD_VECTOR jointPos;
			jointPos.x = 0;
			jointPos.y = 228;
			jointPos.z = 32;

			GetLaraJointPosition(&jointPos, LM_RHAND);

			item->RoomNumber = LaraItem->RoomNumber;

			FLOOR_INFO * floor = GetFloor(jointPos.x, jointPos.y, jointPos.z, &item->RoomNumber);
			int height = GetFloorHeight(floor, jointPos.x, jointPos.y, jointPos.z);

			if (height >= jointPos.y)
			{
				item->Position.xPos = jointPos.x;
				item->Position.yPos = jointPos.y;
				item->Position.zPos = jointPos.z;
			}
			else
			{
				item->Position.xPos = LaraItem->Position.xPos;
				item->Position.yPos = jointPos.y;
				item->Position.zPos = LaraItem->Position.zPos;
				item->RoomNumber = LaraItem->RoomNumber;
			}

			InitialiseItem(itemNumber);

			item->Position.xRot = Lara.LeftArm.Rotation.xRot + LaraItem->Position.xRot;
			item->Position.zRot = 0;
			item->Position.yRot = Lara.LeftArm.Rotation.yRot + LaraItem->Position.yRot;

			if (!Lara.LeftArm.Locked)
			{
				item->Position.xRot += Lara.Control.ExtraTorsoRot.zRot;
				item->Position.yRot += Lara.Control.ExtraTorsoRot.yRot;
			}
		}

		item->Velocity = 512;

		AddActiveItem(itemNumber);

		item->ItemFlags[0] = Lara.Weapons[WEAPON_CROSSBOW].SelectedAmmo;

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

	Lara.Control.WeaponControl.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		item->ObjectNumber = ID_ROCKET;
		item->RoomNumber = LaraItem->RoomNumber;

		if (!ammos.hasInfinite())
			(ammos)--;

		PHD_VECTOR jointPos;
		jointPos.x = 0;
		jointPos.y = 180;
		jointPos.z = 72;

		GetLaraJointPosition(&jointPos, LM_RHAND);

		int x, y, z;
		item->Position.xPos = x = jointPos.x;
		item->Position.yPos = y = jointPos.y;
		item->Position.zPos = z = jointPos.z;

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

		item->Position.xRot = LaraItem->Position.xRot + Lara.LeftArm.Rotation.xRot;
		item->Position.yRot = LaraItem->Position.yRot + Lara.LeftArm.Rotation.yRot;
		item->Position.zRot = 0;

		if (!Lara.LeftArm.Locked)
		{
			item->Position.xRot += Lara.Control.ExtraTorsoRot.zRot;
			item->Position.yRot += Lara.Control.ExtraTorsoRot.yRot;
		}

		item->Velocity = 512 >> 5;
		item->ItemFlags[0] = 0;

		AddActiveItem(itemNumber);

		SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void DoExplosiveDamageOnBaddie(ITEM_INFO* dest, ITEM_INFO* src, int weapon)
{
	if (!(dest->Flags & 0x8000))
	{
		if (dest != LaraItem || LaraItem->HitPoints <= 0)
		{
			if (!src->ItemFlags[2])
			{
				dest->HitStatus = true;

				OBJECT_INFO* obj = &Objects[dest->ObjectNumber];
				// TODO: in TR4 condition was objectNumber != (ID_MUMMY, ID_SKELETON, ID_SETHA)
				if (!obj->undead)
				{
					HitTarget(LaraItem, dest, 0, Weapons[weapon].explosiveDamage, 1);
					if (dest != LaraItem)
					{
						Statistics.Game.AmmoHits++;
						if (dest->HitPoints <= 0)
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
			LaraItem->HitPoints -= (Weapons[weapon].damage * 5);
			if (!(g_Level.Rooms[dest->RoomNumber].flags & ENV_FLAG_WATER) && LaraItem->HitPoints <= Weapons[weapon].damage)
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
		int x = (GetRandomControl() & 0x1FF) + item->Position.xPos - 256;
		int y = item->Position.yPos;
		int z = (GetRandomControl() & 0x1FF) + item->Position.zPos - 256;
		
		TriggerExplosionBubbles(x, y, z, item->RoomNumber);
		TriggerExplosionSparks(x, y, z, 2, -1, 1, item->RoomNumber);
		
		int wh = GetWaterHeight(x, y, z, item->RoomNumber);
		if (wh != NO_HEIGHT)
			SomeSparkEffect(x, wh, z, 8);
	}
	else
	{
		TriggerExplosionBubble(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
		TriggerExplosionSparks(item->Position.xPos, item->Position.yPos, item->Position.zPos, 2, -2, 1, item->RoomNumber);

		for (int i = 0; i < 3; i++)
		{
			TriggerExplosionSparks(item->Position.xPos, item->Position.yPos, item->Position.zPos, 2, -1, 1, item->RoomNumber);
		}

		int wh = GetWaterHeight(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
		if (wh != NO_HEIGHT)
		{
			int dy = item->Position.yPos - wh;
			if (dy < 2048)
			{
				SplashSetup.y = wh;
				SplashSetup.x = item->Position.xPos;
				SplashSetup.z = item->Position.zPos;
				SplashSetup.innerRadius = 160;
				SplashSetup.splashPower = 2048 - dy;

				SetupSplash(&SplashSetup, item->RoomNumber);
			}
		}
	}
}

void undraw_shotgun(int weapon)
{
	ITEM_INFO* item = &g_Level.Items[Lara.Control.WeaponControl.WeaponItem];
	item->TargetState = 3;
	
	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
	{
		Lara.Control.HandStatus = HandStatus::Free;
		Lara.target = nullptr;
		Lara.RightArm.Locked = false;
		Lara.LeftArm.Locked = false;
		KillItem(Lara.Control.WeaponControl.WeaponItem);
		Lara.Control.WeaponControl.WeaponItem = NO_ITEM;
		Lara.RightArm.FrameNumber = 0;
		Lara.LeftArm.FrameNumber = 0;
	}
	else if (item->ActiveState == 3 && item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 21)
	{
		undraw_shotgun_meshes(weapon);
	}
	
	Lara.RightArm.FrameBase = g_Level.Anims[item->AnimNumber].framePtr;
	Lara.LeftArm.FrameBase = g_Level.Anims[item->AnimNumber].framePtr;
	Lara.RightArm.FrameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
	Lara.LeftArm.FrameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
	Lara.RightArm.AnimNumber = item->AnimNumber;
	Lara.LeftArm.AnimNumber = Lara.RightArm.AnimNumber;
}

void undraw_shotgun_meshes(int weapon)
{
	Lara.Control.WeaponControl.HolsterInfo.backHolster = HolsterSlotForWeapon(static_cast<LARA_WEAPON_TYPE>(weapon));
	Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
}

void draw_shotgun_meshes(int weaponType)
{
	Lara.Control.WeaponControl.HolsterInfo.backHolster = HOLSTER_SLOT::Empty;
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

	angles[1] = Lara.LeftArm.Rotation.xRot;
	angles[0] = Lara.LeftArm.Rotation.yRot + LaraItem->Position.yRot;

	if (!Lara.LeftArm.Locked)
	{
		angles[0] = Lara.Control.ExtraTorsoRot.yRot + Lara.LeftArm.Rotation.yRot + LaraItem->Position.yRot;
		angles[1] = Lara.Control.ExtraTorsoRot.zRot + Lara.LeftArm.Rotation.xRot;
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
		Lara.RightArm.FlashGun = Weapons[WEAPON_HK].flashTime;
	}
}

void FireShotgun()
{
	short angles[2];
	
	angles[1] = Lara.LeftArm.Rotation.xRot;
	angles[0] = Lara.LeftArm.Rotation.yRot + LaraItem->Position.yRot;

	if (!Lara.LeftArm.Locked)
	{
		angles[0] = Lara.Control.ExtraTorsoRot.yRot + Lara.LeftArm.Rotation.yRot + LaraItem->Position.yRot;
		angles[1] = Lara.Control.ExtraTorsoRot.zRot + Lara.LeftArm.Rotation.xRot;
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

		if (LaraItem->MeshBits != 0)
		{
			for (int i = 0; i < 7; i++)
				TriggerGunSmoke(pos2.x, pos2.y, pos2.z, pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z, 1, SmokeWeapon, SmokeCountL);
		}

		Lara.RightArm.FlashGun = Weapons[WEAPON_SHOTGUN].flashTime;
		
		SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->Position, 20971524);
		SoundEffect(Weapons[WEAPON_SHOTGUN].sampleNum, &LaraItem->Position, 0);
		
		Statistics.Game.AmmoUsed++;
	}
}

void ready_shotgun(int weaponType)
{
	Lara.Control.HandStatus = HandStatus::WeaponReady;
	Lara.LeftArm.Rotation.zRot = 0;
	Lara.LeftArm.Rotation.yRot = 0;
	Lara.LeftArm.Rotation.xRot = 0;
	Lara.RightArm.Rotation.zRot = 0;
	Lara.RightArm.Rotation.yRot = 0;
	Lara.RightArm.Rotation.xRot = 0;
	Lara.RightArm.FrameNumber = 0;
	Lara.LeftArm.FrameNumber = 0;
	Lara.RightArm.Locked = false;
	Lara.LeftArm.Locked = false;
	Lara.target = nullptr;
	Lara.RightArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
	Lara.LeftArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
}
