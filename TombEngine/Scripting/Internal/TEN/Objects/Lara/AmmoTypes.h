#pragma once
#include <string>
#include <unordered_map>

#include "Game/Lara/lara_struct.h"

/***
Constants for Lara's weapons types IDs.
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
	BOLT_NORMAL
	BOLT_POISON
	BOLT_EXPLOSIVE
	GRENADE_NORMAL
	GRENADE_FRAG
	GRENADE_FLASH
	HARPOON
	ROCKET

@section Objects.AmmoTypeID
*/

/*** Table of LaraAmmoType flag ID constants.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, LaraAmmoType> AMMO_TYPE_IDS
{
	{ "NONE", LaraAmmoType::None },
	{ "PISTOLS", LaraAmmoType::Pistol },
	{ "REVOLVER", LaraAmmoType::Revolver },
	{ "UZI", LaraAmmoType::Uzi },
	{ "SHOTGUN_NORMAL", LaraAmmoType::Shotgun_Normal },
	{ "SHOTGUN_WIDE", LaraAmmoType::Shotgun_Wide },
	{ "HK", LaraAmmoType::HK },
	{ "BOLT_NORMAL", LaraAmmoType::Bolt_Normal },
	{ "BOLT_POISON", LaraAmmoType::Bolt_Poison },
	{ "BOLT_EXPLOSIVE", LaraAmmoType::Bolt_Explosive },
	{ "GRENADE_NORMAL", LaraAmmoType::Grenade_Normal },
	{ "GRENADE_FRAG", LaraAmmoType::Grenade_Frag },
	{ "GRENADE_FLASH", LaraAmmoType::Grenade_Flash },
	{ "HARPOON", LaraAmmoType::Harpoon },
	{ "ROCKET", LaraAmmoType::Rocket }
};
