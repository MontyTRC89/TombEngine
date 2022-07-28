#include "framework.h"
#include "Game/pickup/pickup_consumable.h"

#include <array>
#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickuputil.h"
#include "Objects/objectslist.h"

struct ConsumablePickupInfo
{
	GAME_OBJECT_ID ObjectID;
	// Pointer to array of consumable in question
	int LaraInventoryData::* Count;
	// How many of the item to give the player if the caller
	// does not specify; i.e. default amount per pickup
	int Amount;
};

static constexpr std::array<ConsumablePickupInfo, 3> kConsumables =
{
	{
		{ ID_SMALLMEDI_ITEM, &LaraInventoryData::TotalSmallMedipacks, 1 },
		{ ID_BIGMEDI_ITEM, &LaraInventoryData::TotalLargeMedipacks, 1 },
		{ ID_FLARE_INV_ITEM, &LaraInventoryData::TotalFlares, 12 }
	}
 };

static bool TryModifyingConsumable(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount, bool add)
{
	int arrayPos = GetArraySlot(kConsumables, objectID);
	if (-1 == arrayPos)
		return false;

	ConsumablePickupInfo info = kConsumables[arrayPos];

	if (lara.Inventory.*(info.Count) != -1)
	{
		int defaultModify = add ? info.Amount : -info.Amount;
		int newVal = lara.Inventory.*(info.Count) + (amount ? amount : defaultModify);
		lara.Inventory.*(info.Count) = std::max(0, newVal);
	}

	return true;
}

bool TryAddingConsumable(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount)
{
	return TryModifyingConsumable(lara, objectID, amount, true);
}

bool TryRemovingConsumable(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount)
{
	return TryModifyingConsumable(lara, objectID, -amount, false);
}

std::optional<int> GetConsumableCount(LaraInfo& lara, GAME_OBJECT_ID objectID)
{
	int arrayPos = GetArraySlot(kConsumables, objectID);
	if (-1 == arrayPos)
		return std::nullopt;

	ConsumablePickupInfo info = kConsumables[arrayPos];

	return lara.Inventory.*(info.Count);
}
