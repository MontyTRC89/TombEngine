#include "framework.h"
#include "Game/Lara/lara_cheat.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

void lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(3.0f);
	else if (TrInput & IN_BACK)
		item->Position.xRot += ANGLE(3.0f);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= ANGLE(3.4f);
		if (lara->Control.TurnRate < -ANGLE(6.0f))
			lara->Control.TurnRate = -ANGLE(6.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += ANGLE(3.4f);
		if (lara->Control.TurnRate > ANGLE(6.0f))
			lara->Control.TurnRate = ANGLE(6.0f);
	}

	if (TrInput & IN_ACTION)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos, 31, 255, 255, 255);

	if (TrInput & IN_OPTION)
		lara->Control.TurnRate = -ANGLE(12.0f);

	if (TrInput & IN_JUMP)
	{
		item->Animation.VerticalVelocity += 16;
		if (item->Animation.VerticalVelocity > 400)
			item->Animation.VerticalVelocity = 400;
	}
	else
	{
		if (item->Animation.VerticalVelocity >= 8)
			item->Animation.VerticalVelocity -= item->Animation.VerticalVelocity >> 3;
		else
			item->Animation.VerticalVelocity = 0;
	}
}

void LaraCheatyBits(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (g_GameFlow->FlyCheat)
	{
		if (KeyMap[DIK_O])
		{
			if (lara->Vehicle == NO_ITEM)
			{
				LaraCheatGetStuff(item);
				DelsGiveLaraItemsCheat(item);

				item->Position.yPos -= CLICK(0.5f);

				if (lara->Control.WaterStatus != WaterStatus::FlyCheat)
				{
					SetAnimation(item, LA_DOZY);
					item->Position.xRot = ANGLE(30.0f);
					item->Animation.VerticalVelocity = 30;
					item->Animation.Airborne = false;
					item->HitPoints = 1000;

					ResetLaraFlex(item);
					lara->Control.WaterStatus = WaterStatus::FlyCheat;
					lara->PoisonPotency = 0;
					lara->Air = 1800;
					lara->Control.Count.Death = 0;
				}
			}
			else if (!lara->Control.Count.NoCheat)
			{
				lara->Control.Count.NoCheat = 15;
				SayNo();
			}
		}
	}

	if (lara->Control.Count.NoCheat)
		lara->Control.Count.NoCheat--;
}

void LaraCheatGetStuff(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->Inventory.TotalFlares = -1;
	lara->Inventory.TotalSmallMedipacks = -1;
	lara->Inventory.TotalLargeMedipacks = -1;

	if (Objects[ID_CROWBAR_ITEM].loaded)
		lara->Inventory.HasCrowbar = true;

	if (Objects[ID_LASERSIGHT_ITEM].loaded)
		lara->Inventory.HasLasersight = true;

	if (Objects[ID_CLOCKWORK_BEETLE].loaded)
		lara->Inventory.BeetleComponents |= 1;

	if (Objects[ID_WATERSKIN1_EMPTY].loaded)
		lara->Inventory.SmallWaterskin = 1;

	if (Objects[ID_WATERSKIN2_EMPTY].loaded)
		lara->Inventory.BigWaterskin = 1;

	if (Objects[ID_REVOLVER_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::Revolver];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}

	if (Objects[ID_UZI_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::Uzi];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}

	if (Objects[ID_SHOTGUN_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::Shotgun];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}

	if (Objects[ID_HARPOON_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::HarpoonGun];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}

	if (Objects[ID_GRENADE_GUN_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::GrenadeLauncher];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo2].setInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo3].setInfinite(true);
	}

	if (Objects[ID_ROCKET_LAUNCHER_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::RocketLauncher];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}

	if (Objects[ID_HK_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::HK];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
	}

	if (Objects[ID_CROSSBOW_ITEM].loaded)
	{
		auto& weapon = lara->Weapons[(int)LaraWeaponType::Crossbow];

		weapon.Present = true;
		weapon.SelectedAmmo = WeaponAmmoType::Ammo1;
		weapon.HasLasersight = false;
		weapon.HasSilencer = false;
		weapon.Ammo[(int)WeaponAmmoType::Ammo1].setInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo2].setInfinite(true);
		weapon.Ammo[(int)WeaponAmmoType::Ammo3].setInfinite(true);
	}
}

void DelsGiveLaraItemsCheat(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	for (int i = 0; i < 8; ++i)
	{
		if (Objects[ID_PUZZLE_ITEM1 + i].loaded)
			lara->Inventory.Puzzles[i] = 1;

		lara->Inventory.PuzzlesCombo[2 * i] = false;
		lara->Inventory.PuzzlesCombo[2 * i + 1] = false;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (Objects[ID_KEY_ITEM1 + i].loaded)
			lara->Inventory.Keys[i] = 1;

		lara->Inventory.KeysCombo[2 * i] = false;
		lara->Inventory.KeysCombo[2 * i + 1] = false;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (Objects[ID_PICKUP_ITEM1 + i].loaded)
			lara->Inventory.Pickups[i] = 1;

		lara->Inventory.PickupsCombo[2 * i] = false;
		lara->Inventory.PickupsCombo[2 * i + 1] = false;
	}
	/* Hardcoded code */
}
