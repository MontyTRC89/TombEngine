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
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/input.h"
#include "Sound/sound.h"

using namespace TEN::Input;
using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Switches;
using namespace TEN::Effects::Environment;

enum class CrossbowBoltType
{
	Normal,
	Poison,
	Explosive
};

void AnimateShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	if (lara->LeftArm.GunSmoke > 0)
	{
		Vector3i pos;
		if (weaponType == LaraWeaponType::HK)
			pos = Vector3i(0, 228, 96);
		else if (weaponType == LaraWeaponType::Shotgun)
			pos = Vector3i(0, 228, 0);
		else if (weaponType == LaraWeaponType::GrenadeLauncher)
			pos = Vector3i(0, 180, 80);
		else if (weaponType == LaraWeaponType::RocketLauncher)
			pos = Vector3i(0, 84, 72);

		GetLaraJointPosition(&pos, LM_RHAND);

		if (laraItem->MeshBits)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weaponType, lara->LeftArm.GunSmoke);
	}

	auto* item = &g_Level.Items[lara->Control.Weapon.WeaponItem];
	bool running = (weaponType == LaraWeaponType::HK && laraItem->Animation.Velocity.z != 0.0f);
	
	static bool reloadHarpoonGun = false;
	reloadHarpoonGun = (lara->Weapons[(int)weaponType].Ammo->hasInfinite() || weaponType != LaraWeaponType::HarpoonGun) ? false : reloadHarpoonGun;

	switch (item->Animation.ActiveState)
	{
	case WEAPON_STATE_AIM:
		//HKFlag = 0;
		//HKTimer = 0;
		//HKFlag2 = 0;

		if (lara->Control.WaterStatus == WaterStatus::Underwater || running)
			item->Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
		else if ((!(TrInput & IN_ACTION) || lara->TargetEntity) && lara->LeftArm.Locked == false)
			item->Animation.TargetState = WEAPON_STATE_UNAIM;
		else
			item->Animation.TargetState = WEAPON_STATE_RECOIL;

		if (weaponType == LaraWeaponType::HarpoonGun && !lara->Weapons[(int)weaponType].Ammo->hasInfinite())
		{
			if (reloadHarpoonGun)
			{
				item->Animation.TargetState = WEAPON_STATE_RELOAD;
				reloadHarpoonGun = false;
			}
		}

		break;

	case WEAPON_STATE_UNDERWATER_AIM:
		//HKFlag = 0;
		//HKTimer = 0;
		//HKFlag2 = 0;

		if (lara->Control.WaterStatus == WaterStatus::Underwater || running)
		{
			if ((!(TrInput & IN_ACTION) || lara->TargetEntity) && lara->LeftArm.Locked == false)
				item->Animation.TargetState = WEAPON_STATE_UNDERWATER_UNAIM;
			else
				item->Animation.TargetState = WEAPON_STATE_UNDERWATER_RECOIL;
		}
		else
			item->Animation.TargetState = WEAPON_STATE_AIM;

		if (weaponType == LaraWeaponType::HarpoonGun && !lara->Weapons[(int)weaponType].Ammo->hasInfinite())
		{
			if (reloadHarpoonGun)
			{
				item->Animation.TargetState = WEAPON_STATE_RELOAD;
				reloadHarpoonGun = false;
			}
		}

		break;

	case WEAPON_STATE_RECOIL:
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
		{
			item->Animation.TargetState = WEAPON_STATE_UNAIM;

			if (lara->Control.WaterStatus != WaterStatus::Underwater && !running && !reloadHarpoonGun)
			{
				if (TrInput & IN_ACTION && (!lara->TargetEntity || lara->LeftArm.Locked))
				{
					if (weaponType == LaraWeaponType::HarpoonGun)
					{
						FireHarpoon(laraItem);

						if (!(lara->Weapons[(int)LaraWeaponType::HarpoonGun].Ammo->getCount() % 4) &&
							!lara->Weapons[(int)weaponType].Ammo->hasInfinite())
						{
							reloadHarpoonGun = true;
						}
					}
					else if (weaponType == LaraWeaponType::RocketLauncher)
						FireRocket(laraItem);
					else if (weaponType == LaraWeaponType::GrenadeLauncher)
						FireGrenade(laraItem);
					else if (weaponType == LaraWeaponType::Crossbow)
						FireCrossbow(laraItem, nullptr);
					else if (weaponType == LaraWeaponType::HK)
					{
						FireHK(laraItem, 0);

						if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
							SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 1.0f, 0.4f);
							SoundEffect(SFX_TR4_HK_FIRE, &laraItem->Pose);
						}
					}
					else
						FireShotgun(laraItem);

					item->Animation.TargetState = WEAPON_STATE_RECOIL;
				}
				else if (lara->LeftArm.Locked)
					item->Animation.TargetState = WEAPON_STATE_AIM;
			}


			if (item->Animation.TargetState != WEAPON_STATE_RECOIL && 
				weaponType == LaraWeaponType::HK &&
				!lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
			{
				StopSoundEffect(SFX_TR4_HK_FIRE);
				SoundEffect(SFX_TR4_HK_STOP, &laraItem->Pose);
			}
		}
		else if (weaponType == LaraWeaponType::Shotgun && !(TrInput & IN_ACTION) && !lara->LeftArm.Locked)
			item->Animation.TargetState = WEAPON_STATE_UNAIM;

		if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 12 &&
			weaponType == LaraWeaponType::Shotgun)
		{
			TriggerGunShell(1, ID_SHOTGUNSHELL, LaraWeaponType::Shotgun);
		}

		break;

	case WEAPON_STATE_UNDERWATER_RECOIL:
		if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 0)
		{
			item->Animation.TargetState = WEAPON_STATE_UNDERWATER_UNAIM;

			if ((lara->Control.WaterStatus == WaterStatus::Underwater || running) && !reloadHarpoonGun)
			{
				if (TrInput & IN_ACTION &&
					(!lara->TargetEntity || lara->LeftArm.Locked))
				{
					if (weaponType == LaraWeaponType::HarpoonGun)
					{
						FireHarpoon(laraItem);

						if (!(lara->Weapons[(int)LaraWeaponType::HarpoonGun].Ammo->getCount() % 4) &&
							!lara->Weapons[(int)weaponType].Ammo->hasInfinite())
						{
							reloadHarpoonGun = true;
						}
					}
					else if (weaponType == LaraWeaponType::HK)// && (/*!(lara->HKtypeCarried & 0x18) || */!HKTimer))
					{
						FireHK(laraItem, 1);
						//HKFlag = 1;
						item->Animation.TargetState = WEAPON_STATE_UNDERWATER_RECOIL;

						if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
							SoundEffect(SFX_TR4_HK_SILENCED, nullptr);
						else
						{
							SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 1.0f, 0.4f);
							SoundEffect(SFX_TR4_HK_FIRE, &laraItem->Pose);
						}
					}
					else
						item->Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;

					item->Animation.TargetState = WEAPON_STATE_UNDERWATER_RECOIL;
				}
				else if (lara->LeftArm.Locked)
					item->Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
			}
			else if (item->Animation.TargetState != WEAPON_STATE_UNDERWATER_RECOIL &&
				//HKFlag &&
				!(lara->Weapons[(int)LaraWeaponType::HK].HasSilencer))
			{
				StopSoundEffect(SFX_TR4_HK_FIRE);
				SoundEffect(SFX_TR4_HK_STOP, &laraItem->Pose);
				//HKFlag = 0;
			}
			/*else if (HKFlag)
			{
				if (lara->Weapons[(int)LaraWeaponType::HK].HasSilencer)
					SoundEffect(SFX_HK_SILENCED, nullptr);
				else
				{
					SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->pos, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_HK_FIRE, &laraItem->pos);
				}
			}*/
		}

		break;

	default:
		break;
	}

	AnimateItem(item);

	lara->LeftArm.FrameBase = lara->RightArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].framePtr;
	lara->LeftArm.FrameNumber = lara->RightArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->LeftArm.AnimNumber = lara->RightArm.AnimNumber = item->Animation.AnimNumber;
}

void ReadyShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.HandStatus = HandStatus::WeaponReady;
	lara->LeftArm.Orientation = Vector3Shrt();
	lara->RightArm.Orientation = Vector3Shrt();
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
	lara->LeftArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
	lara->RightArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
	lara->TargetEntity = nullptr;
}

void FireShotgun(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto armOrient = Vector3Shrt(
		lara->LeftArm.Orientation.x,
		lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
		0
	);

	if (!lara->LeftArm.Locked)
	{
		armOrient = Vector3Shrt(
			lara->ExtraTorsoRot.x + lara->LeftArm.Orientation.x,
			lara->ExtraTorsoRot.y + lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
			0
		);
	}

	int value = (lara->Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo == WeaponAmmoType::Ammo1 ? 1820 : 5460);
	bool fired = false;
	for (int i = 0; i < 6; i++)
	{
		auto wobbleArmOrient = Vector3Shrt(
			armOrient.x + value * (GetRandomControl() - ANGLE(90.0f)) / 65536,
			armOrient.y + value * (GetRandomControl() - ANGLE(90.0f)) / 65536,
			0
		);

		if (FireWeapon(LaraWeaponType::Shotgun, lara->TargetEntity, laraItem, wobbleArmOrient) != FireWeaponType::NoAmmo)
			fired = true;
	}

	if (fired)
	{
		auto pos = Vector3i(0, 228, 32);
		GetLaraJointPosition(&pos, LM_RHAND);

		auto pos2 = pos;

		pos = Vector3i(0, 1508, 32);
		GetLaraJointPosition(&pos, LM_RHAND);

		lara->LeftArm.GunSmoke = 32;

		if (laraItem->MeshBits != 0)
		{
			for (int i = 0; i < 7; i++)
				TriggerGunSmoke(pos2.x, pos2.y, pos2.z, pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z, 1, LaraWeaponType::Shotgun, lara->LeftArm.GunSmoke);
		}

		lara->RightArm.GunFlash = Weapons[(int)LaraWeaponType::Shotgun].FlashTime;

		SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, TestEnvironment(ENV_FLAG_WATER, laraItem) ? SoundEnvironment::Water : SoundEnvironment::Land);
		SoundEffect(Weapons[(int)LaraWeaponType::Shotgun].SampleNum, &laraItem->Pose);

		Rumble(0.5f, 0.2f);

		Statistics.Game.AmmoUsed++;
	}
}

void DrawShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	ItemInfo* item;

	if (lara->Control.Weapon.WeaponItem == NO_ITEM)
	{
		lara->Control.Weapon.WeaponItem = CreateItem();
		item = &g_Level.Items[lara->Control.Weapon.WeaponItem];
		item->ObjectNumber = WeaponObject(weaponType);

		if (weaponType == LaraWeaponType::RocketLauncher)
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
		else if (weaponType == LaraWeaponType::GrenadeLauncher)
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 0;
		else
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = WEAPON_STATE_DRAW;
		item->Animation.ActiveState = WEAPON_STATE_DRAW;
		item->Status = ITEM_ACTIVE;
		item->RoomNumber = NO_ROOM;

		lara->RightArm.FrameBase = Objects[item->ObjectNumber].frameBase;
		lara->LeftArm.FrameBase = lara->RightArm.FrameBase;
	}
	else
		item = &g_Level.Items[lara->Control.Weapon.WeaponItem];

	AnimateItem(item);

	if (item->Animation.ActiveState != WEAPON_STATE_AIM &&
		item->Animation.ActiveState != WEAPON_STATE_UNDERWATER_AIM)
	{
		if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == Weapons[(int)weaponType].DrawFrame)
			DrawShotgunMeshes(laraItem, weaponType);
		else if (lara->Control.WaterStatus == WaterStatus::Underwater)
			item->Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
	}
	else
		ReadyShotgun(laraItem, weaponType);

	lara->LeftArm.FrameBase = lara->RightArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].framePtr;
	lara->LeftArm.FrameNumber = lara->RightArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->LeftArm.AnimNumber = lara->RightArm.AnimNumber = item->Animation.AnimNumber;
}

void UndrawShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	auto* item = &g_Level.Items[lara->Control.Weapon.WeaponItem];
	item->Animation.TargetState = WEAPON_STATE_UNDRAW;

	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
	{
		lara->Control.HandStatus = HandStatus::Free;
		lara->TargetEntity = nullptr;
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
		KillItem(lara->Control.Weapon.WeaponItem);
		lara->Control.Weapon.WeaponItem = NO_ITEM;
		lara->RightArm.FrameNumber = 0;
		lara->LeftArm.FrameNumber = 0;
	}
	else if (item->Animation.ActiveState == WEAPON_STATE_UNDRAW)
	{
		if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 21 ||
			(weaponType == LaraWeaponType::GrenadeLauncher && item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 15))
		{
			UndrawShotgunMeshes(laraItem, weaponType);
		}
	}

	lara->RightArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].framePtr;
	lara->LeftArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].framePtr;
	lara->RightArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->LeftArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->RightArm.AnimNumber = item->Animation.AnimNumber;
	lara->LeftArm.AnimNumber = lara->RightArm.AnimNumber;
}

void DrawShotgunMeshes(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;
	lara->MeshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(laraItem, weaponType)].meshIndex + LM_RHAND;
}

void UndrawShotgunMeshes(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlotForWeapon(weaponType);
	lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
}

void FireHarpoon(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto& ammos = GetAmmo(laraItem, LaraWeaponType::HarpoonGun);
	if (!ammos)
		return;

	lara->Control.Weapon.HasFired = true;

	// Create a new item for harpoon
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		if (!ammos.hasInfinite())
			(ammos)--;

		auto* item = &g_Level.Items[itemNumber];

		item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		item->ObjectNumber = ID_HARPOON;
		item->RoomNumber = laraItem->RoomNumber;

		auto jointPos = Vector3i(-2, 373, 77);
		GetLaraJointPosition(&jointPos, LM_RHAND);

		int floorHeight = GetCollision(jointPos.x, jointPos.y, jointPos.z, item->RoomNumber).Position.Floor;
		if (floorHeight >= jointPos.y)
			item->Pose.Position = jointPos;
		else
		{
			item->Pose.Position = Vector3i(laraItem->Pose.Position.x, jointPos.y, laraItem->Pose.Position.z);
			item->RoomNumber = laraItem->RoomNumber;
		}

		InitialiseItem(itemNumber);

		item->Pose.Orientation.x = lara->LeftArm.Orientation.x + laraItem->Pose.Orientation.x;
		item->Pose.Orientation.y = lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y;
		item->Pose.Orientation.z = 0;

		if (!lara->LeftArm.Locked)
		{
			item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
			item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
		}

		item->Pose.Orientation.z = 0;
		item->Animation.Velocity.z = HARPOON_VELOCITY * phd_cos(item->Pose.Orientation.x);
		item->Animation.Velocity.y = -HARPOON_VELOCITY * phd_sin(item->Pose.Orientation.x);
		item->HitPoints = HARPOON_TIME;

		Rumble(0.2f, 0.1f);
		AddActiveItem(itemNumber);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void HarpoonBoltControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	// Store old position for later
	auto oldPos = item->Pose.Position;
	bool aboveWater = false;

	// Update speed and check if above water
	if (item->HitPoints == HARPOON_TIME)
	{
		item->Pose.Orientation.z += ANGLE(35.0f);
		if (!TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
		{
			item->Pose.Orientation.x -= ANGLE(1.0f);

			if (item->Pose.Orientation.x < -ANGLE(90.0f))
				item->Pose.Orientation.x = -ANGLE(90.0f);

			item->Animation.Velocity.y = -HARPOON_VELOCITY * phd_sin(item->Pose.Orientation.x);
			item->Animation.Velocity.z = HARPOON_VELOCITY * phd_cos(item->Pose.Orientation.x);
			aboveWater = true;
		}
		else
		{
			// Create bubbles
			if (Wibble & 4)
				CreateBubble((Vector3i*)&item->Pose, item->RoomNumber, 0, 0, BUBBLE_FLAG_CLUMP | BUBBLE_FLAG_HIGH_AMPLITUDE, 0, 0, 0);
			
			item->Animation.Velocity.y = -HARPOON_VELOCITY * phd_sin(item->Pose.Orientation.x) / 2;
			item->Animation.Velocity.z = HARPOON_VELOCITY * phd_cos(item->Pose.Orientation.x) / 2;
			aboveWater = false;
		}

		TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.z);
	}
	else
	{
		if (item->HitPoints > 0)
			item->HitPoints--;
		else
		{
			ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
			KillItem(itemNumber);
		}

		return;
	}

	auto probe = GetCollision(item);

	// Check if bolt has hit a solid wall
	if (probe.Position.Floor < item->Pose.Position.y ||
		probe.Position.Ceiling > item->Pose.Position.y)
	{
		// I have hit a solid wall, this is the end for the bolt
		item->HitPoints--;
		return;
	}

	// Has harpoon changed room?
	if (item->RoomNumber != probe.RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	// If now in water and before in land, add a ripple
	if (TestEnvironment(ENV_FLAG_WATER, item) && aboveWater)
		SetupRipple(item->Pose.Position.x, g_Level.Rooms[item->RoomNumber].minfloor, item->Pose.Position.z, (GetRandomControl() & 7) + 8, 0);

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
		auto* currentItem = CollidedItems[0];
		
		int k = 0;
		do
		{
			auto* currentObj = &Objects[currentItem->ObjectNumber];

			if (!currentObj->isPickup && currentObj->collision && currentItem->Collidable)
				foundCollidedObjects = true;

			if (currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE && !currentObj->undead)
			{
				explodeItem = false;
				HitTarget(LaraItem, currentItem, (GameVector*)&item->Pose, Weapons[(int)LaraWeaponType::HarpoonGun].Damage, 0);
			}

			// All other items (like puzzles) can't be hit
			k++;
			currentItem = CollidedItems[k];

		} while (currentItem);
	}

	if (CollidedMeshes[0])
	{
		auto* currentMesh = CollidedMeshes[0];

		int k = 0;
		do
		{
			auto* s = &StaticObjects[currentMesh->staticNumber];
			if (s->shatterType != SHT_NONE)
			{
				currentMesh->HitPoints -= Weapons[(int)LaraWeaponType::Crossbow].Damage;
				if (currentMesh->HitPoints <= 0)
				{
					TriggerExplosionSparks(currentMesh->pos.Position.x, currentMesh->pos.Position.y, currentMesh->pos.Position.z, 3, -2, 0, item->RoomNumber);
					auto pos = PHD_3DPOS(currentMesh->pos.Position.x, currentMesh->pos.Position.y - 128, currentMesh->pos.Position.z, 0, currentMesh->pos.Orientation.y, 0);
					TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
					ShatterObject(nullptr, currentMesh, -128, item->RoomNumber, 0);
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
			ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
		KillItem(itemNumber);
	}
}

void FireGrenade(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	int x = 0;
	int y = 0;
	int z = 0;
	
	Ammo& ammo = GetAmmo(laraItem, LaraWeaponType::GrenadeLauncher);
	if (!ammo)
		return;

	lara->Control.Weapon.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber];
		
		item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		item->ObjectNumber = ID_GRENADE;
		item->RoomNumber = laraItem->RoomNumber;

		auto jointPos = Vector3i(0, 276, 80);
		GetLaraJointPosition(&jointPos, LM_RHAND);

		item->Pose.Position.x = x = jointPos.x;
		item->Pose.Position.y = y = jointPos.y;
		item->Pose.Position.z = z = jointPos.z;

		int floorHeight = GetCollision(jointPos.x, jointPos.y, jointPos.z, item->RoomNumber).Position.Floor;
		if (floorHeight < jointPos.y)
		{
			item->Pose.Position.x = laraItem->Pose.Position.x;
			item->Pose.Position.y = jointPos.y;
			item->Pose.Position.z = laraItem->Pose.Position.z;
			item->RoomNumber = laraItem->RoomNumber;
		}

		jointPos = Vector3i(0, 1204, 5);
		GetLaraJointPosition(&jointPos, LM_RHAND);

		lara->LeftArm.GunSmoke = 32;

		if (laraItem->MeshBits)
		{
			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(x, y, z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 1, LaraWeaponType::GrenadeLauncher, lara->LeftArm.GunSmoke);
		}

		InitialiseItem(itemNumber);

		item->Pose.Orientation.x = laraItem->Pose.Orientation.x + lara->LeftArm.Orientation.x;
		item->Pose.Orientation.y = laraItem->Pose.Orientation.y + lara->LeftArm.Orientation.y;
		item->Pose.Orientation.z = 0;

		if (!lara->LeftArm.Locked)
		{
			item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
			item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
		}

		item->Animation.Velocity.z = GRENADE_VELOCITY;
		item->Animation.Velocity.y = -CLICK(2) * phd_sin(item->Pose.Orientation.x);
		item->Animation.ActiveState = item->Pose.Orientation.x;
		item->Animation.TargetState = item->Pose.Orientation.y;
		item->Animation.RequiredState = 0;
		item->HitPoints = GRENADE_TIME;
		item->ItemFlags[0] = (int)WeaponAmmoType::Ammo2;

		Rumble(0.4f, 0.25f);

		AddActiveItem(itemNumber);

		if (!ammo.hasInfinite())
			(ammo)--;

		item->ItemFlags[0] = (int)lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo;

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void GrenadeControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ItemFlags[1])
	{
		item->ItemFlags[1]--;

		if (item->ItemFlags[1])
		{
			if (item->ItemFlags[0] == (int)(int)GrenadeType::Flash)
			{
				// Flash grenades
				int R, G, B;
				if (item->ItemFlags[1] == 1)
				{
					FlashGrenadeAftershockTimer = 120;
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

				TriggerFlashSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
				TriggerFlashSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
			}
			else
			{
				// Trigger a new grenade in the case of GRENADE_SUPER until itemFlags[1] is > 0
				short newGrenadeItemNumber = CreateItem();
				if (newGrenadeItemNumber != NO_ITEM)
				{
					auto* newGrenade = &g_Level.Items[newGrenadeItemNumber];

					newGrenade->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
					newGrenade->ObjectNumber = ID_GRENADE;
					newGrenade->RoomNumber = item->RoomNumber;
					newGrenade->Pose.Position.x = (GetRandomControl() & 0x1FF) + item->Pose.Position.x - 256;
					newGrenade->Pose.Position.y = item->Pose.Position.y - 256;
					newGrenade->Pose.Position.z = (GetRandomControl() & 0x1FF) + item->Pose.Position.z - 256;
					
					InitialiseItem(newGrenadeItemNumber);
					
					newGrenade->Pose.Orientation.x = (GetRandomControl() & 0x3FFF) + ANGLE(45);
					newGrenade->Pose.Orientation.y = GetRandomControl() * 2;
					newGrenade->Pose.Orientation.z = 0;
					newGrenade->Animation.Velocity.z = 64.0f;
					newGrenade->Animation.Velocity.y = -64.0f * phd_sin(newGrenade->Pose.Orientation.x);
					newGrenade->Animation.ActiveState = newGrenade->Pose.Orientation.x;
					newGrenade->Animation.TargetState = newGrenade->Pose.Orientation.y;
					newGrenade->Animation.RequiredState = 0;
					
					AddActiveItem(newGrenadeItemNumber);
					
					newGrenade->Status = ITEM_INVISIBLE;
					newGrenade->ItemFlags[2] = item->ItemFlags[2];
					newGrenade->HitPoints = 3000; // 60; // 3000;
					newGrenade->ItemFlags[0] = (int)(int)GrenadeType::Ultra;

					if (TestEnvironment(ENV_FLAG_WATER, newGrenade->RoomNumber))
						newGrenade->HitPoints = 1;
				}
			}

			return;
		}

		KillItem(itemNumber);
		return;
	}
	   
	// Store old position for later
	auto oldPos = item->Pose.Position;

	item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

	// Check if above water and update velocity and vertical velocity
	bool aboveWater = false;
	bool someFlag = false;
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) ||
		TestEnvironment(ENV_FLAG_SWAMP, item->RoomNumber))
	{
		aboveWater = false;
		someFlag = false;
		item->Animation.Velocity.y += (5.0f - item->Animation.Velocity.y) / 2.0f;
		item->Animation.Velocity.z -= item->Animation.Velocity.z / 4.0f;

		if (item->Animation.Velocity.z)
		{
			item->Pose.Orientation.z += (short((item->Animation.Velocity.z / 16.0f) + 3.0f) * ANGLE(1.0f));
			if (item->Animation.RequiredState)
				item->Pose.Orientation.y += (short((item->Animation.Velocity.z / 4.0f) + 3.0f) * ANGLE(1.0f));
			else
				item->Pose.Orientation.x += (short((item->Animation.Velocity.z / 4.0f) + 3.0f) * ANGLE(1.0f));
		}
	}
	else
	{
		aboveWater = true;
		someFlag = true;
		item->Animation.Velocity.y += 3.0f;

		if (item->Animation.Velocity.z)
		{
			item->Pose.Orientation.z += (short((item->Animation.Velocity.z / 4.0f) + 7.0f) * ANGLE(1.0f));
			if (item->Animation.RequiredState)
				item->Pose.Orientation.y += (short((item->Animation.Velocity.z / 2.0f) + 7.0f) * ANGLE(1.0f));
			else
				item->Pose.Orientation.x += (short((item->Animation.Velocity.z / 2.0f) + 7.0f) * ANGLE(1.0f));
		}
	}

	// Trigger fire and smoke sparks in the direction of motion
	if (item->Animation.Velocity.z && aboveWater)
	{
		Matrix world = Matrix::CreateFromYawPitchRoll(
			TO_RAD(item->Pose.Orientation.y - ANGLE(180.0f)),
			TO_RAD(item->Pose.Orientation.x),
			TO_RAD(item->Pose.Orientation.z)
		) * Matrix::CreateTranslation(0, 0, -64);

		int wx = world.Translation().x;
		int wy = world.Translation().y;
		int wz = world.Translation().z;

		TriggerRocketSmoke(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z, -1);
		TriggerRocketFire(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z);
	}

	// Update grenade position
	auto velocity = Vector3i(
		item->Animation.Velocity.z * phd_sin(item->Animation.TargetState),
		item->Animation.Velocity.y,
		item->Animation.Velocity.z * phd_cos(item->Animation.TargetState)
	);
	item->Pose.Position += velocity;

	// Grenades that originate from first grenade when special ammo is selected
	if (item->ItemFlags[0] == (int)GrenadeType::Ultra)
	{
		auto probe = GetCollision(item);
		if (probe.Position.Floor < item->Pose.Position.y ||
			probe.Position.Ceiling > item->Pose.Position.y)
		{
			item->HitPoints = 1;
		}
	}
	else
	{
		// Do grenade's physics
		short sYrot = item->Pose.Orientation.y;
		item->Pose.Orientation.y = item->Animation.TargetState;

		DoProjectileDynamics(itemNumber, oldPos.x, oldPos.y, oldPos.z, velocity.x, velocity.y, velocity.z);

		item->Animation.TargetState = item->Pose.Orientation.y;
		item->Pose.Orientation.y = sYrot;
	}

	short probedRoomNumber = GetCollision(item).RoomNumber;

	if (item->ItemFlags[0] == (int)GrenadeType::Ultra)
		TriggerFireFlame(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, -1, 1);

	// Check if it's time to explode
	int radius = 0;
	bool explode = false; 

	if (item->HitPoints)
	{
		item->HitPoints--;

		if (item->HitPoints)
		{
			if (item->HitPoints > EXPLOSION_TRIGGER_TIME)
				return;
		}
		else
		{
			radius = GRENADE_EXPLODE_RADIUS;
			explode = true;
		}
	}

	// If is not a flash grenade then try to destroy surrounding objects
	if (!(item->ItemFlags[0] == (int)GrenadeType::Flash && explode))
	{
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
					if (CollidedItems[i] == nullptr)
						break;

					auto* currentItem = CollidedItems[i];

					if (currentItem->ObjectNumber < ID_SMASH_OBJECT1 || currentItem->ObjectNumber > ID_SMASH_OBJECT16)
					{
						if (currentItem->ObjectNumber < ID_SHOOT_SWITCH1 || currentItem->ObjectNumber > ID_SHOOT_SWITCH4 || (currentItem->Flags & 0x40))
						{
							if (Objects[currentItem->ObjectNumber].intelligent || currentItem->ObjectNumber == ID_LARA)
								DoExplosiveDamageOnBaddy(LaraItem, currentItem, item, LaraWeaponType::GrenadeLauncher);
						}
						else
						{
							if ((currentItem->Flags & IFLAG_ACTIVATION_MASK) &&
								(currentItem->Flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
							{
								TestTriggers(currentItem->Pose.Position.x, currentItem->Pose.Position.y - CLICK(1), currentItem->Pose.Position.z, probedRoomNumber, true, IFLAG_ACTIVATION_MASK);
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
								ExplodeItemNode(currentItem, Objects[currentItem->ObjectNumber].nmeshes - 1, 0, 64);

							AddActiveItem(currentItem - g_Level.Items.data());

							currentItem->Status = ITEM_ACTIVE;
							currentItem->Flags |= 0x3E40;
						}
					}
					else
					{
						// Smash objects are legacy objects from TRC, let's make them explode in the legacy way
						TriggerExplosionSparks(currentItem->Pose.Position.x, currentItem->Pose.Position.y, currentItem->Pose.Position.z, 3, -2, 0, currentItem->RoomNumber);
						auto pos = PHD_3DPOS(currentItem->Pose.Position.x, currentItem->Pose.Position.y - 128, currentItem->Pose.Position.z);
						TriggerShockwave(&pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);
						ExplodeItemNode(currentItem, 0, 0, 128);
						short currentItemNumber = (currentItem - CollidedItems[0]);
						SmashObject(currentItemNumber);
						KillItem(currentItemNumber);
					}
				}

				if (CollidedMeshes[0])
				{
					auto* currentMesh = CollidedMeshes[0];
					int k = 0;

					do
					{
						auto* s = &StaticObjects[currentMesh->staticNumber];
						if (s->shatterType != SHT_NONE)
						{
							currentMesh->HitPoints -= Weapons[(int)LaraWeaponType::GrenadeLauncher].Damage;
							if (currentMesh->HitPoints <= 0)
							{
								TriggerExplosionSparks(currentMesh->pos.Position.x, currentMesh->pos.Position.y, currentMesh->pos.Position.z, 3, -2, 0, item->RoomNumber);
								auto pos = PHD_3DPOS(currentMesh->pos.Position.x, currentMesh->pos.Position.y - 128, currentMesh->pos.Position.z, 0, currentMesh->pos.Orientation.y, 0);
								TriggerShockwave(&pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
								ShatterObject(nullptr, currentMesh, -128, item->RoomNumber, 0);
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
				if (item->ItemFlags[0] == (int)GrenadeType::Flash)
					break;

				radius = GRENADE_HIT_RADIUS;
			}
		}
	}

	// Handle explosion effects
	if (explode || (item->ItemFlags[0] == (int)GrenadeType::Flash && explode))
	{
		if (item->ItemFlags[0] == (int)GrenadeType::Flash)
		{
			Weather.Flash(255, 255, 255, 0.03f);
			TriggerFlashSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
			TriggerFlashSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
		}
		else if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			item->Pose.Position.y -= 128;
			TriggerShockwave(&item->Pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);

			TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->RoomNumber);
			for (int x = 0; x < 2; x++)
				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->RoomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose);

		// Setup the counter for spawned grenades in the case of flash and super grenades ammos
		if (item->ItemFlags[0] != (int)GrenadeType::Normal && item->ItemFlags[0] != (int)GrenadeType::Ultra)
		{
			item->MeshBits = 0;
			item->ItemFlags[1] = (item->ItemFlags[0] != (int)GrenadeType::Super ? 16 : 4);
			return;
		}

		KillItem(itemNumber);
		return;
	}
}

void FireRocket(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto& ammos = GetAmmo(laraItem, LaraWeaponType::RocketLauncher);
	if (!ammos)
		return;

	lara->Control.Weapon.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->ObjectNumber = ID_ROCKET;
		item->RoomNumber = laraItem->RoomNumber;

		if (!ammos.hasInfinite())
			(ammos)--;

		auto jointPos = Vector3i(0, 180, 72);
		GetLaraJointPosition(&jointPos, LM_RHAND);

		int x, y, z;
		item->Pose.Position.x = x = jointPos.x;
		item->Pose.Position.y = y = jointPos.y;
		item->Pose.Position.z = z = jointPos.z;

		jointPos = Vector3i(0, 2004, 72);
		GetLaraJointPosition(&jointPos, LM_RHAND);

		lara->LeftArm.GunSmoke = 32;

		for (int i = 0; i < 5; i++)
			TriggerGunSmoke(x, y, z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 1, LaraWeaponType::RocketLauncher, lara->LeftArm.GunSmoke);

		jointPos = Vector3i(0, -256, 0);
		GetLaraJointPosition(&jointPos, LM_RHAND);

		for (int i = 0; i < 10; i++)
			TriggerGunSmoke(jointPos.x, jointPos.y, jointPos.z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 2, LaraWeaponType::RocketLauncher, 32);

		InitialiseItem(itemNumber);

		item->Pose.Orientation.x = laraItem->Pose.Orientation.x + lara->LeftArm.Orientation.x;
		item->Pose.Orientation.y = laraItem->Pose.Orientation.y + lara->LeftArm.Orientation.y;
		item->Pose.Orientation.z = 0;
		item->HitPoints = ROCKET_TIME;

		if (!lara->LeftArm.Locked)
		{
			item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
			item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
		}

		item->Animation.Velocity.z = 512.0f / 32.0f;
		item->ItemFlags[0] = 0;

		AddActiveItem(itemNumber);

		Rumble(0.4f, 0.3f);
		SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void RocketControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	// Save old position for later
	auto oldPos = item->Pose.Position;
	short oldRoom = item->RoomNumber;

	// Update speed and rotation and check if above water or underwater
	bool abovewater = false;
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
	{
		if (item->Animation.Velocity.z > (ROCKET_VELOCITY / 4.0f))
			item->Animation.Velocity.z -= item->Animation.Velocity.z / 4.0f;
		else
		{
			item->Animation.Velocity.z += (item->Animation.Velocity.z / 4.0f) + 4.0f;

			if (item->Animation.Velocity.z > (ROCKET_VELOCITY / 4.0f))
				item->Animation.Velocity.z = ROCKET_VELOCITY / 4.0f;
		}

		item->Pose.Orientation.z += (short((item->Animation.Velocity.z / 8.0f) + 3.0f) * ANGLE(1.0f));
		abovewater = false;
	}
	else
	{
		if (item->Animation.Velocity.z < ROCKET_VELOCITY)
			item->Animation.Velocity.z += (item->Animation.Velocity.z / 4.0f) + 4.0f;

		item->Pose.Orientation.z += (short((item->Animation.Velocity.z / 4.0f) + 7.0f) * ANGLE(1.0f));
		abovewater = true;
	}

	item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

	// Calculate offset in rocket direction for fire and smoke sparks
	Matrix world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Pose.Orientation.y - ANGLE(180.0f)),
		TO_RAD(item->Pose.Orientation.x),
		TO_RAD(item->Pose.Orientation.z)
	) * Matrix::CreateTranslation(0, 0, -64);

	int wx = world.Translation().x;
	int wy = world.Translation().y;
	int wz = world.Translation().z;

	// Trigger fire, smoke and lighting
	TriggerRocketSmoke(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z, -1);
	TriggerRocketFire(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z);
	TriggerDynamicLight(wx + item->Pose.Position.x + (GetRandomControl() & 15) - 8, 
						wy + item->Pose.Position.y + (GetRandomControl() & 15) - 8, 
						wz + item->Pose.Position.z + (GetRandomControl() & 15) - 8, 
						14, 28 + (GetRandomControl() & 3), 16 + (GetRandomControl() & 7), (GetRandomControl() & 7));

	// If underwater generate bubbles
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
	{
		auto pos = Vector3i(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z);
		CreateBubble(&pos, item->RoomNumber, 4, 8, 0, 0, 0, 0);
	}

	// Update rocket's position
	short speed = item->Animation.Velocity.z * phd_cos(item->Pose.Orientation.x);
	item->Pose.Position.x += speed * phd_sin(item->Pose.Orientation.y);
	item->Pose.Position.y += -item->Animation.Velocity.z * phd_sin(item->Pose.Orientation.x);
	item->Pose.Position.z += speed * phd_cos(item->Pose.Orientation.y);

	bool explode = false;
	bool hitRoom = false;
	
	// Check if solid wall and then decide if explode or not
	auto probe = GetCollision(item);
	if (probe.Position.Floor < item->Pose.Position.y ||
		probe.Position.Ceiling > item->Pose.Position.y)
	{
		item->Pose.Position = oldPos;
		hitRoom = true;
	}

	// Has bolt changed room?
	if (item->RoomNumber != probe.RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	// If now in water and before in land, add a ripple
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) && abovewater)
		SetupRipple(item->Pose.Position.x, g_Level.Rooms[item->RoomNumber].minfloor, item->Pose.Position.z, (GetRandomControl() & 7) + 8, 0);

	int radius = (explode ? ROCKET_EXPLODE_RADIUS : ROCKET_HIT_RADIUS);
	bool foundCollidedObjects = false;

	// Decrease launch timer
	if (item->HitPoints)
		item->HitPoints--;

	for (int n = 0; n < 2; n++)
	{
		// Step 0: check for specific collision in a small radius
		// Step 1: done only if explosion, try to smash all objects in the blast radius

		// Found possible collided items and statics
		GetCollidedObjects(item, radius, true, &CollidedItems[0], &CollidedMeshes[0], false);

		// If no collided items and meshes are found, then exit the loop
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		if (CollidedItems[0])
		{
			auto* currentItem = CollidedItems[0];
			
			int k = 0;
			do
			{
				auto* currentObj = &Objects[currentItem->ObjectNumber];

				if ((currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE) ||
					currentItem->ObjectNumber == ID_LARA || (currentItem->Flags & 0x40 &&
					(Objects[currentItem->ObjectNumber].explodableMeshbits || currentItem == LaraItem)))
				{
					// All active intelligent creatures explode, if their HP is <= 0
					// Explosion is handled by CreatureDie()
					// Also Lara can be damaged, if enough time has passed or missile has hit the room.
					// HitTarget() is called inside this

					if (currentItem != LaraItem || (hitRoom || item->HitPoints < EXPLOSION_TRIGGER_TIME))
					{
						DoExplosiveDamageOnBaddy(LaraItem, currentItem, item, LaraWeaponType::RocketLauncher);
						explode = true;
					}
				}
				else if (currentItem->ObjectNumber >= ID_SMASH_OBJECT1 && currentItem->ObjectNumber <= ID_SMASH_OBJECT8)
				{
					// Smash objects are legacy objects from TRC, let's make them explode in the legacy way
					TriggerExplosionSparks(currentItem->Pose.Position.x, currentItem->Pose.Position.y, currentItem->Pose.Position.z, 3, -2, 0, currentItem->RoomNumber);
					auto pose = PHD_3DPOS(currentItem->Pose.Position.x, currentItem->Pose.Position.y - 128, currentItem->Pose.Position.z);
					TriggerShockwave(&pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);
					ExplodeItemNode(currentItem, 0, 0, 128);
					short currentItemNumber = (currentItem - CollidedItems[0]);
					SmashObject(currentItemNumber);
					KillItem(currentItemNumber);
					explode = true;
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
			auto* currentMesh = CollidedMeshes[0];
			int k = 0;

			do
			{
				auto* s = &StaticObjects[currentMesh->staticNumber];
				if (s->shatterType != SHT_NONE)
				{
					currentMesh->HitPoints -= Weapons[(int)LaraWeaponType::RocketLauncher].Damage;
					if (currentMesh->HitPoints <= 0)
					{
						TriggerExplosionSparks(currentMesh->pos.Position.x, currentMesh->pos.Position.y, currentMesh->pos.Position.z, 3, -2, 0, item->RoomNumber);
						auto pose = PHD_3DPOS(currentMesh->pos.Position.x, currentMesh->pos.Position.y - 128, currentMesh->pos.Position.z, 0, currentMesh->pos.Orientation.y, 0);
						TriggerShockwave(&pose, 40, 176, 64, 0, 96, 128, 16, 0, 0);
						ShatterObject(nullptr, currentMesh, -128, item->RoomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->RoomNumber;
						SmashedMesh[SmashedMeshCount] = currentMesh;
						SmashedMeshCount++;
						currentMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
					}
				}

				explode = true;
				k++;
				currentMesh = CollidedMeshes[k];

			} while (currentMesh);
		}

		radius = ROCKET_EXPLODE_RADIUS;
	}

	// Do explosion if needed
	if (hitRoom || explode)
	{
		if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			TriggerShockwave(&item->Pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);
			item->Pose.Position.y += 128;
			TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->RoomNumber);
			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->RoomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose);

		ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
		KillItem(itemNumber);
	}
}

void FireCrossbow(ItemInfo* laraItem, PHD_3DPOS* pos)
{
	auto* lara = GetLaraInfo(laraItem);

	auto& ammos = GetAmmo(laraItem, LaraWeaponType::Crossbow);
	if (!ammos)
		return;

	lara->Control.Weapon.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->ObjectNumber = ID_CROSSBOW_BOLT;
		item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

		if (!ammos.hasInfinite())
			(ammos)--;

		if (pos)
		{
			item->Pose.Position = pos->Position;
			item->RoomNumber = laraItem->RoomNumber;

			InitialiseItem(itemNumber);

			item->Pose.Orientation = pos->Orientation;
		}
		else
		{
			auto jointPos = Vector3i(0, 228, 32);
			GetLaraJointPosition(&jointPos, LM_RHAND);

			item->RoomNumber = laraItem->RoomNumber;

			int floorHeight = GetCollision(jointPos.x, jointPos.y, jointPos.z, item->RoomNumber).Position.Floor;
			if (floorHeight >= jointPos.y)
				item->Pose.Position = jointPos;
			else
			{
				item->Pose.Position = Vector3i(laraItem->Pose.Position.x, jointPos.y, laraItem->Pose.Position.z);
				item->RoomNumber = laraItem->RoomNumber;
			}

			InitialiseItem(itemNumber);

			item->Pose.Orientation.x = lara->LeftArm.Orientation.x + laraItem->Pose.Orientation.x;
			item->Pose.Orientation.z = 0;
			item->Pose.Orientation.y = lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y;

			if (!lara->LeftArm.Locked)
			{
				item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
				item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
			}
		}

		item->Animation.Velocity.z = 512.0f;

		AddActiveItem(itemNumber);

		item->ItemFlags[0] = (int)lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo;

		Rumble(0.2f, 0.1f);
		SoundEffect(SFX_TR4_CROSSBOW_FIRE, &laraItem->Pose);

		Statistics.Level.AmmoUsed++;
		Statistics.Game.AmmoUsed++;
	}
}

void FireCrossBowFromLaserSight(ItemInfo* laraItem, GameVector* src, GameVector* target)
{
	/* this part makes arrows fire at bad angles
	target->x &= ~1023;
	target->z &= ~1023;
	target->x |= 512;
	target->z |= 512;*/

	auto angles = GetOrientTowardPoint(
		Vector3(src->x, src->y, src->z),
		Vector3(target->x, target->y, target->z));
	auto boltPose = PHD_3DPOS(src->x, src->y, src->z, angles);
	FireCrossbow(laraItem, &boltPose);
}

void CrossbowBoltControl(short itemNumber)
{
	auto* lara = GetLaraInfo(LaraItem);
	auto* item = &g_Level.Items[itemNumber];

	// Store old position for later
	auto oldPos = item->Pose.Position;

	bool aboveWater = false;
	bool explode = false;

	// Update speed and check if above water
	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		auto bubblePos = item->Pose.Position;

		if (item->Animation.Velocity.z > 64.0f)
			item->Animation.Velocity.z -= item->Animation.Velocity.z / 16.0f;

		if (GlobalCounter & 1)
			CreateBubble(&bubblePos, item->RoomNumber, 4, 7, 0, 0, 0, 0);

		aboveWater = false;
	}
	else
		aboveWater = true;

	TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.z);

	auto probe = GetCollision(item);

	// Check if bolt has hit a solid wall
	if (probe.Position.Floor < item->Pose.Position.y ||
		probe.Position.Ceiling > item->Pose.Position.y)
	{
		// I have hit a solid wall, this is the end for the bolt
		item->Pose.Position = oldPos;

		// If ammos are normal, then just shatter the bolt and quit
		if (item->ItemFlags[0] != (int)CrossbowBoltType::Explosive)
		{
			ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
			KillItem(itemNumber);
			return;
		}
		// Otherwise, bolt must explode
		else
			explode = true;
	}

	// Has bolt changed room?
	if (item->RoomNumber != probe.RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	// If now in water and before in land, add a ripple
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) && aboveWater)
		SetupRipple(item->Pose.Position.x, g_Level.Rooms[item->RoomNumber].minfloor, item->Pose.Position.z, (GetRandomControl() & 7) + 8, 0);

	int radius = explode ? CROSSBOW_EXPLODE_RADIUS : CROSSBOW_HIT_RADIUS;
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
		if (item->ItemFlags[0] == (int)CrossbowBoltType::Explosive)
			explode = true;

		if (CollidedItems[0])
		{
			auto* currentItem = CollidedItems[0];
			
			int k = 0;
			do
			{
				auto* currentObj = &Objects[currentItem->ObjectNumber];

				if ((currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE && !currentObj->undead) ||
					(currentItem->ObjectNumber == ID_LARA && explode) ||
					(currentItem->Flags & 0x40 &&
					(Objects[currentItem->ObjectNumber].explodableMeshbits || currentItem == LaraItem)))
				{
					if (explode)
					{
						// All active intelligent creatures explode, if their HP is <= 0
						// Explosion is handled by CreatureDie()
						// Also Lara can be damaged
						// HitTarget() is called inside this
						DoExplosiveDamageOnBaddy(LaraItem, currentItem, item, LaraWeaponType::Crossbow);
					}
					else if (currentItem->ObjectNumber != ID_LARA)
					{
						// Normal hit
						HitTarget(LaraItem, currentItem, (GameVector*)& item->Pose, Weapons[(int)LaraWeaponType::Crossbow].Damage << item->ItemFlags[0], 0);

						// Poisoned ammos
						if (item->ItemFlags[0] == (int)CrossbowBoltType::Poison)
						{
							if (currentItem->Data.is<CreatureInfo>())
							{
								auto* creature = GetCreatureInfo(currentItem);
								creature->Poisoned = true;
							}
						}
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
			auto* currentMesh = CollidedMeshes[0];
			int k = 0;

			do
			{
				auto* s = &StaticObjects[currentMesh->staticNumber];
				if (s->shatterType != SHT_NONE)
				{
					currentMesh->HitPoints -= Weapons[(int)LaraWeaponType::Crossbow].Damage;
					if (currentMesh->HitPoints <= 0)
					{
						ShatterObject(nullptr, currentMesh, -128, item->RoomNumber, 0);
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
			ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
			KillItem(itemNumber);
		}
	}
	else
	{
		// Explode
		if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			TriggerShockwave(&item->Pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);
			item->Pose.Position.y += 128;
			TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->RoomNumber);

			for (int j = 0; j < 2; j++)
				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->RoomNumber);
		}

		AlertNearbyGuards(item);

		SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 0.7f, 0.5f);
		SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose);

		ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
		KillItem(itemNumber);
	}
}

void FireHK(ItemInfo* laraItem, int mode)
{
	auto* lara = GetLaraInfo(laraItem);

	auto angles = Vector3Shrt(
		lara->LeftArm.Orientation.x,
		lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
		0
	);

	if (!lara->LeftArm.Locked)
	{
		angles = Vector3Shrt(
			lara->ExtraTorsoRot.x + lara->LeftArm.Orientation.x,
			lara->ExtraTorsoRot.y + lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
			0
		);
	}

	if (mode)
	{
		Weapons[(int)LaraWeaponType::HK].ShotAccuracy = 2184;
		Weapons[(int)LaraWeaponType::HK].Damage = 1;
	}
	else
	{
		Weapons[(int)LaraWeaponType::HK].ShotAccuracy = 728;
		Weapons[(int)LaraWeaponType::HK].Damage = 3;
	}

	if (FireWeapon(LaraWeaponType::HK, lara->TargetEntity, laraItem, angles) != FireWeaponType::NoAmmo)
	{
		lara->LeftArm.GunSmoke = 12;

		TriggerGunShell(1, ID_GUNSHELL, LaraWeaponType::HK);
		lara->RightArm.GunFlash = Weapons[(int)LaraWeaponType::HK].FlashTime;

		Rumble(0.2f, 0.1f);
	}
}

void RifleHandler(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	if (BinocularRange)
		return; // Never handle weapons when in binocular mode!

	auto* weapon = &Weapons[(int)weaponType];
	LaraGetNewTarget(laraItem, weapon);

	if (TrInput & IN_ACTION)
		LaraTargetInfo(laraItem, weapon);

	AimWeapon(laraItem, weapon, &lara->LeftArm);

	if (lara->LeftArm.Locked)
	{
		lara->ExtraTorsoRot.x = lara->LeftArm.Orientation.x;
		lara->ExtraTorsoRot.y = lara->LeftArm.Orientation.y;

		if (Camera.oldType != CameraType::Look && !BinocularRange)
			lara->ExtraHeadRot = { 0, 0, 0 };
	}

	if (weaponType == LaraWeaponType::Revolver)
		AnimatePistols(laraItem, LaraWeaponType::Revolver);
	else
		AnimateShotgun(laraItem, weaponType);

	if (lara->RightArm.GunFlash)
	{
		if (weaponType == LaraWeaponType::Shotgun || weaponType == LaraWeaponType::HK)
		{
			auto pos = Vector3i();
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
		else if (weaponType == LaraWeaponType::Revolver)
		{
			auto pos = Vector3i();
			pos.y = -32;
			GetLaraJointPosition(&pos, LM_RHAND);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, (GetRandomControl() & 0x3F));
		}
	}
}

void DoExplosiveDamageOnBaddy(ItemInfo* laraItem, ItemInfo* dest, ItemInfo* src, LaraWeaponType weaponType)
{
	if (dest->Flags & IFLAG_KILLED)
		return;

	if (dest->HitPoints == NOT_TARGETABLE)
		return;

	if (dest != laraItem || laraItem->HitPoints <= 0)
	{
		if (src->ItemFlags[2])
			return;

		dest->HitStatus = true;

		HitTarget(laraItem, dest, 0, Weapons[(int)weaponType].ExplosiveDamage, 1);
					
		if (dest != laraItem)
		{
			Statistics.Game.AmmoHits++;
			if (dest->HitPoints <= 0)
			{
				Statistics.Level.Kills++;
				CreatureDie((dest - g_Level.Items.data()), true);
			}
		}
	}
	else
	{
		DoDamage(laraItem, Weapons[(int)weaponType].Damage * 5);
		if (!TestEnvironment(ENV_FLAG_WATER, dest->RoomNumber) && laraItem->HitPoints <= Weapons[(int)weaponType].Damage)
			LaraBurn(laraItem);
	}
}

void HitSpecial(ItemInfo* projectile, ItemInfo* target, int flags)
{

}
