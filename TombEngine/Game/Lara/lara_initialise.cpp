#include "framework.h"
#include "Game/Lara/lara_initialise.h"

#include "Game/Hud/Hud.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/Lara/PlayerStateMachine.h"
#include "Game/Setup.h"
#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"
#include "Specific/level.h"

using namespace TEN::Entities::Player;
using namespace TEN::Hud;

// Globals
int					PlayerHitPoints		  = 0;
LaraInfo			PlayerBackup		  = {};
EntityAnimationData PlayerAnim			  = {};
GAME_OBJECT_ID		PlayerVehicleObjectID = GAME_OBJECT_ID::ID_NO_OBJECT;

void BackupLara()
{
	if (LaraItem == nullptr || LaraItem->Index == NO_VALUE)
		return;

	PlayerHitPoints = LaraItem->HitPoints;
	memcpy(&PlayerBackup, &Lara, sizeof(LaraInfo));
	memcpy(&PlayerAnim, &LaraItem->Animation, sizeof(EntityAnimationData));

	if (Lara.Context.Vehicle != NO_VALUE)
	{
		PlayerVehicleObjectID = g_Level.Items[Lara.Context.Vehicle].ObjectNumber;
	}
	else
	{
		PlayerVehicleObjectID = GAME_OBJECT_ID::ID_NO_OBJECT;
	}
}

void InitializeLara(bool restore)
{
	if (LaraItem == nullptr || LaraItem->Index == NO_VALUE)
		return;

	ZeroMemory(&Lara, sizeof(LaraInfo));

	LaraItem->Data = &Lara;
	LaraItem->Collidable = false;
	
	Lara.Context = PlayerContext(*LaraItem, LaraCollision);

	Lara.Status.Air = LARA_AIR_MAX;
	Lara.Status.Exposure = LARA_EXPOSURE_MAX;
	Lara.Status.Poison = 0;
	Lara.Status.Stamina = LARA_STAMINA_MAX;

	Lara.Control.Look.Mode = LookMode::None;
	Lara.HitDirection = NO_VALUE;
	Lara.Control.Weapon.WeaponItem = NO_VALUE;
	Lara.Context.WaterSurfaceDist = 100;

	Lara.ExtraAnim = NO_VALUE;
	Lara.Context.Vehicle = NO_VALUE;
	Lara.Location = NO_VALUE;
	Lara.HighestLocation = NO_VALUE;
	Lara.Control.Rope.Ptr = NO_VALUE;
	Lara.Control.HandStatus = HandStatus::Free;

	InitializePlayerStateMachine();
	InitializeLaraMeshes(LaraItem);
	InitializeLaraStartPosition(*LaraItem);

	if (restore)
	{
		InitializeLaraLevelJump(LaraItem, &PlayerBackup);
	}
	else
	{
		InitializeLaraDefaultInventory(*LaraItem);
	}

	// Player anim init should happen after leveljump init.
	InitializeLaraAnims(LaraItem);

	g_Hud.StatusBars.Initialize(*LaraItem);
}

void InitializeLaraMeshes(ItemInfo* item)
{
	auto& player = GetLaraInfo(*item);

	// Override base mesh and mesh indices to player skin if it exists.
	item->Model.BaseMesh = Objects[(Objects[ID_LARA_SKIN].loaded ? ID_LARA_SKIN : ID_LARA)].meshIndex;

	for (int i = 0; i < NUM_LARA_MESHES; i++)
		item->Model.MeshIndex[i] = item->Model.BaseMesh + i;

	player.Control.Weapon.HolsterInfo.LeftHolster =
	player.Control.Weapon.HolsterInfo.RightHolster = 
	player.Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;
}

void InitializeLaraAnims(ItemInfo* item)
{
	auto& player = GetLaraInfo(*item);

	player.Control.HandStatus = HandStatus::Free;
	player.TargetEntity = nullptr;
	player.LeftArm.FrameNumber = 0;
	player.RightArm.FrameNumber = 0;
	player.LeftArm.Locked = false;
	player.RightArm.Locked = false;

	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		SetAnimation(item, LA_UNDERWATER_IDLE);
		item->Animation.Velocity.y = 0.0f;
		player.Control.WaterStatus = WaterStatus::Underwater;
	}
	else
	{
		player.Control.WaterStatus = WaterStatus::Dry;

		// Allow player to start in crawl idle anim if start position is too low.
		auto pointColl = GetCollision(item);
		if (abs(pointColl.Position.Ceiling - pointColl.Position.Floor) < LARA_HEIGHT)
		{
			SetAnimation(item, LA_CRAWL_IDLE);
			player.Control.IsLow =
			player.Control.KeepLow = true;
		}
		else
		{
			SetAnimation(item, LA_STAND_SOLID);
		}
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

		// HACK: For some reason, player can't be immediately updated and moved on loading.
		// Need to simulate "game loop" happening so that its position actually updates on next loop.
		// However, room number must be also be manually set in advance, so that startup anim detection
		// won't fail (otherwise player may start crouching because probe uses previous room number).

		InItemControlLoop = true;

		playerItem.Pose = item.Pose;
		playerItem.RoomNumber = item.RoomNumber;
		ItemNewRoom(playerItem.Index, item.RoomNumber);

		InItemControlLoop = false;

		TENLog("Player start position has been set according to start position of object with ID " + std::to_string(item.TriggerFlags) + ".", LogLevel::Info);
		break;
	}

	playerItem.Location.RoomNumber = playerItem.RoomNumber;
	playerItem.Location.Height = playerItem.Pose.Position.y;
}

static void InitializePlayerVehicle(ItemInfo& playerItem)
{
	if (PlayerVehicleObjectID == GAME_OBJECT_ID::ID_NO_OBJECT)
		return;

	auto* vehicle = FindItem(PlayerVehicleObjectID);
	if (vehicle == nullptr)
		return;

	// Restore vehicle.
	TENLog("Transferring vehicle " + GetObjectName(PlayerVehicleObjectID) + " from the previous level.");
	vehicle->Pose = playerItem.Pose;
	ItemNewRoom(vehicle->Index, playerItem.RoomNumber);
	SetLaraVehicle(&playerItem, vehicle);
	playerItem.Animation = PlayerAnim;

	// HACK: Reinitialize vehicles which require specific parameters to be reset according to Lara pose.

	switch (vehicle->ObjectNumber)
	{
	case GAME_OBJECT_ID::ID_KAYAK:
		InitializeKayak(vehicle->Index);
		break;

	case GAME_OBJECT_ID::ID_MOTORBIKE:
		InitializeMotorbike(vehicle->Index);
		break;

	case GAME_OBJECT_ID::ID_JEEP:
		InitializeJeep(vehicle->Index);
		break;

	case GAME_OBJECT_ID::ID_QUAD:
		InitializeQuadBike(vehicle->Index);
		break;

	case GAME_OBJECT_ID::ID_SNOWMOBILE:
		InitializeSkidoo(vehicle->Index);
		break;

	default:
		break;
	}
}

void InitializeLaraLevelJump(ItemInfo* item, LaraInfo* playerBackup)
{
	auto& player = GetLaraInfo(*item);

	// Restore inventory.
	player.Inventory = playerBackup->Inventory;
	player.Status = playerBackup->Status;
	player.Control.Weapon.LastGunType = playerBackup->Control.Weapon.LastGunType;
	memcpy(&player.Weapons, &playerBackup->Weapons, sizeof(CarriedWeaponInfo) * int(LaraWeaponType::NumWeapons));

	// Restore holsters. First attempt restoring original holster data, then refer to selected weapons.
	player.Control.Weapon.HolsterInfo = playerBackup->Control.Weapon.HolsterInfo;
	
	if ((player.Control.Weapon.HolsterInfo.RightHolster == HolsterSlot::Empty ||
		 player.Control.Weapon.HolsterInfo.LeftHolster  == HolsterSlot::Empty) &&
		 player.Control.Weapon.LastGunType <= LaraWeaponType::Uzi)
	{
		UndrawPistolMesh(*item, player.Control.Weapon.LastGunType, true);
		UndrawPistolMesh(*item, player.Control.Weapon.LastGunType, false);
	}

	if (player.Control.Weapon.HolsterInfo.BackHolster == HolsterSlot::Empty &&
		player.Control.Weapon.LastGunType > LaraWeaponType::Uzi)
	{
		UndrawShotgunMeshes(*item, player.Control.Weapon.LastGunType);
	}

	// Restore flare.
	if (playerBackup->Control.Weapon.GunType == LaraWeaponType::Flare)
	{
		player.LeftArm = playerBackup->LeftArm;
		player.RightArm = playerBackup->RightArm;
		player.Control.HandStatus = playerBackup->Control.HandStatus;
		player.Control.Weapon = playerBackup->Control.Weapon;
		player.Flare = playerBackup->Flare;
		DrawFlareMeshes(*item);
	}

	// Restore hit points.
	item->HitPoints = PlayerHitPoints;

	// Restore vehicle.
	InitializePlayerVehicle(*item);
}

void InitializeLaraDefaultInventory(ItemInfo& item)
{
	constexpr auto DEFAULT_FLARE_COUNT			= 3;
	constexpr auto DEFAULT_SMALL_MEDIPACK_COUNT = 3;
	constexpr auto DEFAULT_LARGE_MEDIPACK_COUNT = 1;
	constexpr auto DEFAULT_BEETLE_LIFE			= 3;

	auto& player = GetLaraInfo(item);

	item.HitPoints = LARA_HEALTH_MAX;

	if (Objects[ID_FLARE_INV_ITEM].loaded)
		player.Inventory.TotalFlares = DEFAULT_FLARE_COUNT;

	if (Objects[ID_SMALLMEDI_ITEM].loaded)
		player.Inventory.TotalSmallMedipacks = DEFAULT_SMALL_MEDIPACK_COUNT;

	if (Objects[ID_BIGMEDI_ITEM].loaded)
		player.Inventory.TotalLargeMedipacks = DEFAULT_LARGE_MEDIPACK_COUNT;

	if (Objects[ID_BINOCULARS_ITEM].loaded)
		player.Inventory.HasBinoculars = true;

	player.Inventory.BeetleLife = DEFAULT_BEETLE_LIFE;

	auto weaponType = LaraWeaponType::None;
	if (Objects[ID_PISTOLS_ITEM].loaded)
	{
		weaponType = LaraWeaponType::Pistol;
		player.Weapons[(int)LaraWeaponType::Pistol].Present = true;
		player.Weapons[(int)LaraWeaponType::Pistol].Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	player.Control.Weapon.LastGunType =
	player.Control.Weapon.GunType =
	player.Control.Weapon.RequestGunType = weaponType;

	if (player.Weapons[(int)LaraWeaponType::Pistol].Present)
	{
		player.Control.Weapon.HolsterInfo.LeftHolster =
		player.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Pistols;
	}
	else
	{
		player.Control.Weapon.HolsterInfo.LeftHolster =
		player.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
	}
}
