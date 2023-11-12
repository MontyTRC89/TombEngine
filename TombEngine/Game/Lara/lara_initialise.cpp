#include "framework.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/Hud/Hud.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/PlayerStateMachine.h"
#include "Game/Setup.h"
#include "Specific/level.h"

using namespace TEN::Entities::Player;
using namespace TEN::Hud;

LaraInfo lBackup = {};
int lHitPoints   = 0;

void BackupLara()
{
	if (LaraItem == nullptr || LaraItem->Index == NO_ITEM)
		return;

	memcpy(&lBackup, &Lara, sizeof(LaraInfo));
	lHitPoints = LaraItem->HitPoints;
}

void InitializeLara(bool restore)
{
	if (LaraItem == nullptr || LaraItem->Index == NO_ITEM)
		return;

	ZeroMemory(&Lara, sizeof(LaraInfo));

	LaraItem->Data = &Lara;
	LaraItem->Collidable = false;
	LaraItem->Location.roomNumber = LaraItem->RoomNumber;
	LaraItem->Location.yNumber = LaraItem->Pose.Position.y;

	Lara.Status.Air = LARA_AIR_MAX;
	Lara.Status.Exposure = LARA_EXPOSURE_MAX;
	Lara.Status.Poison = 0;
	Lara.Status.Stamina = LARA_STAMINA_MAX;

	Lara.Control.Look.Mode = LookMode::None;
	Lara.HitDirection = -1;
	Lara.Control.Weapon.WeaponItem = NO_ITEM;
	Lara.Context.WaterSurfaceDist = 100;

	Lara.ExtraAnim = NO_ITEM;
	Lara.Context.Vehicle = NO_ITEM;
	Lara.Location = -1;
	Lara.HighestLocation = -1;
	Lara.Control.Rope.Ptr = -1;
	Lara.Control.HandStatus = HandStatus::Free;

	if (restore)
	{
		InitializeLaraLevelJump(LaraItem->Index, &lBackup);
		LaraItem->HitPoints = lHitPoints;
	}
	else
	{
		InitializeLaraDefaultInventory();
		LaraItem->HitPoints = LARA_HEALTH_MAX;
	}

	InitializePlayerStateMachine();
	InitializeLaraMeshes(LaraItem);
	InitializeLaraAnims(LaraItem);

	g_Hud.StatusBars.Initialize(*LaraItem);
}

void InitializeLaraMeshes(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	// Override base mesh and mesh indices to Lara skin, if it exists.
	item->Model.BaseMesh = Objects[(Objects[ID_LARA_SKIN].loaded ? ID_LARA_SKIN : ID_LARA)].meshIndex;

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

	if (lara->Weapons[(int)LaraWeaponType::Pistol].Present)
	{
		Lara.Control.Weapon.HolsterInfo.LeftHolster =
		Lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Pistols;
	}
	else
	{
		Lara.Control.Weapon.HolsterInfo.LeftHolster =
		Lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
	}

	lara->Control.HandStatus = HandStatus::Free;
	lara->TargetEntity = nullptr;
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
}

void InitializeLaraAnims(ItemInfo* item)
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

void InitializeLaraLoad(short itemNumber)
{
	LaraItem = &g_Level.Items[itemNumber];
}

void InitializeLaraLevelJump(short itemNum, LaraInfo* lBackup)
{
	auto* item = &g_Level.Items[itemNum];
	auto* lara = GetLaraInfo(item);

	// Restore inventory.
	// It restores even puzzle/key items, to reset them, a ResetHub analog must be made.
	lara->Inventory = lBackup->Inventory;
	lara->Control.Weapon.LastGunType = lBackup->Control.Weapon.LastGunType;
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
	DrawFlareMeshes(*item);
}

void InitializeLaraDefaultInventory()
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
	}

	Lara.Control.Weapon.LastGunType =
	Lara.Control.Weapon.GunType =
	Lara.Control.Weapon.RequestGunType = weapon;
}