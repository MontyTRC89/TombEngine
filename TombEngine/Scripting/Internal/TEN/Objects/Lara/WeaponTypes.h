#pragma once
#include "framework.h"

#include "Game/control/control.h"
#include "Game/Lara/lara_struct.h"

/// Constants for weapon types.
// @enum Objects.WeaponType
// @pragma nostrip

/// Objects.WeaponType constants. To be used with @{Objects.LaraObject.GetWeaponType} and @{Objects.LaraObject.SetWeaponType} functions.
// 
// Note that this enumeration also contains flare and torch - these are counted as "weapon" internally by the engine, and indicate
// an object that is currently in Lara's hands.
// 
// - `NONE`
// - `PISTOLS`
// - `UZI`
// - `REVOLVER`
// - `SHOTGUN`
// - `HK`
// - `CROSSBOW`
// - `FLARE`
// - `TORCH`
// - `GRENADE_LAUNCHER`
// - `HARPOON_GUN`
// - `ROCKET_LAUNCHER`
// 
// @table Objects.WeaponType

namespace TEN::Scripting
{
	static const auto WEAPON_TYPES = std::unordered_map<std::string, LaraWeaponType>
	{
		{ "NONE", LaraWeaponType::None },
		{ "PISTOL", LaraWeaponType::Pistol },
		{ "PISTOLS", LaraWeaponType::Pistol },
		{ "REVOLVER", LaraWeaponType::Revolver },
		{ "UZI", LaraWeaponType::Uzi },
		{ "UZIS", LaraWeaponType::Uzi },
		{ "SHOTGUN", LaraWeaponType::Shotgun },
		{ "HK", LaraWeaponType::HK },
		{ "CROSSBOW", LaraWeaponType::Crossbow },
		{ "FLARE", LaraWeaponType::Flare },
		{ "TORCH", LaraWeaponType::Torch },
		{ "GRENADE_LAUNCHER", LaraWeaponType::GrenadeLauncher },
		{ "HARPOON_GUN", LaraWeaponType::HarpoonGun },
		{ "ROCKET_LAUNCHER", LaraWeaponType::RocketLauncher }
	};
}
