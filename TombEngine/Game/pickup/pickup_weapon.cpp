#include "framework.h"
#include "Game/pickup/pickup_weapon.h"

#include <array>

#include "lara_fire.h"
#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickuputil.h"
#include "Game/pickup/pickup_ammo.h"
#include "Objects/objectslist.h"

enum class HolsterType
{
	Hips,
	Back
};

struct WeaponPickupInfo
{
	GAME_OBJECT_ID ObjectID;
	GAME_OBJECT_ID AmmoID;	// When the player picks up a weapon, they get one clip's worth of this ammo.
	LaraWeaponType LaraWeaponType;
	HolsterType Holster;
};

static constexpr std::array<WeaponPickupInfo, 9> kWeapons
{
	{
		{ ID_PISTOLS_ITEM, ID_PISTOLS_AMMO_ITEM, LaraWeaponType::Pistol, HolsterType::Hips },
		{ ID_UZI_ITEM, ID_UZI_AMMO_ITEM, LaraWeaponType::Uzi, HolsterType::Hips},
		{ ID_SHOTGUN_ITEM, ID_SHOTGUN_AMMO1_ITEM, LaraWeaponType::Shotgun, HolsterType::Back },
		{ ID_CROSSBOW_ITEM, ID_CROSSBOW_AMMO1_ITEM, LaraWeaponType::Crossbow, HolsterType::Back},
		{ ID_REVOLVER_ITEM, ID_REVOLVER_AMMO_ITEM, LaraWeaponType::Revolver, HolsterType::Hips},
		{ ID_HK_ITEM, ID_HK_AMMO_ITEM, LaraWeaponType::HK, HolsterType::Back},
		{ ID_GRENADE_GUN_ITEM, ID_GRENADE_AMMO1_ITEM, LaraWeaponType::GrenadeLauncher, HolsterType::Back },
		{ ID_ROCKET_LAUNCHER_ITEM, ID_ROCKET_LAUNCHER_AMMO_ITEM, LaraWeaponType::RocketLauncher, HolsterType::Back },
		{ ID_HARPOON_ITEM, ID_HARPOON_AMMO_ITEM, LaraWeaponType::HarpoonGun, HolsterType::Back }
	}
};

static int GetWeapon(GAME_OBJECT_ID objectID)
{
	for (int i = 0; i < kWeapons.size(); ++i)
	{
		if (objectID == kWeapons[i].ObjectID)
			return i;
	}
	
	return -1;
}

static bool TryModifyWeapon(LaraInfo& lara, GAME_OBJECT_ID objectID, int ammoAmount, bool add)
{
	int arrayPos = GetArraySlot(kWeapons, objectID);
	if (-1 == arrayPos)
		return false;

	WeaponPickupInfo info = kWeapons[arrayPos];

	// Set the SelectedAmmo type to WeaponAmmoType::Ammo1 (0) if adding the weapon for the first time.
	// Note that this refers to the index of the weapon's ammo array, and not the weapon's actual ammunition count.
	auto& currWeapon = lara.Weapons[(int)info.LaraWeaponType];

	if (!currWeapon.Present)
		currWeapon.SelectedAmmo = WeaponAmmoType::Ammo1;
	
	currWeapon.Present = add;
	if(!add)
	{
		if (info.LaraWeaponType == lara.Control.Weapon.GunType || info.LaraWeaponType == lara.Control.Weapon.LastGunType)
		{
			lara.Control.Weapon.RequestGunType = LaraWeaponType::None;

			// If Lara has pistols and it's not the pistols we're removing, set them
			// as the "next weapon" so that Lara equips them next.
			if (LaraWeaponType::Pistol == info.LaraWeaponType || !lara.Weapons[(int)LaraWeaponType::Pistol].Present)
			{
				lara.Control.Weapon.LastGunType = LaraWeaponType::None;
			}
			else
			{
				lara.Control.Weapon.LastGunType = LaraWeaponType::Pistol;
			}
		}

		if (HolsterType::Hips == info.Holster && lara.Control.Weapon.HolsterInfo.LeftHolster == HolsterSlotForWeapon(info.LaraWeaponType))
		{
			lara.Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;
			lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
		}
		else if (HolsterType::Back == info.Holster && lara.Control.Weapon.HolsterInfo.BackHolster == HolsterSlotForWeapon(info.LaraWeaponType))
		{
			lara.Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;
		}
	}

	auto ammoID = info.AmmoID;
	return add ? TryAddingAmmo(lara, ammoID, ammoAmount) : TryRemovingAmmo(lara, ammoID, ammoAmount);
}

// Adding a weapon will either give the player the weapon + an amount of ammo, or,
// if they already have the weapon, simply the ammo.
bool TryAddingWeapon(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount)
{
	return TryModifyWeapon(lara, objectID, amount, true);
}

// Removing a weapon is the reverse of the above; it will remove the weapon (if it's there) and the amount of ammo.
bool TryRemovingWeapon(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount)
{
	return TryModifyWeapon(lara, objectID, amount, false);
}

std::optional<bool> HasWeapon(LaraInfo& lara, GAME_OBJECT_ID objectID)
{
	int arrPos = GetArraySlot(kWeapons, objectID);
	if (-1 == arrPos)
		return std::nullopt;

	WeaponPickupInfo info = kWeapons[arrPos];
	return lara.Weapons[(int)info.LaraWeaponType].Present;
}
