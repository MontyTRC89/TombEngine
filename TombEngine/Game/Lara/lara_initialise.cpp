#include "framework.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/health.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void InitialiseLara(int restore)
{
	if (Lara.ItemNumber == NO_ITEM)
		return;

	short itemNumber = Lara.ItemNumber;

	LaraItem->Data = &Lara;
	LaraItem->Collidable = false;
	LaraItem->Location.roomNumber = LaraItem->RoomNumber;
	LaraItem->Location.yNumber = LaraItem->Pose.Position.y;

	LaraInfo backup = {};

	if (restore)
		memcpy(&backup, &Lara, sizeof(LaraInfo));

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

	LaraWeaponType weapon = LaraWeaponType::None;

	if (Objects[ID_HK_ITEM].loaded)
		weapon = LaraWeaponType::HK;

	if (Objects[ID_PISTOLS_ITEM].loaded)
		weapon = LaraWeaponType::Pistol;

	Lara.Control.Weapon.LastGunType = Lara.Control.Weapon.GunType = Lara.Control.Weapon.RequestGunType = weapon;

	LaraInitialiseMeshes(LaraItem);

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

	if (!restore)
	{
		if (Objects[ID_FLARE_INV_ITEM].loaded)
			Lara.Inventory.TotalFlares = 3;

		Lara.Inventory.TotalSmallMedipacks = 3;
		Lara.Inventory.TotalLargeMedipacks = 1;
	}

	InitialiseLaraAnims(LaraItem);
	Lara.Inventory.BeetleLife = 3;
}

void LaraInitialiseMeshes(ItemInfo* item)
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
		item->Animation.VerticalVelocity = 0;
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
