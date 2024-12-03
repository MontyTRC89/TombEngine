#pragma once
#include "framework.h"
#include "Game/control/control.h"
#include "Game/Lara/lara_struct.h"

/***
Constants for weapon types.
@enum Flow.WeaponType
@pragma nostrip
*/

/*** Flow.WeaponType constants.

The following constants are inside Flow.WeaponType.

	WeaponType.PISTOL
	WeaponType.UZI
	WeaponType.REVOLVER
	WeaponType.SHOTGUN
	WeaponType.CROSSBOW
	WeaponType.GRENADE_LAUNCHER
	WeaponType.ROCKET_LAUNCHER
	WeaponType.HARPOON_GUN
	WeaponType.HK

@section Flow.WeaponType
*/

/*** Table of weapon types.
@table WeaponType
*/

static const std::unordered_map<std::string, LaraWeaponType> WEAPON_TYPES
{
	{ "NONE", LaraWeaponType::None },
	{ "PISTOL", LaraWeaponType::Pistol },
	{ "REVOLVER", LaraWeaponType::Revolver },
	{ "UZI", LaraWeaponType::Uzi },
	{ "SHOTGUN", LaraWeaponType::Shotgun },
	{ "HK", LaraWeaponType::HK },
	{ "CROSSBOW", LaraWeaponType::Crossbow },
	{ "FLARE", LaraWeaponType::Flare },
	{ "TORCH", LaraWeaponType::Torch },
	{ "GRENADE_LAUNCHER", LaraWeaponType::GrenadeLauncher },
	{ "HARPOON_GUN", LaraWeaponType::HarpoonGun },
	{ "ROCKET_LAUNCHER", LaraWeaponType::RocketLauncher }
};

