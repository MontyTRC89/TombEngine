#include "framework.h"
#include "pickup_misc_items.h"
#include "objectslist.h"
#include <array>
#include "lara_struct.h"
#include "pickuputil.h"

auto SilencerIsEquipped(LaraInfo& lara)
{
	return lara.Weapons[WEAPON_UZI].HasSilencer
		|| lara.Weapons[WEAPON_PISTOLS].HasSilencer
		|| lara.Weapons[WEAPON_SHOTGUN].HasSilencer
		|| lara.Weapons[WEAPON_REVOLVER].HasSilencer
		|| lara.Weapons[WEAPON_CROSSBOW].HasSilencer
		|| lara.Weapons[WEAPON_HK].HasSilencer;
};

auto LaserSightIsEquipped(LaraInfo& lara)
{
	return lara.Weapons[WEAPON_REVOLVER].HasLasersight
		|| lara.Weapons[WEAPON_CROSSBOW].HasLasersight
		|| lara.Weapons[WEAPON_HK].HasLasersight;
};

static bool TryModifyMiscCount(LaraInfo & lara, GAME_OBJECT_ID obj, bool add)
{
	auto modifyBeetleCount = [&](int bit)
	{
		if (add)
		{
			lara.hasBeetleThings |= 1 << bit;
		}
		else
		{
			lara.hasBeetleThings &= ~(1 << bit);
		}
	};
	switch (obj) {
	case ID_SILENCER_ITEM:
		lara.Silencer = add && !SilencerIsEquipped(lara);
		break;
	case ID_LASERSIGHT_ITEM:
		lara.Lasersight = add && !LaserSightIsEquipped(lara);
		break;

	case ID_BINOCULARS_ITEM:
		lara.Binoculars = add;
		break;
	case ID_CROWBAR_ITEM:
		lara.Crowbar = add;
		break;
	case ID_DIARY_ITEM:
		lara.Diary.Present = add;
		break;
	//todo.. what if the waterskin is already full?
	case ID_WATERSKIN1_EMPTY:
		lara.small_waterskin = int{ add };
		break;
	case ID_WATERSKIN2_EMPTY:
		lara.big_waterskin = int{ add };
		break;
	case ID_CLOCKWORK_BEETLE:
		modifyBeetleCount(0);
		break;
	case ID_CLOCKWORK_BEETLE_COMBO1:
		modifyBeetleCount(1);
		break;
	case ID_CLOCKWORK_BEETLE_COMBO2:
		modifyBeetleCount(2);
		break;
	default:
		return false;
	}
	return true;
}

bool TryAddMiscItem(LaraInfo & lara, GAME_OBJECT_ID obj)
{
	return TryModifyMiscCount(lara, obj, true);
}

bool TryRemoveMiscItem(LaraInfo & lara, GAME_OBJECT_ID obj)
{
	return TryModifyMiscCount(lara, obj, false);
}

std::optional<bool> HasMiscItem(LaraInfo& lara, GAME_OBJECT_ID obj)
{	
	auto HasBeetle = [&](int bit)
	{
		return lara.hasBeetleThings &= 1 << bit;
	};

	switch (obj) {
		//TODO does Lara "HAVE" a silencer if it's combined but not in her inventory?
	case ID_SILENCER_ITEM:
		return lara.Silencer || SilencerIsEquipped(lara);
	case ID_LASERSIGHT_ITEM:
		return lara.Lasersight || LaserSightIsEquipped(lara);
	case ID_BINOCULARS_ITEM:
		return lara.Binoculars;
	case ID_CROWBAR_ITEM:
		return lara.Crowbar;
	case ID_DIARY_ITEM:
		return lara.Diary.Present;
		//TODO what about other waterskins?
	case ID_WATERSKIN1_EMPTY:
		return lara.small_waterskin;
	case ID_WATERSKIN2_EMPTY:
		return lara.big_waterskin;
	case ID_CLOCKWORK_BEETLE:
		return HasBeetle(0);
	case ID_CLOCKWORK_BEETLE_COMBO1:
		return HasBeetle(1);
	case ID_CLOCKWORK_BEETLE_COMBO2:
		return HasBeetle(2);
	default:
		return std::nullopt;
	}
}
