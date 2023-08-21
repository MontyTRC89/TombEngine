#pragma once

struct ItemInfo;

namespace TEN::Entities::Generic
{
	void DoPushableRipples(int itemNumber);
	void DoPushableSplash(int itemNumber);
	void DoPushableBubbles(int itemNumber);

	void FloatingItem(ItemInfo& item, float floatForce = 1.0f);
	void FloatingBridge(ItemInfo& item, float floatForce = 1.0f);
}
