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

	Lara.Control.CanLook = true;
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

	if (restore)
		InitialiseLaraLevelJump(itemNumber, &lBackup);
	else
		InitialiseLaraDefaultInventory();

	InitialiseLaraMeshes(LaraItem);
	InitialiseLaraAnims(LaraItem);
}

void InitialiseLaraMeshes(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	// Override base mesh and mesh indices to Lara skin.

	item->Model.BaseMesh = Objects[ID_LARA_SKIN].meshIndex;
	for (int i = 0; i < NUM_LARA_MESHES; i++)
		item->Model.MeshIndex[i] = item->Model.BaseMesh + i;

	switch (lara->Control.Weapon.GunType)
	{
	case LaraWeaponType::Shotgun:
		if (lara->Weapons[(int)LaraWeaponType::Shotgun].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Shotgun;
		break;

	case LaraWeaponType::Crossbow:
		if (lara->Weapons[(int)LaraWeaponType::Crossbow].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Crossbow;
		break;

	case LaraWeaponType::HarpoonGun:
		if (lara->Weapons[(int)LaraWeaponType::HarpoonGun].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Harpoon;
		break;

	case LaraWeaponType::GrenadeLauncher:
		if (lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::GrenadeLauncher;
		break;

	case LaraWeaponType::RocketLauncher:
		if (lara->Weapons[(int)LaraWeaponType::RocketLauncher].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::RocketLauncher;
		break;

	case LaraWeaponType::HK:
		if (lara->Weapons[(int)LaraWeaponType::HK].Present)
			lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::HK;
		break;

	default:
		lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;
		break;
	}

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

	if (Objects[ID_BINOCULARS_ITEM].loaded)
		Lara.Inventory.HasBinoculars = true;

	Lara.Inventory.BeetleLife = 3;

	LaraWeaponType weapon = LaraWeaponType::None;

	if (Objects[ID_PISTOLS_ITEM].loaded)
	{
		weapon = LaraWeaponType::Pistol;

		Lara.Weapons[(int)LaraWeaponType::Pistol].Present = true;
		Lara.Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);

		Lara.Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Pistols;
		Lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Pistols;
	}

	Lara.Control.Weapon.LastGunType =
	Lara.Control.Weapon.GunType =
	Lara.Control.Weapon.RequestGunType = weapon;
}