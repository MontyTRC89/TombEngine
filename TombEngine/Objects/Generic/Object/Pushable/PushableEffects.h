#pragma once

struct ItemInfo;

namespace TEN::Entities::Generic
{
	void HandlePushableRippleEffect(ItemInfo& pushableItem);
	void SpawnPushableSplash(ItemInfo& pushableItem);
	void SpawnPushableBubbles(const ItemInfo& pushableItem);

	// TODO: Move. Not effects.
	void HandlePushableFloatOscillation(ItemInfo& item, float oscillation = 1.0f);
	void HandlePushableBridgeFloatOscillation(ItemInfo& item, float oscillation = 1.0f);
	void HandlePushableFallRotation(ItemInfo& item);
}
