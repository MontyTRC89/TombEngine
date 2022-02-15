#include "framework.h"
#include "Game/pickup/pickup_ammo.h"

#include <array>
#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickuputil.h"
#include "Objects/objectslist.h"

struct AmmoPickupInfo
{
	GAME_OBJECT_ID id;
	LaraWeaponType laraWeaponType;
	WeaponAmmoType ammoType;
	int amt;
};

static constexpr std::array<AmmoPickupInfo, 14> kAmmo{ {
	{ID_PISTOLS_AMMO_ITEM, WEAPON_PISTOLS, WEAPON_AMMO1, 0},
	{ID_UZI_AMMO_ITEM, WEAPON_UZI, WEAPON_AMMO1, 30},
	{ID_SHOTGUN_AMMO1_ITEM, WEAPON_SHOTGUN, WEAPON_AMMO1, 36},
	{ID_SHOTGUN_AMMO2_ITEM, WEAPON_SHOTGUN, WEAPON_AMMO2, 36},
	{ID_CROSSBOW_AMMO1_ITEM, WEAPON_CROSSBOW, WEAPON_AMMO1, 10},
	{ID_CROSSBOW_AMMO2_ITEM, WEAPON_CROSSBOW, WEAPON_AMMO2, 10},
	{ID_CROSSBOW_AMMO3_ITEM, WEAPON_CROSSBOW, WEAPON_AMMO3, 10},
	{ID_REVOLVER_AMMO_ITEM, WEAPON_REVOLVER, WEAPON_AMMO1, 6},
	{ID_HK_AMMO_ITEM, WEAPON_HK, WEAPON_AMMO1, 30},
	{ID_GRENADE_AMMO1_ITEM, WEAPON_GRENADE_LAUNCHER, WEAPON_AMMO1, 10},
	{ID_GRENADE_AMMO2_ITEM, WEAPON_GRENADE_LAUNCHER, WEAPON_AMMO2, 10},
	{ID_GRENADE_AMMO3_ITEM, WEAPON_GRENADE_LAUNCHER, WEAPON_AMMO3, 10},
	{ID_ROCKET_LAUNCHER_AMMO_ITEM, WEAPON_ROCKET_LAUNCHER, WEAPON_AMMO1, 10},
	{ID_HARPOON_AMMO_ITEM, WEAPON_HARPOON_GUN, WEAPON_AMMO1, 10} }
};

static bool TryModifyAmmo(LaraInfo& lara, GAME_OBJECT_ID obj, int amt, bool add)
{
	int arrPos = GetArrSlot(kAmmo, obj);
	if (-1 == arrPos)
	{
		return false;
	}

	AmmoPickupInfo info = kAmmo[arrPos];
	auto currentAmmo = lara.Weapons[info.laraWeaponType].Ammo[info.ammoType];
	if (!currentAmmo.hasInfinite())
	{
		auto defaultModify = add ? info.amt : -info.amt;
		auto newVal = int{ currentAmmo.getCount() } + (amt ? amt : defaultModify);
		lara.Weapons[info.laraWeaponType].Ammo[info.ammoType] = std::max(0, newVal);
	}
	return true;
}

// We need the extra bool because amt might be zero to signify the
// default amount
bool TryAddAmmo(LaraInfo& lara, GAME_OBJECT_ID obj, int amt)
{
	return TryModifyAmmo(lara, obj, amt, true);
}

bool TryRemoveAmmo(LaraInfo & lara, GAME_OBJECT_ID obj, int amt)
{	
	return TryModifyAmmo(lara, obj, -amt, false);
}

std::optional<int> GetAmmoCount(LaraInfo& lara, GAME_OBJECT_ID obj)
{
	int arrPos = GetArrSlot(kAmmo, obj);
	if (-1 == arrPos)
	{
		return std::nullopt;
	}

	AmmoPickupInfo info = kAmmo[arrPos];
	if (!lara.Weapons[info.laraWeaponType].Ammo[info.ammoType].hasInfinite())
	{
		return lara.Weapons[info.laraWeaponType].Ammo[info.ammoType].getCount();
	}
	// -1 means infinite ammo
	return -1;
}

