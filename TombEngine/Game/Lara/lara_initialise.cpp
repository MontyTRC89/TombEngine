#include "framework.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/Hud/Hud.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/Lara/PlayerStateMachine.h"
#include "Game/Setup.h"
#include "Specific/level.h"

using namespace TEN::Entities::Player;
using namespace TEN::Hud;

LaraInfo lBackup = {};
ItemInfo lItemBackup = {};
GAME_OBJECT_ID lVehicleID = GAME_OBJECT_ID::ID_NO_OBJECT;

void BackupLara()
{
	if (LaraItem == nullptr || LaraItem->Index == NO_ITEM)
		return;

	memcpy(&lBackup, &Lara, sizeof(LaraInfo));
	memcpy(&lItemBackup, LaraItem, sizeof(ItemInfo));

	if (Lara.Context.Vehicle != NO_ITEM)
		lVehicleID = g_Level.Items[Lara.Context.Vehicle].ObjectNumber;
	else
		lVehicleID = GAME_OBJECT_ID::ID_NO_OBJECT;
}

void InitializeLara(bool restore)
{
	if (LaraItem == nullptr || LaraItem->Index == NO_ITEM)
		return;

	ZeroMemory(&Lara, sizeof(LaraInfo));

	LaraItem->Data = &Lara;
	Lara.Context = PlayerContext(*LaraItem, LaraCollision);

	LaraItem->Collidable = false;

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

	InitializePlayerStateMachine();
	InitializeLaraMeshes(LaraItem);
	InitializeLaraAnims(LaraItem);
	InitializeLaraStartPosition(*LaraItem);

	if (restore)
	{
		InitializeLaraLevelJump(LaraItem, &lBackup);
		LaraItem->HitPoints = lItemBackup.HitPoints;

		if (lVehicleID != GAME_OBJECT_ID::ID_NO_OBJECT)
		{
			auto* vehicle = FindItem(lVehicleID);
			if (vehicle != nullptr)
			{ 
				vehicle->Pose = LaraItem->Pose;
				ItemNewRoom(vehicle->Index, LaraItem->RoomNumber);
				SetLaraVehicle(LaraItem, vehicle);
				LaraItem->Animation = lItemBackup.Animation;
			}
		}
	}
	else
	{
		InitializeLaraDefaultInventory(LaraItem);
		LaraItem->HitPoints = LARA_HEALTH_MAX;
	}

	g_Hud.StatusBars.Initialize(*LaraItem);
}

void InitializeLaraMeshes(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	// Override base mesh and mesh indices to Lara skin, if it exists.
	item->Model.BaseMesh = Objects[(Objects[ID_LARA_SKIN].loaded ? ID_LARA_SKIN : ID_LARA)].meshIndex;

	for (int i = 0; i < NUM_LARA_MESHES; i++)
		item->Model.MeshIndex[i] = item->Model.BaseMesh + i;
}

void InitializeLaraAnims(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.HandStatus = HandStatus::Free;
	lara->TargetEntity = nullptr;
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;

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

void InitializeLaraStartPosition(ItemInfo& playerItem)
{
	for (const auto& item : g_Level.Items)
	{
		if (item.ObjectNumber != GAME_OBJECT_ID::ID_LARA_START_POS)
			continue;

		if (!item.TriggerFlags || item.TriggerFlags != RequiredStartPos)
			continue;

		playerItem.Pose = item.Pose;
		ItemNewRoom(playerItem.Index, item.RoomNumber);

		TENLog("Player start position has been set according to start position object with index " + std::to_string(item.TriggerFlags), LogLevel::Info);
		break;
	}

	playerItem.Location.RoomNumber = playerItem.RoomNumber;
	playerItem.Location.Height = playerItem.Pose.Position.y;
}

void InitializeLaraLevelJump(ItemInfo* item, LaraInfo* lBackup)
{
	auto* lara = GetLaraInfo(item);

	// Restore inventory.
	// It restores even puzzle/key items, to reset them, a ResetHub analog must be made.
	lara->Inventory = lBackup->Inventory;
	lara->Status = lBackup->Status;
	lara->Control.Weapon.LastGunType = lBackup->Control.Weapon.LastGunType;
	memcpy(&lara->Weapons, &lBackup->Weapons, sizeof(CarriedWeaponInfo) * int(LaraWeaponType::NumWeapons));

	// Restore holsters.
	// At first, attempt to restore original holster data, then refer to selected weapons.
	lara->Control.Weapon.HolsterInfo = lBackup->Control.Weapon.HolsterInfo;
	
	if (lara->Control.Weapon.HolsterInfo.RightHolster == HolsterSlot::Empty ||
		lara->Control.Weapon.HolsterInfo.LeftHolster  == HolsterSlot::Empty)
	{
		UndrawPistolMesh(*item, lara->Control.Weapon.LastGunType, true);
		UndrawPistolMesh(*item, lara->Control.Weapon.LastGunType, false);
	}

	if (lara->Control.Weapon.HolsterInfo.BackHolster == HolsterSlot::Empty)
	{
		UndrawShotgunMeshes(*item, lara->Control.Weapon.LastGunType);
	}

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

void InitializeLaraDefaultInventory(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (Objects[ID_FLARE_INV_ITEM].loaded)
		lara->Inventory.TotalFlares = 3;

	if (Objects[ID_SMALLMEDI_ITEM].loaded)
		lara->Inventory.TotalSmallMedipacks = 3;

	if (Objects[ID_BIGMEDI_ITEM].loaded)
		lara->Inventory.TotalLargeMedipacks = 1;

	if (Objects[ID_BINOCULARS_ITEM].loaded)
		lara->Inventory.HasBinoculars = true;

	lara->Inventory.BeetleLife = 3;

	auto weapon = LaraWeaponType::None;

	if (Objects[ID_PISTOLS_ITEM].loaded)
	{
		weapon = LaraWeaponType::Pistol;
		lara->Weapons[(int)LaraWeaponType::Pistol].Present = true;
		lara->Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	lara->Control.Weapon.LastGunType =
	lara->Control.Weapon.GunType =
	lara->Control.Weapon.RequestGunType = weapon;

	if (lara->Weapons[(int)LaraWeaponType::Pistol].Present)
	{
		lara->Control.Weapon.HolsterInfo.LeftHolster =
		lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Pistols;
	}
	else
	{
		lara->Control.Weapon.HolsterInfo.LeftHolster =
		lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
	}
}