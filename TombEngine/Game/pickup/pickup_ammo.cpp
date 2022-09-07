#include "framework.h"
#include "Game/pickup/pickup_ammo.h"

#include <array>

#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickuputil.h"
#include "Objects/objectslist.h"

struct AmmoPickupInfo
{
	GAME_OBJECT_ID ObjectID;
	LaraWeaponType LaraWeaponType;
	WeaponAmmoType AmmoType;
	int Amount;
};

static constexpr std::array<AmmoPickupInfo, 14> kAmmo
{
	{
		{ ID_PISTOLS_AMMO_ITEM, LaraWeaponType::Pistol, WeaponAmmoType::Ammo1, 0 },
		{ ID_UZI_AMMO_ITEM, LaraWeaponType::Uzi, WeaponAmmoType::Ammo1, 30 },
		{ ID_SHOTGUN_AMMO1_ITEM, LaraWeaponType::Shotgun, WeaponAmmoType::Ammo1, 36 },
		{ ID_SHOTGUN_AMMO2_ITEM, LaraWeaponType::Shotgun, WeaponAmmoType::Ammo2, 36 },
		{ ID_CROSSBOW_AMMO1_ITEM, LaraWeaponType::Crossbow, WeaponAmmoType::Ammo1, 10 },
		{ ID_CROSSBOW_AMMO2_ITEM, LaraWeaponType::Crossbow, WeaponAmmoType::Ammo2, 10 },
		{ ID_CROSSBOW_AMMO3_ITEM, LaraWeaponType::Crossbow, WeaponAmmoType::Ammo3, 10 },
		{ ID_REVOLVER_AMMO_ITEM, LaraWeaponType::Revolver, WeaponAmmoType::Ammo1, 6 },
		{ ID_HK_AMMO_ITEM, LaraWeaponType::HK, WeaponAmmoType::Ammo1, 30 },
		{ ID_GRENADE_AMMO1_ITEM, LaraWeaponType::GrenadeLauncher, WeaponAmmoType::Ammo1, 10 },
		{ ID_GRENADE_AMMO2_ITEM, LaraWeaponType::GrenadeLauncher, WeaponAmmoType::Ammo2, 10 },
		{ ID_GRENADE_AMMO3_ITEM, LaraWeaponType::GrenadeLauncher, WeaponAmmoType::Ammo3, 10 },
		{ ID_ROCKET_LAUNCHER_AMMO_ITEM, LaraWeaponType::RocketLauncher, WeaponAmmoType::Ammo1, 10 },
		{ ID_HARPOON_AMMO_ITEM, LaraWeaponType::HarpoonGun, WeaponAmmoType::Ammo1, 10 }
	}
};

static bool TryModifyingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount, bool add)
{
	int arrayPos = GetArraySlot(kAmmo, objectID);
	if (-1 == arrayPos)
		return false;

	auto info = kAmmo[arrayPos];

	auto currentAmmo = lara.Weapons[(int)info.LaraWeaponType].Ammo[(int)info.AmmoType];
	if (!currentAmmo.hasInfinite())
	{
		int defaultModify = add ? info.Amount : -info.Amount;
		int newVal = int{ currentAmmo.getCount() } + (amount ? amount : defaultModify);
		lara.Weapons[(int)info.LaraWeaponType].Ammo[(int)info.AmmoType] = std::max(0, newVal);
	}

	return true;
}

// We need the extra bool because amount might be zero to signify the default amount.
bool TryAddingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount)
{
	return TryModifyingAmmo(lara, objectID, amount, true);
}

bool TryRemovingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount)
{	
	return TryModifyingAmmo(lara, objectID, -amount, false);
}

std::optional<int> GetAmmoCount(LaraInfo& lara, GAME_OBJECT_ID objectID)
{
	int arrayPos = GetArraySlot(kAmmo, objectID);
	if (-1 == arrayPos)
		return std::nullopt;

	auto info = kAmmo[arrayPos];

	if (!lara.Weapons[(int)info.LaraWeaponType].Ammo[(int)info.AmmoType].hasInfinite())
		return lara.Weapons[(int)info.LaraWeaponType].Ammo[(int)info.AmmoType].getCount();

	// -1 signifies infinite ammo.
	return -1;
}
