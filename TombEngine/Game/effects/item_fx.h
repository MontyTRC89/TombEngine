#pragma once

struct ItemInfo;

namespace TEN::Effects::Items
{
	constexpr int DEFAULT_NONLETHAL_EFFECT_TIMEOUT = 1.5f * FPS;

	void ItemBurn(ItemInfo* item, int timeout = -1);
	void ItemElectricBurn(ItemInfo* item, int timeout = DEFAULT_NONLETHAL_EFFECT_TIMEOUT);
	void ItemSmoke(ItemInfo* item, int timeout = DEFAULT_NONLETHAL_EFFECT_TIMEOUT);
	void LavaBurn(ItemInfo* item);
	void LaraBreath(ItemInfo* item);
}
