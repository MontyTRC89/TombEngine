#pragma once
#include "framework.h"

#include "Game/control/control.h"
#include "Game/Lara/lara_struct.h"

namespace TEN::Scripting
{
	/***
	Constants for weapon types.
	@enum Flow.WeaponType
	@pragma nostrip
	*/

	/*** Flow.WeaponType constants.

	The following constants are inside Flow.WeaponType. <br>
	Note that this enumeration also contains flare and torch - these are counted as "weapon" internally by the engine, and indicate
	an object that is currently in Lara's hands, as seen in @{Objects.LaraObject.GetWeaponType}.

		WeaponType.NONE
		WeaponType.PISTOLS
		WeaponType.UZI
		WeaponType.REVOLVER
		WeaponType.SHOTGUN
		WeaponType.HK
		WeaponType.CROSSBOW
		WeaponType.FLARE
		WeaponType.TORCH
		WeaponType.GRENADE_LAUNCHER
		WeaponType.HARPOON_GUN
		WeaponType.ROCKET_LAUNCHER

	@section Flow.WeaponType
	*/

	/*** Table of weapon types.
	@table WeaponType
	*/

	static const auto WEAPON_TYPES = std::unordered_map<std::string, LaraWeaponType>
	{
		{ "NONE", LaraWeaponType::None },
		{ "PISTOL", LaraWeaponType::Pistol }, // Deprecated
		{ "PISTOLS", LaraWeaponType::Pistol },
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
}
