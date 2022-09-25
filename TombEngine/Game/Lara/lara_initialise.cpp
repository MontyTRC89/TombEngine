#include "framework.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/health.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseLara(bool restore)
{
	if (Lara.ItemNumber == NO_ITEM)
		return;

	LaraInfo lBackup = {};
	if (restore)
		memcpy(&lBackup, &Lara, sizeof(LaraInfo));

	short itemNumber = Lara.ItemNumber;

	LaraItem->Data = &Lara;
	LaraItem->Collidable = false;
	LaraItem->Location.roomNumber = LaraItem->RoomNumber;
	LaraItem->Location.yNumber = LaraItem->Pose.Position.y;

	ZeroMemory(&Lara, sizeof(LaraInfo));

	Lara.Control.Look.Mode = LookMode::None;
	Lara.ItemNumber = itemNumber;
	Lara.HitDirection = -1;
	Lara.SprintEnergy = LARA_SPRINT_ENERGY_MAX;
	Lara.Air = LARA_AIR_MAX;
	Lara.Control.Weapon.WeaponItem = NO_ITEM;
	Lara.PoisonPotency = 0;
	Lara.WaterSurfaceDist = 100;

	Lara.ExtraAnim = NO_ITEM;
	Lara.Vehicle = NO_ITEM;
	Lara.Location = -1;
	Lara.HighestLocation = -1;
	Lara.Control.Rope.Ptr = -1;
	LaraItem->HitPoints = LARA_HEALTH_MAX;
	Lara.Control.HandStatus = HandStatus::Free;

	LaraWeaponType weapon = LaraWeaponType::None;

	if (Objects[ID_HK_ITEM].loaded)
		weapon = LaraWeaponType::HK;

	if (Objects[ID_PISTOLS_ITEM].loaded)
		weapon = LaraWeaponType::Pistol;

	Lara.Control.Weapon.LastGunType = Lara.Control.Weapon.GunType = Lara.Control.Weapon.RequestGunType = weapon;

	InitialiseLaraMeshes(LaraItem);

	if (weapon == LaraWeaponType::Pistol)
	{
		Lara.Weapons[(int)LaraWeaponType::Pistol].Present = true;
		Lara.Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}
	else if (weapon == LaraWeaponType::HK)
	{
		Lara.Weapons[(int)LaraWeaponType::HK].Present = true;
		Lara.Weapons[(int)LaraWeaponType::HK].Ammo[(int)WeaponAmmoType::Ammo1] = 100;
	}

	if (Lara.Weapons[(int)LaraWeaponType::Pistol].Present)
	{
		Lara.Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Pistols;
		Lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Pistols;
	}
	else
	{
		Lara.Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;
		Lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
	}

	if (Lara.Weapons[(int)LaraWeaponType::Shotgun].Present)
		Lara.Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Shotgun;
	else
		Lara.Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;

	Lara.Inventory.HasBinoculars = true;

	if (restore)
		InitialiseLaraLevelJump(itemNumber, &lBackup);
	else
		InitialiseLaraDefaultInventory();

	InitialiseLaraAnims(LaraItem);
	Lara.Inventory.BeetleLife = 3;
}

void InitialiseLaraMeshes(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	for (int i = 0; i < NUM_LARA_MESHES; i++)
	{
		//Meshes[i] = Meshes[MESHES(ID_LARA_SKIN, i)];
		//LARA_MESHES(ID_LARA, MESHES(ID_LARA_SKIN, i));
		lara->MeshPtrs[i] = Objects[ID_LARA_SKIN].meshIndex + i;
	}

	/* Hardcoded code */

	if (lara->Control.Weapon.GunType == LaraWeaponType::HK)
		lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::HK;
	else if (!lara->Weapons[(int)LaraWeaponType::Shotgun].Present)
	{
		if (lara->Weapons[(int)LaraWeaponType::HK].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::HK;
	}
	else
		lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;

	lara->Control.HandStatus = HandStatus::Free;
	lara->TargetEntity = NULL;
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
}

void InitialiseLaraAnims(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		lara->Control.WaterStatus = WaterStatus::Underwater;
		item->Animation.Velocity.y = 0;
		SetAnimation(item, LA_UNDERWATER_IDLE);
	}
	else
	{
		lara->Control.WaterStatus = WaterStatus::Dry;
		SetAnimation(item, LA_STAND_SOLID);
	}
}

void InitialiseLaraLoad(short itemNum)
{
	Lara.ItemNumber = itemNum;
	LaraItem = &g_Level.Items[itemNum];
}

void InitialiseLaraLevelJump(short itemNum, LaraInfo* lBackup)
{
	auto* item = &g_Level.Items[itemNum];
	auto* lara = GetLaraInfo(item);

	// Restore inventory.
	// It restores even puzzle/key items, to reset them, a ResetHub analog must be made.
	lara->Inventory = lBackup->Inventory;
	memcpy(&lara->Weapons, &lBackup->Weapons, sizeof(CarriedWeaponInfo) * int(LaraWeaponType::NumWeapons));

	// If no flare present, quit
	if (lBackup->Control.Weapon.GunType != LaraWeaponType::Flare)
		return;

	// Restore flare
	lara->LeftArm = lBackup->LeftArm;
	lara->RightArm = lBackup->RightArm;
	lara->Control.HandStatus = lBackup->Control.HandStatus;
	lara->Control.Weapon = lBackup->Control.Weapon;
	lara->Flare = lBackup->Flare;
	DrawFlareMeshes(item);
}

void InitialiseLaraDefaultInventory()
{
	if (Objects[ID_FLARE_INV_ITEM].loaded)
		Lara.Inventory.TotalFlares = 3;

	if (Objects[ID_SMALLMEDI_ITEM].loaded)
		Lara.Inventory.TotalSmallMedipacks = 3;

	if (Objects[ID_BIGMEDI_ITEM].loaded)
		Lara.Inventory.TotalLargeMedipacks = 1;
}