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
		{ ID_PISTOLS_AMMO_ITEM, LaraWeaponType::Pistol, WEAPON_AMMO1, 0 },
		{ ID_UZI_AMMO_ITEM, LaraWeaponType::Uzi, WEAPON_AMMO1, 30 },
		{ ID_SHOTGUN_AMMO1_ITEM, LaraWeaponType::Shotgun, WEAPON_AMMO1, 36 },
		{ ID_SHOTGUN_AMMO2_ITEM, LaraWeaponType::Shotgun, WEAPON_AMMO2, 36 },
		{ ID_CROSSBOW_AMMO1_ITEM, LaraWeaponType::Crossbow, WEAPON_AMMO1, 10 },
		{ ID_CROSSBOW_AMMO2_ITEM, LaraWeaponType::Crossbow, WEAPON_AMMO2, 10 },
		{ ID_CROSSBOW_AMMO3_ITEM, LaraWeaponType::Crossbow, WEAPON_AMMO3, 10 },
		{ ID_REVOLVER_AMMO_ITEM, LaraWeaponType::Revolver, WEAPON_AMMO1, 6 },
		{ ID_HK_AMMO_ITEM, LaraWeaponType::HK, WEAPON_AMMO1, 30 },
		{ ID_GRENADE_AMMO1_ITEM, LaraWeaponType::GrenadeLauncher, WEAPON_AMMO1, 10 },
		{ ID_GRENADE_AMMO2_ITEM, LaraWeaponType::GrenadeLauncher, WEAPON_AMMO2, 10 },
		{ ID_GRENADE_AMMO3_ITEM, LaraWeaponType::GrenadeLauncher, WEAPON_AMMO3, 10 },
		{ ID_ROCKET_LAUNCHER_AMMO_ITEM, LaraWeaponType::RocketLauncher, WEAPON_AMMO1, 10 },
		{ ID_HARPOON_AMMO_ITEM, LaraWeaponType::HarpoonGun, WEAPON_AMMO1, 10 }
	}
};

static bool TryModifyingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount, bool add)
{
	int arrayPos = GetArraySlot(kAmmo, objectID);
	if (-1 == arrayPos)
		return false;

	AmmoPickupInfo info = kAmmo[arrayPos];

	auto currentAmmo = lara.Weapons[(int)info.LaraWeaponType].Ammo[info.AmmoType];
	if (!currentAmmo.hasInfinite())
	{
		int defaultModify = add ? info.Amount : -info.Amount;
		int newVal = int{ currentAmmo.getCount() } + (amount ? amount : defaultModify);
		lara.Weapons[(int)info.LaraWeaponType].Ammo[info.AmmoType] = std::max(0, newVal);
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

	AmmoPickupInfo info = kAmmo[arrayPos];

	if (!lara.Weapons[(int)info.LaraWeaponType].Ammo[info.AmmoType].hasInfinite())
		return lara.Weapons[(int)info.LaraWeaponType].Ammo[info.AmmoType].getCount();

	// -1 signifies infinite ammo.
	return -1;
}
