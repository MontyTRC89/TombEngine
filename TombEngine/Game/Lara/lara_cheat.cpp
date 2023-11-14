#include "framework.h"
#include "Game/Lara/lara_cheat.h"

#include <OISKeyboard.h>

#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/GuiObjects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Gui;
using namespace TEN::Input;

void lara_as_swimcheat(ItemInfo* item, CollisionInfo* coll)
{
	if (IsHeld(In::Forward))
	{
		item->Pose.Orientation.x -= ANGLE(3.0f);
	}
	else if (IsHeld(In::Back))
	{
		item->Pose.Orientation.x += ANGLE(3.0f);
	}

	if (IsHeld(In::Left))
	{
		ModulateLaraTurnRateY(item, ANGLE(3.4f), 0, ANGLE(6.0f));
	}
	else if (IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, ANGLE(3.4f), 0, ANGLE(6.0f));
	}

	if (IsHeld(In::Action))
		TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 31, 150, 150, 150);

	if (IsHeld(In::Jump))
	{
		float velCoeff = IsHeld(In::Sprint) ? 2.5f : 1.0f;

		item->Animation.Velocity.y += (LARA_SWIM_VELOCITY_ACCEL * 4) * velCoeff;
		if (item->Animation.Velocity.y > (LARA_SWIM_VELOCITY_MAX * 2) * velCoeff)
			item->Animation.Velocity.y = (LARA_SWIM_VELOCITY_MAX * 2) * velCoeff;
	}
	else
	{
		if (item->Animation.Velocity.y >= LARA_SWIM_VELOCITY_ACCEL)
		{
			item->Animation.Velocity.y -= item->Animation.Velocity.y / 8;
		}
		else
		{
			item->Animation.Velocity.y = 0.0f;
		}
	}
}

static void GivePlayerItemsCheat(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	for (int i = 0; i < 8; ++i)
	{
		if (Objects[ID_PUZZLE_ITEM1 + i].loaded)
			player.Inventory.Puzzles[i] = true;

		player.Inventory.PuzzlesCombo[2 * i] = false;
		player.Inventory.PuzzlesCombo[(92 * i) + 1] = false;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (Objects[ID_KEY_ITEM1 + i].loaded)
			player.Inventory.Keys[i] = true;

		player.Inventory.KeysCombo[2 * i] = false;
		player.Inventory.KeysCombo[(2 * i) + 1] = false;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (Objects[ID_PICKUP_ITEM1 + i].loaded)
			player.Inventory.Pickups[i] = true;

		player.Inventory.PickupsCombo[2 * i] = false;
		player.Inventory.PickupsCombo[(2 * i) + 1] = false;
	}
}

static void GivePlayerWeaponsCheat(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	player.Inventory.TotalFlares = -1;
	player.Inventory.TotalSmallMedipacks = -1;
	player.Inventory.TotalLargeMedipacks = -1;

	if (Objects[ID_CROWBAR_ITEM].loaded)
		player.Inventory.HasCrowbar = true;

	if (Objects[ID_LASERSIGHT_ITEM].loaded)
		player.Inventory.HasLasersight = true;

	if (Objects[ID_CLOCKWORK_BEETLE].loaded)
		player.Inventory.BeetleComponents |= BEETLECOMP_FLAG_BEETLE;

	if (Objects[ID_WATERSKIN1_EMPTY].loaded)
		player.Inventory.SmallWaterskin = 1;

	if (Objects[ID_WATERSKIN2_EMPTY].loaded)
		player.Inventory.BigWaterskin = 1;

	if (Objects[ID_PISTOLS_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::Pistol];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	if (Objects[ID_REVOLVER_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::Revolver];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	if (Objects[ID_UZI_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::Uzi];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	if (Objects[ID_SHOTGUN_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::Shotgun];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo2].SetInfinite(true);
	}

	if (Objects[ID_HARPOON_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::HarpoonGun];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	if (Objects[ID_GRENADE_GUN_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::GrenadeLauncher];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo2].SetInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo3].SetInfinite(true);
	}

	if (Objects[ID_ROCKET_LAUNCHER_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::RocketLauncher];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	if (Objects[ID_HK_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::HK];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.WeaponMode = LaraWeaponTypeCarried::WTYPE_AMMO_1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
	}

	if (Objects[ID_CROSSBOW_ITEM].loaded)
	{
		auto& weapon = player.Weapons[(int)LaraWeaponType::Crossbow];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].SetInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo2].SetInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo3].SetInfinite(true);
	}
}

void HandlePlayerFlyCheat(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	if (!g_GameFlow->IsFlyCheatEnabled())
		return;

	static bool dbFlyCheat = true;
	if (KeyMap[OIS::KeyCode::KC_O] && dbFlyCheat)
	{
		if (player.Context.Vehicle == NO_ITEM)
		{
			GivePlayerItemsCheat(item);
			GivePlayerWeaponsCheat(item);

			if (player.Control.WaterStatus != WaterStatus::FlyCheat)
			{
				SetAnimation(item, LA_DOZY);
				item.Animation.IsAirborne = false;
				item.HitPoints = LARA_HEALTH_MAX;

				ResetPlayerFlex(&item);
				player.Control.WaterStatus = WaterStatus::FlyCheat;
				player.Control.Count.Death = 0;
				player.Status.Air = LARA_AIR_MAX;
				player.Status.Poison = 0;
				player.Status.Stamina = LARA_STAMINA_MAX;
			}
		}
		else
		{
			SayNo();
		}
	}
	dbFlyCheat = !KeyMap[OIS::KeyCode::KC_O];
}
