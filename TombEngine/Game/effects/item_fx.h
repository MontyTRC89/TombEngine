#pragma once

struct ItemInfo;

namespace TEN::Effects::Items
{
	void ItemBurn(ItemInfo* item, int timeout = -1);
	void ItemElectricBurn(ItemInfo* item, int timeout = 48);
	void ItemSmoke(ItemInfo* item, int timeout = 48);
	void LavaBurn(ItemInfo* item);
	void LaraBreath(ItemInfo* item);
}
