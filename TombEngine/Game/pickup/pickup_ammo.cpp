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
		{ ID_SHOTGUN_AMMO1_ITEM, LaraWeaponType::Shotgun, WeaponAmmoType::Ammo1, 6 },
		{ ID_SHOTGUN_AMMO2_ITEM, LaraWeaponType::Shotgun, WeaponAmmoType::Ammo2, 6 },
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

bool TryModifyingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, std::optional<int> amount, ModificationType modType)
{
	int arrayPos = GetArraySlot(kAmmo, objectID);
	if (-1 == arrayPos)
		return false;

	auto ammoPickup = kAmmo[arrayPos];

	auto& currentWeapon = lara.Weapons[(int)ammoPickup.LaraWeaponType];
	auto& currentAmmo = currentWeapon.Ammo[(int)ammoPickup.AmmoType];

	switch(modType)
	{
	case ModificationType::Set:
		currentAmmo = amount.value();
		currentAmmo.setInfinite(amount == -1);
		break;

	default:
		if (!currentAmmo.hasInfinite())
		{
			int defaultModify = modType == ModificationType::Add ? ammoPickup.Amount : -ammoPickup.Amount;
			int newVal = (int)currentAmmo.getCount() + (amount.has_value() ? amount.value() : defaultModify);
			currentAmmo = std::max(0, newVal);
		}

		break;
	};

	return true;
}

bool TryAddingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, std::optional<int> amount)
{
	return TryModifyingAmmo(lara, objectID, amount, ModificationType::Add);
}

bool TryRemovingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, std::optional<int> amount)
{	
	if (amount.has_value())
		return TryModifyingAmmo(lara, objectID, -amount.value(), ModificationType::Remove);
	else
		return TryModifyingAmmo(lara, objectID, amount, ModificationType::Remove);
}

std::optional<int> GetAmmoCount(LaraInfo& lara, GAME_OBJECT_ID objectID)
{
	int arrayPos = GetArraySlot(kAmmo, objectID);
	if (-1 == arrayPos)
		return std::nullopt;

	AmmoPickupInfo info = kAmmo[arrayPos];

	if (!lara.Weapons[(int)info.LaraWeaponType].Ammo[(int)info.AmmoType].hasInfinite())
		return lara.Weapons[(int)info.LaraWeaponType].Ammo[(int)info.AmmoType].getCount();

	// -1 signifies infinite ammo.
	return -1;
}
