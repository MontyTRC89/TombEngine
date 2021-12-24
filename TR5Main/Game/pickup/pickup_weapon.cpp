#include "framework.h"
#include "Game/pickup/pickup_weapon.h"

#include <array>
#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickuputil.h"
#include "Game/pickup/pickup_ammo.h"
#include "Objects/objectslist.h"

struct WeaponPickupInfo
{
	GAME_OBJECT_ID id;
	// when the player picks up a weapon they
	// get one clip's worth of the following ammo
	GAME_OBJECT_ID ammoID;
	LARA_WEAPON_TYPE laraWeaponType;
};

static constexpr std::array<WeaponPickupInfo, 9> kWeapons{ {
	{ID_PISTOLS_ITEM, ID_PISTOLS_AMMO_ITEM, WEAPON_PISTOLS},
	{ID_UZI_ITEM, ID_UZI_AMMO_ITEM, WEAPON_UZI},
	{ID_SHOTGUN_ITEM, ID_SHOTGUN_AMMO1_ITEM, WEAPON_SHOTGUN},
	{ID_CROSSBOW_ITEM, ID_CROSSBOW_AMMO1_ITEM, WEAPON_CROSSBOW},
	{ID_REVOLVER_ITEM, ID_REVOLVER_AMMO_ITEM, WEAPON_REVOLVER},
	{ID_HK_ITEM, ID_HK_AMMO_ITEM, WEAPON_HK},
	{ID_GRENADE_GUN_ITEM, ID_GRENADE_AMMO1_ITEM, WEAPON_GRENADE_LAUNCHER},
	{ID_ROCKET_LAUNCHER_ITEM, ID_ROCKET_LAUNCHER_AMMO_ITEM, WEAPON_ROCKET_LAUNCHER},
	{ID_HARPOON_ITEM, ID_HARPOON_AMMO_ITEM, WEAPON_HARPOON_GUN} }
};

static int GetWeapon(GAME_OBJECT_ID obj)
{
	for (int i = 0; i < kWeapons.size(); ++i)
	{
		if (obj == kWeapons[i].id)
		{
			return i;
		}
	}
	return -1;
}

static bool TryModifyWeapon(LaraInfo& lara, GAME_OBJECT_ID obj, int ammoAmt, bool add)
{
	int arrPos = GetArrSlot(kWeapons, obj);
	if (-1 == arrPos)
	{
		return false;
	}

	WeaponPickupInfo info = kWeapons[arrPos];
	// set the SelectedAmmo type to 0 if adding the weapon for the
	// first time. Note that this refers to the index of the weapon's
	// ammo array, and not the weapon's actual ammunition count
	if (!lara.Weapons[info.laraWeaponType].Present) {
		lara.Weapons[info.laraWeaponType].SelectedAmmo = 0;
	}
	lara.Weapons[info.laraWeaponType].Present = add;
	auto ammoID = info.ammoID;
	return add ? TryAddAmmo(lara, ammoID, ammoAmt) : TryRemoveAmmo(lara, ammoID, ammoAmt);
}


// Adding a weapon will either give the player the weapon + amt ammo
// or, if they already have the weapon, simply the ammo.
bool TryAddWeapon(LaraInfo& lara, GAME_OBJECT_ID obj, int amt)
{
	return TryModifyWeapon(lara, obj, amt, true);
}

// Removing a weapon is the reverse of the above; it will remove the weapon
// (if it's there) and amt ammo.
bool TryRemoveWeapon(LaraInfo& lara, GAME_OBJECT_ID obj, int amt)
{
	return TryModifyWeapon(lara, obj, amt, false);
}

std::optional<bool> HasWeapon(LaraInfo& lara, GAME_OBJECT_ID obj)
{
	int arrPos = GetArrSlot(kWeapons, obj);
	if (-1 == arrPos)
	{
		return std::nullopt;
	}

	WeaponPickupInfo info = kWeapons[arrPos];
	return lara.Weapons[info.laraWeaponType].Present;
}
