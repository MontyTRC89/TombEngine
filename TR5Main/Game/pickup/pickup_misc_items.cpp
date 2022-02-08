#include "framework.h"
#include "Game/pickup/pickup_misc_items.h"

#include <array>
#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickuputil.h"
#include "Objects/objectslist.h"

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
	// If adding, replace the small/large waterskin with one of the requested
	// capacity. If removing, only remove the waterskin if it contains the given
	// capacity.
	auto modifyWaterSkinAmount = [&](byte& currentFlag, byte newFlag)
	{
		if (add)
		{
			currentFlag = newFlag;
		}
		else
		{
			if (currentFlag == newFlag)
			{
				currentFlag = 0;
			}
		}
	};

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
	case ID_WATERSKIN1_EMPTY:
		modifyWaterSkinAmount(lara.smallWaterskin, 1);
		break;
	case ID_WATERSKIN1_1:
		modifyWaterSkinAmount(lara.smallWaterskin, 2);
		break;
	case ID_WATERSKIN1_2:
		modifyWaterSkinAmount(lara.smallWaterskin, 3);
		break;
	case ID_WATERSKIN1_3:
		modifyWaterSkinAmount(lara.smallWaterskin, 4);
		break;
	case ID_WATERSKIN2_EMPTY:
		modifyWaterSkinAmount(lara.bigWaterskin, 1);
		break;
	case ID_WATERSKIN2_1:
		modifyWaterSkinAmount(lara.bigWaterskin, 2);
		break;
	case ID_WATERSKIN2_2:
		modifyWaterSkinAmount(lara.bigWaterskin, 3);
		break;
	case ID_WATERSKIN2_3:
		modifyWaterSkinAmount(lara.bigWaterskin, 4);
		break;
	case ID_WATERSKIN2_4:
		modifyWaterSkinAmount(lara.bigWaterskin, 5);
		break;
	case ID_WATERSKIN2_5:
		modifyWaterSkinAmount(lara.bigWaterskin, 6);
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
	case ID_WATERSKIN1_EMPTY:
		return lara.smallWaterskin == 1;
	case ID_WATERSKIN1_1:
		return lara.smallWaterskin == 2;
	case ID_WATERSKIN1_2:
		return lara.smallWaterskin == 3;
	case ID_WATERSKIN1_3:
		return lara.smallWaterskin == 4;
	case ID_WATERSKIN2_EMPTY:
		return lara.bigWaterskin == 1;
	case ID_WATERSKIN2_1:
		return lara.bigWaterskin == 2;
	case ID_WATERSKIN2_2:
		return lara.bigWaterskin == 3;
	case ID_WATERSKIN2_3:
		return lara.bigWaterskin == 4;
	case ID_WATERSKIN2_4:
		return lara.bigWaterskin == 5;
	case ID_WATERSKIN2_5:
		return lara.bigWaterskin == 6;
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
