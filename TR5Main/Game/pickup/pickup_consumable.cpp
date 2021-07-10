#pragma once
#include "framework.h"
#include "pickup_consumable.h"
#include "objectslist.h"
#include <array>
#include "lara_struct.h"
#include "pickuputil.h"

struct ConsumablePickupInfo
{
	GAME_OBJECT_ID id;
	int LaraInfo::* count;
	// How many of the item to give the player if the caller
	// does not specify; i.e. default amount per pickup
	int amt;
};

static constexpr std::array<ConsumablePickupInfo, 3> kConsumables = { {
	{ID_SMALLMEDI_ITEM, &LaraInfo::NumSmallMedipacks, 1},
	{ID_BIGMEDI_ITEM, &LaraInfo::NumLargeMedipacks, 1},
	{ID_FLARE_INV_ITEM, &LaraInfo::NumFlares, 12} }
 };

static bool TryModifyConsumable(LaraInfo & lara, GAME_OBJECT_ID obj, int amt, bool add)
{
	int arrPos = GetArrSlot(kConsumables, obj);
	if (-1 == arrPos)
	{
		return false;
	}

	ConsumablePickupInfo info = kConsumables[arrPos];
	if (lara.*(info.count) != -1)
	{
		auto defaultModify = add ? info.amt : -info.amt;
		auto newVal = lara.*(info.count) + (amt ? amt : defaultModify);
		lara.*(info.count) = std::max(0, newVal);
	}
	return true;
}

bool TryAddConsumable(LaraInfo & lara, GAME_OBJECT_ID obj, int amt)
{
	return TryModifyConsumable(lara, obj, amt, true);
}

bool TryRemoveConsumable(LaraInfo & lara, GAME_OBJECT_ID obj, int amt)
{
	return TryModifyConsumable(lara, obj, -amt, false);
}

std::optional<int> GetConsumableCount(LaraInfo& lara, GAME_OBJECT_ID obj)
{
	int arrPos = GetArrSlot(kConsumables, obj);
	if (-1 == arrPos)
	{
		return std::nullopt;
	}

	ConsumablePickupInfo info = kConsumables[arrPos];
	return lara.*(info.count);
}
