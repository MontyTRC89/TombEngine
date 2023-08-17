#pragma once
#include "Game/Lara/lara_struct.h"

/***
Constants for player weapon ammo types.
@enum Objects.AmmoTypeID
@pragma nostrip
*/

/*** Objects.AmmoTypeID constants.

The following constants are inside AmmoTypeID.

	NONE
	PISTOLS
	REVOLVER
	UZI
	SHOTGUN_NORMAL
	SHOTGUN_WIDE
	HK
	CROSSBOW_BOLT_NORMAL
	CROSSBOW_BOLT_POISON
	CROSSBOW_BOLT_EXPLOSIVE
	GRENADE_NORMAL
	GRENADE_FRAG
	GRENADE_FLASH
	HARPOON
	ROCKET

@section Objects.AmmoTypeID
*/

/*** PlayerAmmoType constants table.
@table CONSTANT_STRING_HERE
*/
static const std::unordered_map<std::string, PlayerWeaponAmmoType> WEAPON_AMMO_TYPES
{
	{ "PISTOLS", PlayerWeaponAmmoType::Pistol },
	{ "REVOLVER", PlayerWeaponAmmoType::Revolver },
	{ "UZI", PlayerWeaponAmmoType::Uzi },
	{ "SHOTGUN_NORMAL", PlayerWeaponAmmoType::ShotgunNormal },
	{ "SHOTGUN_WIDE", PlayerWeaponAmmoType::ShotgunWide },
	{ "HK", PlayerWeaponAmmoType::HK },
	{ "BOLT_NORMAL", PlayerWeaponAmmoType::CrossbowBoltNormal },
	{ "BOLT_POISON", PlayerWeaponAmmoType::CrossbowBoltPoison },
	{ "BOLT_EXPLOSIVE", PlayerWeaponAmmoType::CrossbowBoltExplosive },
	{ "GRENADE_NORMAL", PlayerWeaponAmmoType::GrenadeNormal },
	{ "GRENADE_FRAG", PlayerWeaponAmmoType::GrenadeFrag },
	{ "GRENADE_FLASH", PlayerWeaponAmmoType::GrenadeFlash },
	{ "HARPOON", PlayerWeaponAmmoType::Harpoon },
	{ "ROCKET", PlayerWeaponAmmoType::Rocket }
};
